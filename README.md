## TODO (RM only):
-- Correct Title Part
-- Correct Specs
-- Correct Dates/Information (of such)
-- Clean up wording
-- Correct info (regarding firmware and stuff)

# Cobalt-X Zero — v1.0.0
### Fully Custom Ultralight - 50g, Fully Open-Source, Zero-Software (Includes a VFS for config), Optimized Firmware, Gaming Mouse (built by a kid).

Firmware -- [![License: GPL v3](https://img.shields.io/badge/License-GPLv3-blue.svg)](https://www.gnu.org/licenses/gpl-3.0)

Hardware (PCB & 3D Models) -- [![License: CC BY-SA 4.0](https://img.shields.io/badge/License-CC%20BY--SA%204.0-lightgrey.svg)](https://creativecommons.org/licenses/by-sa/4.0/)

**Cobalt-X Zero** is a high-performance, C++ based engineering project designed to prove that top tier gaming peripherals don't need bloated, proprietary software and a big brand. In doing so even if it doesn't have fancy 8000hz polling rate or wireless design, it still outperforms many mice for a fraction of the cost. Engineered from the ground up to challenge industry standards of latency, weight, and customization, this project represents a few months of self-taught development in PCB design, 3D modeling (mechanical) and embedded systems firmware development.

---

## Technical Details & Performance
Most "pro" mice rely on software tricks and marketing, while Cobalt-X Zero relies on pure performance with no software bloat or a mass marketing gimick.

* **Internal 32kHz Oversampling:** While the USB polls at 1000Hz, the **STM32G4** (170MHz) polls the **PMW3360** sensor at 32,000Hz internally. This allows for advanced hardware-level jitter filtering and superior motion smoothness.
* **DMA-Driven Pipeline:** Implemented Direct Memory Access (DMA) to handle sensor data transfer, bypassing CPU interrupts to ensure the processor is 100% focused on packets and the macro.
* **Eager Debounce Algorithm:** Unlike standard mice that wait ~5ms to verify a click, Cobalt-X Zero triggers on the initial falling edge of the signal for **sub-ms click latency** (note that sub-ms latency is internal, and usually has the delay in the table below).
* **Predictive Packet Readiness:** HID packets are formatted and buffered in advance of the USB/OS request, ensuring data is delivered instantly from the buffer.

## Key Features
* **Zero-Software Config:** FILL ME OUT
* **'ENTER WEIGHT HERE'g Ultra-Lightweight:** Engineered on a **1.2mm FR4 PCB** providing a ~25% weight reduction compared to standard 1.6mm boards while maintaining structural integrity. Along with  a meticulously developed lightweight shell, this mouse is clean and light.
* **Hotswap USB-C:** A true modular interface allowing for custom paracord cables to reduce e-waste and achieve a "wireless" feel.
* **Superior Firmware:** FILL ME OUT
* **HyprX Hardware Macro:** A dedicated hardware toggle that maps the **Rotary Encoder** to rapid-fire LMB/RMB actions with real-time neon yellow LED feedback.
* **Programmable Macro Button:** Other than the HyprX button, the other side button can be programmed for any macro desireable.

## Specifications
| Category | Spec |
| :--- | :--- |
| **MCU** | STM32G431KBT6 (170MHz ARM Cortex-M4) |
| **Sensor** | PixArt PMW3360 (12,000 DPI / 250 IPS / 50G) |
| **Polling Rate** | True 1,000Hz (Stable via length-matched differential pairs + SOF Sync) |
| **Click Latency** | Min: 1ms Max: 8ms Avg: 3.5ms
| **PCB** | 1.2mm FR-4 HASL, Hand-routed with Ground Plane for Signal Integrity |
| **Shell** | Custom PETG Matte-Finish modeled in TinkerCAD + MeshInspector |

## IMPORTANT NOTE
* **Double-Clicks & Latency:** While the firmware registers clicks in nanoseconds, the usb stack unfortunately caps polling at 1ms which means that all clicks are at best 1ms in delay, and at worst 4ms (from testing), with 2.8ms being the average. Also, becasue of how the firmware is written, the design is prone to double clicks, but from my testing there is no real concern on this.

## Open Source & Licensing
This project is built to empower the community through transparency and accessibility. It stands as a testament to the power of self-taught engineering, bridging the gap between digital programming and physical hardware to outperform multi-million/billion dollar corporations through bare-metal efficiency.

I chose licenses that allow this to be open source while keeping this mine and not anyone elses. You are free to remix, edit, and share, provided you credit me and distribute contributions under the same licenses.

---
**Created by Synarix**
*15 Years Old | Melbourne, Australia*
