# Cobalt-X Zero: Firmware Architecture & Engineering Datasheet

---

> **Implementation Note:** All code, algorithms, and logic descriptions are reference specifications. They define required behavior and constraints but are not drop-in implementations. All register-level details must be validated against the STM32G4 Reference Manual (RM0440), PMW3360 datasheet, and USB 2.0 HID specification. Only direct register requirements and electrical constraints are to be treated as authoritative.

---

## Table of Contents

1. [Firmware Design Principles](#1-firmware-design-principles)
2. [Hardware Architecture](#2-hardware-architecture)
3. [Clock & Power](#3-clock--power)
4. [PMW3360 Sensor Driver](#4-pmw3360-sensor-driver)
5. [Firmware Structure](#5-firmware-structure)
6. [Virtual Drive: vDrive](#6-virtual-drive-vdrive)
7. [Execution Logic](#7-execution-logic)
8. [MAC Button & HyprX Macro](#8-mac-button--hyprx-macro)
9. [System Integrity](#9-system-integrity)

---

## 1. Firmware Design Principles

The firmware targets deterministic, zero-jitter USB report delivery. Every report must be produced under identical timing conditions, with no variation from scheduling, buffering, or conditional execution paths.

**Core constraints:**

- All processing must complete before the USB IN token arrives. The USB interrupt handler transfers precomputed data only, with no logic or computation inside the ISR.
- Constant 170 MHz operation: no sleep states, no clock scaling, no conditional paths that alter timing.
- The input pipeline is strictly linear: no acceleration, no prediction, no nonlinear transforms. All movement is 1:1.
- Use Atomic Reports for handling packets to ensure when there is a click or other action, the mouse does not stop moving. Essentially multithreading.

At 170 MHz with a 32 kHz processing loop, each tick has a budget of 5,312 cycles. With DMA handling all SPI transfers, actual CPU work per tick is approximately 100–200 cycles (accumulator math, button reads, encoder counter, report staging). The USB ISR at 1 kHz costs roughly 50 cycles per invocation. This yields an estimated **3–6% sustained CPU utilisation** under full motion and click load. The remaining 94–97% of available cycles are structurally idle, providing deterministic worst-case headroom with no risk of a frame being late due to compute pressure. The MCU is never clocked down, throttled, or power-gated. All cycles are available, always.

---

## 2. Hardware Architecture

### 2.1 System Overview

| Parameter  | Value                                   |
| ---------- | --------------------------------------- |
| MCU        | STM32G431KBT6                           |
| Package    | LQFP-32 (7 x 7 mm)                      |
| Core       | ARM Cortex-M4F with FPU                 |
| Flash      | 128 KB                                  |
| SRAM       | 32 KB                                   |
| SYSCLK     | 170 MHz                                 |
| Sensor     | PixArt PMW3360DM-T2QU                   |
| USB        | Full-Speed 12 Mbps, HID + MSC composite |
| Power Rail | AP2112K-3.3V LDO (600 mA), 5 V to 3.3 V |
| LEDs       | 3x WS2812B on PB7 via TIM4_CH2 + DMA    |

### 2.2 Pin Mapping: Authoritative Register Configuration

All GPIO output pins: `OSPEEDR = 0b11` (high-speed).  
All input pins: `PUPDR = 0b01` (pull-up), active-low logic.

| Pin  | Net     | Mode      | AF / Config    | Description                      |
| ---- | ------- | --------- | -------------- | -------------------------------- |
| PA0  | WHEEL_A | Alternate | AF1, TIM2_CH1  | Rotary encoder Phase A           |
| PA1  | WHEEL_B | Alternate | AF1, TIM2_CH2  | Rotary encoder Phase B           |
| PA4  | CS      | Output    | Push-Pull      | PMW3360 NCS, active LOW          |
| PA5  | SCLK    | Alternate | AF5, SPI1_SCK  | SPI1 clock, max 2 MHz            |
| PA6  | MISO    | Alternate | AF5, SPI1_MISO | SPI1 data to MCU                 |
| PA7  | MOSI    | Alternate | AF5, SPI1_MOSI | SPI1 data to sensor              |
| PA11 | USB_D-  | Alternate | AF10, USB_DM   | USB differential minus           |
| PA12 | USB_D+  | Alternate | AF10, USB_DP   | USB differential plus            |
| PB0  | MOTION  | Input     | EXTI0          | Motion interrupt, active LOW     |
| PB3  | LMB     | Input     | Pull-Up        | Left mouse button                |
| PB4  | RMB     | Input     | Pull-Up        | Right mouse button               |
| PB5  | MMB     | Input     | Pull-Up        | Middle mouse button / HyprX swap |
| PB6  | MAC     | Input     | Pull-Up        | Mode toggle / vDrive trigger     |
| PB7  | LED     | Alternate | AF2, TIM4_CH2  | WS2812B data via DMA             |

> **SPI Clock Correction:** The PMW3360 maximum SPI clock is **2 MHz**, not 12 MHz. Exceeding this will cause corrupted register reads and motion data loss.

---

## 3. Clock & Power

All clocks are sourced exclusively from the STM32G431's **internal oscillators**. There is no external crystal, no external clock source, and no PCB footprint required for clock generation. The HSI16 drives the main PLL and the HSI48 drives USB. Both are fully integrated on-die. This is not a compromise: the CRS peripheral continuously trims HSI48 against the USB SOF signal from the host, locking it to the host's reference clock with sufficient accuracy for Full-Speed USB. The result is a correct, stable, low-BOM clock architecture with zero dependency on external passives for timing.

### 3.1 Power Sequencing

1. Set voltage scaling to **Range 1**
2. Enable **Boost mode** before configuring PLL
3. Set **Flash latency** before switching SYSCLK

### 3.2 PLL Configuration: 170 MHz

```
Source:  HSI16 (16 MHz)
/M = 4  ->  4 MHz
xN = 85 ->  340 MHz VCO
/R = 2  ->  170 MHz SYSCLK
```

VCO must remain within 64–344 MHz (RM0440 §6.4).

### 3.3 USB Clock

| Setting | Value                         |
| ------- | ----------------------------- |
| Source  | HSI48 (48 MHz)                |
| CRS     | Enabled, locked to USB SOF    |
| Routing | `RCC->CCIPR` USB clock select |

---

## 4. PMW3360 Sensor Driver

### 4.1 Initialisation Sequence

The sensor requires SROM firmware upload on every power-on. Without it, tracking degrades significantly.

```
1. Write 0x5A -> register 0x3A  (Power_Up_Reset)
2. Delay >= 50 ms
3. Burst-write SROM image -> register 0x62
4. Read and verify SROM_ID
```

Use the SROM binary from `/drivers/`. Do not substitute or skip.

### 4.2 SPI Configuration

| Parameter   | Value                       |
| ----------- | --------------------------- |
| Mode        | SPI Mode 3 (CPOL=1, CPHA=1) |
| Bit order   | MSB first                   |
| Max SCLK    | 2 MHz                       |
| Chip select | Manual (PA4, active LOW)    |

### 4.3 Timing Constraints

| Parameter | Minimum | Notes                        |
| --------- | ------- | ---------------------------- |
| tSRAD     | 35 µs   | Address-to-data read delay   |
| tSCLK     | N/A     | Max 2 MHz                    |
| tBEXIT    | 500 ns  | Burst exit guard time        |
| NCS       | Stable  | Must not glitch mid-transfer |

Any violation results in corrupted data.

### 4.4 Motion Handling

- Sensor outputs signed 16-bit deltas (int16_t) for X and Y.
- The internal FIFO can overflow if not read promptly. Service the MOTION interrupt (PB0, active LOW) immediately.
- Use DMA for all SPI transfers. The CPU must never block on SPI.

### 4.5 DMA Channel Assignment

| Channel  | Function |
| -------- | -------- |
| DMA1 CH2 | SPI1 RX  |
| DMA1 CH3 | SPI1 TX  |

Transfer completion signalled via interrupt.

---

## 5. Firmware Structure

The project root contains all top-level source files alongside a `Makefile` and `Linker.ld`. A `drivers/` subdirectory contains all C drivers, either sourced from the internet or written for this project. No other directory structure is assumed.

**Implementation requirements:**

- Direct register access only, no HAL or LL library abstractions
- DMA for all high-throughput I/O (SPI, WS2812B)
- Strict module separation, no cross-domain side effects
- All shared state declared `volatile`
- Eager debounce on all buttons (trigger on first edge, lockout timer)

### 5.1 Flashing

The primary flashing method is **DFU (Device Firmware Upgrade)** via **Arduino IDE v2 on Linux**. Any compliant DFU toolchain will work regardless, but this is the expected and tested path. In rare cases, such as a bricked DFU bootloader or early board bring-up, the PCB exposes test copper pads for an **ST-Link** connection as a recovery method.

The flashing method has no bearing on runtime behaviour. Once flashed, the device operates identically regardless of how the firmware was written.

**Host compatibility:** The device uses only standard USB HID and USB Mass Storage Class descriptors. It requires no drivers and is compatible with any operating system that supports a USB mouse and USB mass storage. If the host can move a cursor and mount a USB drive, the device will work.

---

## 6. Virtual Drive: vDrive

### 6.1 Activation

vDrive is activated **only when MAC (PB6) is held during USB plug-in**. Under normal plug-in, the device enumerates as HID only.

### 6.2 Overview

The device presents a virtual USB Mass Storage Class (MSC) volume for configuration. No real storage media exists. All filesystem structures are generated dynamically in RAM from Flash-stored config.

| Property      | Value                    |
| ------------- | ------------------------ |
| Volume label  | `Cobalt-X Zero (Config)` |
| Reported size | 67 MB (70,254,592 bytes) |

### 6.3 Filesystem Model

All sectors generated at runtime:

- FAT12/16 tables constructed in RAM
- Root directory entry generated dynamically
- `config.ini` contents read from dedicated Flash page

No persistent FAT or directory storage exists.

### 6.4 Host Write Pipeline

```
1. Buffer incoming sector writes in RAM
2. Detect writes targeting config.ini cluster range
3. Accumulate until full file received
4. Parse entire file
5. Validate all fields
6. If valid:   commit to Flash atomically
7. If invalid: discard entirely, no partial update
```

### 6.5 Validation Rules

The parser must:

- Match config keys case-sensitively and exactly
- Enforce numeric bounds per field definition
- Reject malformed syntax, unknown keys, and invalid characters
- Reject any file that fails validation in full, no partial commits

### 6.6 Flash Storage

- One dedicated Flash page (2 KB on STM32G431)
- Full page erase before every write
- Double-word (64-bit) aligned programming
- Verify write via `FLASH->SR` after completion

### 6.7 Boot Regeneration

On every boot, the virtual filesystem is rebuilt from Flash. Any host-side changes (renames, creates, deletes) are discarded. The canonical state is always restored.

### 6.8 Host Interference Handling

The host may attempt formatting, volume rename, file creation/deletion, or config.ini rename. All such operations must be silently accepted during the session and discarded on the next plug cycle. The device never enters an inconsistent state.

---

## 7. Execution Logic

### 7.1 32 kHz Processing Engine

A hardware timer fires at 32 kHz, driving all input processing. This gives 32 internal processing ticks per 1 ms USB frame. All motion data is accumulated and resolved before each USB report.

- Consistent inter-tick spacing, no burst catch-up
- All sensor reads and input processing occur here

### 7.2 Motion Accumulation

32-bit signed accumulators prevent sub-count motion loss across frames:

```c
acc_x += dx;
acc_y += dy;

// At USB frame boundary (1 ms):
out_x  = clamp(acc_x, -127, 127);
acc_x -= out_x;   // retain remainder
```

Remainder is carried forward. No motion is dropped.

### 7.3 Optional Smoothing

Smoothing is **disabled by default** and must not alter motion accuracy.

| Method                  | Constraint                |
| ----------------------- | ------------------------- |
| FIR filter (N=2–3)      | 1 frame added latency max |
| Exponential (a=0.6–0.8) | No nonlinear distortion   |

Any smoothing must be linear and latency-bounded. Default: **OFF**.

### 7.4 Frame Alignment

USB frame boundary defines the output window. All accumulated motion is flushed at the boundary. No re-timing, no dropping, no inter-frame carryover of output.

### 7.5 Button Debounce

- Trigger on first falling edge, zero artificial input delay
- Lockout timer: ~5 ms to reject bounce
- Implemented via timer comparison, not blocking delay

### 7.6 Scroll Encoder

TIM2 configured in encoder mode (PA0/PA1, AF1):

- Hardware quadrature decoding, no software edge tracking
- Steps accumulated per frame
- Scroll events output as discrete wheel report values in Normal mode only (see §8.4)
- Filtering via timer, not software delay

### 7.7 CPI Switching

- Accumulators are **not reset** on CPI change
- New CPI applied to sensor register immediately
- No output spike or report discontinuity

### 7.8 Report Staging: Double Buffer

Two HID report buffers maintained:

- **Write buffer:** built during current frame
- **Read buffer:** transferred by USB interrupt

Buffers swapped atomically at frame boundary. USB interrupt only reads the read buffer, never writes.

### 7.9 USB HID Report Format

```c
typedef struct {
    int8_t  x;
    int8_t  y;
    uint8_t buttons;   // bitmask: bit0=LMB, bit1=RMB, bit2=MMB
    int8_t  wheel;
} hid_report_t;
```

Constraints:

- Exactly **1 report per 1 ms frame**, no skips, no duplicates
- Report always sent, even when idle (zero-delta report)
- USB IN token handled exclusively by interrupt, no logic inside ISR

### 7.10 Jitter Elimination

| Source              | Mitigation                                                |
| ------------------- | --------------------------------------------------------- |
| ISR delay           | Precomputed report ready before IN token                  |
| Blocking SPI        | DMA-only transfers                                        |
| Late computation    | All logic completes in 32 kHz ticks before frame boundary |
| Scheduling variance | Fixed timer-driven loop, no RTOS                          |

---

## 8. MAC Button & HyprX Macro

### 8.1 Activation: Plug-in Behaviour

MAC button (PB6) behaviour is determined at USB enumeration:

| Condition                   | Result                               |
| --------------------------- | ------------------------------------ |
| MAC **not held** at plug-in | Normal HID enumeration, mode: Normal |
| MAC **held** at plug-in     | vDrive enumeration (see §6)          |

### 8.2 Runtime Mode State Machine

During normal HID operation, MAC press toggles a **volatile** two-state machine. State is never saved to Flash. Device always boots in Normal mode. 

```
        MAC press                   MAC press
Normal Mode  --------->  HyprX Mode  --------->  Normal Mode
 (default)
```

### 8.3 Normal Mode

| Input        | Behaviour             |
| ------------ | --------------------- |
| LED colour   | Cobalt blue           |
| LMB          | Standard left click   |
| RMB          | Standard right click  |
| MMB          | Standard middle click |
| Scroll wheel | Standard scroll wheel |
| MAC press    | Switch to HyprX mode  |

### 8.4 HyprX Mode

HyprX is a click automation macro. Its purpose is to convert each scroll wheel notch into a click signal, enabling high sustained CPS without physical button actuation.

| Input        | Behaviour                                                                                                                                                                   |
| ------------ | --------------------------------------------------------------------------------------------------------------------------------------------------------------------------- |
| LED colour   | Blood red                                                                                                                                                                   |
| LMB          | Standard left click                                                                                                                                                         |
| RMB          | Standard right click                                                                                                                                                        |
| Scroll wheel | Each notch (either direction) injects 1x active button click into the next USB poll. The `wheel` field is forced to zero. **No scroll packets are produced in HyprX mode.** |
| MMB press    | Toggles active click target: LMB or RMB (default: LMB)                                                                                                                      |
| MAC press    | Switch back to Normal mode                                                                                                                                                  |

**Click injection detail:**

Each scroll encoder step increments a pending-click counter. At the next 1 ms USB frame boundary, one pending click is consumed and serialised as a press+release pair across two consecutive reports. Encoder steps are fully consumed by this counter and are never written to the `wheel` field while HyprX mode is active. This ensures the host receives only click events from scroll input, with no simultaneous or accidental scroll output.

**Volatile state:**

| State variable      | Reset value | Persistence |
| ------------------- | ----------- | ----------- |
| Mode (Normal/HyprX) | Normal      | Power cycle |
| Active click (L/R)  | LMB         | Power cycle |

No HyprX state is written to Flash. Every plug cycle starts fresh in Normal mode with LMB as the active click target.

---

## 9. System Integrity

All subsystems must maintain:

- **Deterministic execution:** no variable-latency paths in the report pipeline
- **Fixed timing:** all outputs aligned to hardware timer boundaries
- **No hidden latency:** ISRs are data transfer only, computation lives in the 32 kHz loop
- **Atomic config operations:** Flash commits are all-or-nothing

Deviation from any of the above will manifest as polling jitter or click latency inconsistency, measurable on hardware click latency testers.

---

## Appendix: Performance Architecture & Design Philosophy

### A.1 The Fundamental Problem with Competing Designs

Most gaming mice on the market, including products from major manufacturers, are built around MCUs in the 48–72 MHz range. These are typically ARM Cortex-M0 or Cortex-M3 cores, sometimes custom silicon, clocked conservatively to meet power and cost targets. At 1 kHz polling, a 48 MHz MCU has a cycle budget of **48,000 cycles per frame**. At 8 kHz polling (as seen in some high-end wired mice), that budget collapses to **6,000 cycles per frame**, which is barely sufficient to execute a DMA-driven SPI burst read of the sensor, update a report buffer, and respond to the USB IN token. There is no meaningful headroom remaining for deterministic precomputation, jitter elimination, or any form of inter-frame smoothing. The firmware on those devices is pushed to its absolute limit simply to sustain the polling rate. The hardware is the bottleneck, and the firmware is written around that constraint.

The consequence is jitter. Even a mouse rated at 8 kHz (125 µs nominal frame interval) will exhibit measurable variance in actual report delivery timing when the MCU is running close to capacity. A report that should arrive at t=125 µs arrives at t=118 µs or t=134 µs depending on what else the MCU was doing in that window. This jitter is perceived as microstutter, inconsistent cursor response, and variable click latency, and it is an inevitable result of running a small MCU at its ceiling.

### A.2 The Cobalt-X Zero Approach

The STM32G431 at 170 MHz inverts this constraint entirely. At 1 kHz polling, the per-frame cycle budget is **170,000 cycles**. This is approximately **3.5x the total budget of a 48 MHz competitor running at the same rate, and 28x the budget of a 48 MHz device running at 8 kHz**. The entire steady-state firmware workload, including sensor reads via DMA, motion accumulation, button debounce, encoder processing, HyprX logic, and USB report staging, consumes an estimated 3–6% of this budget. The remaining 94–97% of cycles exist for one purpose: guaranteeing that the next USB report is precomputed and staged before the IN token arrives, unconditionally, every single frame, with zero possibility of a late compute path.

This firmware is written in strict C with direct register access only. No HAL abstractions, no vendor middleware, no RTOS. Every subsystem is a deterministic state machine operating on fixed timer boundaries. The USB ISR does exactly one thing: swap the double buffer and trigger the transfer. No decision-making, no conditional logic, no timing-sensitive operations occur inside the interrupt handler. All of that work is done in the 32 kHz processing loop, 32 ticks in advance of the USB frame boundary.

### A.3 Why 1 kHz Feels Better Than a Poorly Implemented 8 kHz

Raw polling rate is a ceiling, not a guarantee. A mouse advertised at 8 kHz with ±30 µs of delivery jitter produces report intervals that vary between approximately 95 µs and 155 µs. The host receives data at irregular intervals and must handle that variance in cursor rendering. The perceived smoothness is worse than the headline rate implies.

The Cobalt-X Zero targets **zero measurable jitter at 1 kHz**. Every report arrives at a 1 ms boundary, within the tolerance of the USB SOF clock which is hardware-locked via CRS to the host's USB reference. The host receives data on a perfectly metronomic schedule. Combined with the 32 kHz internal sampling rate (31.25 µs resolution), all sub-millisecond motion is accumulated and preserved in the 32-bit accumulators before being flushed to the report. No motion is lost, no motion is duplicated, and no report is ever late.

The result is that measured click latency on a hardware analyser reads `1 ms, 1 ms, 1 ms`, not `1.2 ms, 1.8 ms, 0.9 ms`. This consistency, delivered at 1 kHz, produces a tracking and response characteristic that is perceptually indistinguishable from or superior to a higher-rate device with worse timing discipline.

### A.4 Zero Host CPU Load

The device is designed so that plugging it in produces no measurable increase in host CPU utilisation beyond the baseline cost of a USB HID interrupt at 1 kHz. The mouse performs all computation internally. The USB payload is a precomputed 4-byte struct, handed to the USB peripheral before the IN token arrives. The host receives a completed report and does nothing except pass it to the HID driver. There is no companion software, no background service, no driver beyond the OS built-in HID stack. Configuration is handled entirely through vDrive on the device itself.

### A.5 Firmware Language and Implementation Standard

All firmware is written in **strict C** (C11), with no C++ and no vendor abstraction layers. Every peripheral is configured by writing directly to its memory-mapped registers as defined in RM0440. This is not a stylistic choice; it is a correctness and determinism requirement. Abstraction layers introduce indirection, hidden branching, and execution paths that are not auditable for timing behaviour. In this firmware, every cycle path is explicit and intentional.

### A.6 vDrive: Zero Software Footprint on the Host

The virtual drive system exists to eliminate companion software entirely. Companion software in mainstream mice is a source of background CPU load, OS-version dependencies, update processes, and latency from settings round-tripping between the PC and device. On the Cobalt-X Zero, configuration is written directly to the device over USB MSC. The device owns its configuration state completely. No host-side process is involved in normal operation. The host's involvement ends the moment the HID descriptor is accepted at enumeration.

---

*Cobalt-X Zero: Firmware & Engineering Datasheet*
