# Cobalt-X Zero

Open source, 3D-printable, 50g balanced mouse. Built around STM32G431 and PMW3360. No driver bloat, config lives in the browser via WebHID.

**Config site:** https://realsynarix.github.io/Cobalt-X-Zero/ - works in Chrome / Edge desktop. Plug in, hit Connect, edit, save.

### Why this exists

Most mice ship with heavy software. This one doesn't. Everything is open. PCB, shell, firmware, configurator. You can build it, change it, and it stays light on the host.

Weight is exactly 50g without cable. Balance is centered so it doesn't pull to one side.

### Quick specs

| Part | Detail |
| --- | --- |
| MCU | STM32G431KBT6, 170MHz, LQFP-32 |
| Sensor | PixArt PMW3360, SPI, SROM 0x04 |
| Polling | 1000Hz USB, 8000Hz internal loop + SOF sync |
| Click latency | ~2.5ms measured from switch to USB report |
| Weight | 50g without cable |
| Switches | Kailh GM 8.0 for LMB/RMB/MMB, silent tactile for side + wheel |
| Wheel | EC10 encoder + custom printed wheel |
| LEDs | 2x RGB, TIM1 PWM on PA8/9/10 |
| Config | WebHID, VID 0x1209 PID 0xC0BA, Report ID 2, 64 bytes |
| Shell | 6 parts, PETG, tree support, 0.25mm layer |

### Repo layout

| Folder | What is inside |
| --- | --- |
| `firmware/source` | C++ source firmware for devs |
| `firmware/build/` | Built binaries, flash these for consumer use |
| `hardware/fabrication/pcb/` | KiCad PCB, Gerber zip, BOM, positions, netlist |
| `hardware/fabrication/shell/` | STLs for bottom, top, wheel, side buttons, plungers, holder |
| `docs/guides/` | Assembly, firmware flashing, configuration, main index |

### Getting started

1. Read `docs/guides/README.md` first.
2. Print shell using `hardware/fabrication/shell/readme/settings.txt`.
3. Post-process using `post-processing.txt`.
4. Order PCB from `hardware/fabrication/pcb/Cobalt-X_Zero_rev-3.zip` + BOM.
5. Flash firmware, see `docs/guides/firmware.md` (source for dev, binaries in `firmware/build/` for consumer).
6. Assemble per `docs/guides/assembly.md` but flash before closing top shell.
7. Open config site at https://realsynarix.github.io/Cobalt-X-Zero/ and tune.

### Links

- Config site: https://realsynarix.github.io/Cobalt-X-Zero/
- License: MIT for firmware and site, CC BY-SA 4.0 for hardware.

### Status

Rev 3 works. Two known PCB issues listed in `hardware/fabrication/pcb/README.md`. You can build as-is.