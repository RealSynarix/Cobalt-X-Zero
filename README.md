# Cobalt-X Zero

An open source, 3D-printable, 50g balanced custom gaming mouse. Built around STM32G431 and PMW3360. The mouse uses WebHID rather than traditional software for configuration, and has a vast array of configs to modify.

**Config site:** https://realsynarix.github.io/Cobalt-X-Zero/ - works in Chrome / Edge desktop. Plug in, hit Connect, edit, save.

### Why this exists

Most mice require closed software that uses host resources. This one doesn't. Additionally, Everything is open. PCB, shell, firmware, configurator. You can build it, change it, and its ideal for people who hate proprietary peripherals.

### Mouse Specs

| Part          | Detail                                      |
|---------------|---------------------------------------------|
| MCU           | STM32G431KBT6 (170 MHz, LQFP-32)            |
| Sensor        | PixArt PMW3360                              |
| Polling       | 1000 Hz USB (8 kHz internal + SOF sync)     |
| Click latency | ~2.5 ms (switch → USB report)               |
| Weight        | 50 g (without cable)                        |
| Switches      | Kailh Mute push buttons                     |
| Wheel         | EC10 encoder + custom printed wheel         |
| LEDs          | 2× RGB (TIM1 PWM on PA8/9/10)               |
| Config        | WebHID Website hosted on github pages       |
| Shell         | 6-part PETG (0.25 mm layer, tree supports)  |

### Repo layout

| Folder | What is inside |
| --- | --- |
| `/firmware` | C++ firmware for flashing (includes both source and compiled binaries) |
| `hardware/fabrication/pcb/` | Files for KiCAD + Gerbers and other fab files |
| `hardware/fabrication/shell/` | 3D models to print + info|
| `docs/guides/` | Assembly, firmware flashing, configuration, general info |

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
