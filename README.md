# Cobalt-X Zero

**Open-source • 3D-printable • 50g balanced gaming mouse**

Built around the STM32G431 and PixArt PMW3360.  
No bloated software. Configuration lives entirely in the browser via WebHID.

**Config site:** [Click Me :D](https://realsynarix.github.io/Cobalt-X-Zero/)  
Works in Chrome / Edge (desktop). Plug the mouse in, hit **Connect**, create or edit profiles, save.

---

## Why this exists

Most gaming mice force you to install closed, resource-heavy software.  
Cobalt-X Zero does the opposite:

- Fully open hardware & software (PCB, shell, firmware, configurator)
- Zero host-side software required after initial flash (you need software to flash, but then you can close and optionally delete it)
- Lightweight, relatively cheap, and modifiable by design

Ideal for anyone who wants a high-performance mouse without proprietary software and host resources being hogged.

---

## Specs

| Part            | Detail                                              |
|-----------------|-----------------------------------------------------|
| **MCU**         | STM32G431KBT6 (170 MHz, LQFP-32) |
| **Sensor**      | PixArt PMW3360 |
| **Polling**     | 1000 Hz USB (8 kHz internal loop + SOF sync) |
| **Click latency**| ~2.5 ms |
| **Weight**      | 50 g (without cable) |
| **Switches**    | Kailh Mute Push Buttons (red cap, 2 pin + 10M click life-span) |
| **Wheel**       | TTC EC10 rotary encoder + custom printed wheel |
| **LEDs**        | 2× RGB (Cathode) |
| **Config**      | WebHID Github Pages Website |
| **Shell**       | Custom PETG 3D Print |

---

## Repository Layout

| Path                              | Contents                                              |
|-----------------------------------|-------------------------------------------------------|
| `firmware/`                       | C++ firmware source + pre-built binaries              |
| `hardware/fabrication/pcb/`       | KiCad project, Gerbers, BOM, position files           |
| `hardware/fabrication/shell/`     | 3D printable STLs + print settings                    |
| `docs/guides/`                    | Assembly, flashing, configuration, and general guides |

---

## Getting Started

1. **Read the docs**  
   Start with [`docs/guides/`](docs/guides/) for assembly instructions, print settings, and firmware details.

2. **Print the shell**  
   Use the STLs and recommended settings in `hardware/fabrication/shell/`.

3. **Order & assemble the PCB**  
   - Fabricate from the Gerbers in `hardware/fabrication/pcb/` (JLCPCB works well).  
   - The PMW3360 sensor must be purchased separately (e.g. AliExpress) and hand-soldered.

4. **Flash the firmware**  
   Follow the flashing guide in `docs/guides/`. Pre-built binaries are available for quick setup; source is there if you want to modify it.

5. **Configure**  
   Open the [config site](https://realsynarix.github.io/Cobalt-X-Zero/), connect the mouse, create profiles, and tune everything to your preference. Multiple profiles are supported.

---

## License & Contributions

Everything in this project is open source.  
You are free to fork, modify, improve, and share your own builds.

**Attribution is appreciated.**  
If you use Cobalt-X Zero commercially or base a project on it, please include a link back to this repository.

Issues and pull requests are always welcome.
