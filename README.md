# Cobalt-X Zero

An open source, 3D-printable, 50g balanced custom gaming mouse. Built around STM32G431 and PMW3360. The mouse uses WebHID rather than traditional software for configuration, and has a vast array of configs to modify.

**Config site:** https://realsynarix.github.io/Cobalt-X-Zero/ - works in Chrome / Edge browser. Plug it in, hit Connect, add a profile, edit, save.

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
| Wheel         | TTC rotary encoder + custom printed wheel   |
| Lights        | 2× RGB LEDs Cathode                         |
| Config        | WebHID Website hosted on github pages       |

### Repo layout

| Folder | What is inside |
| --- | --- |
| `/firmware` | C++ firmware for flashing (includes both source and compiled binaries) |
| `hardware/fabrication/pcb/` | Files for KiCAD + Gerbers and other fab files |
| `hardware/fabrication/shell/` | 3D models to print + info |
| `docs/guides/` | Assembly, firmware flashing, configuration, general info |

### Getting started

1. Read the docs in /docs/guides for information about the mouse, and instructions for assembly.
2. Order the PCB from whichever fabricator you like (Ive used JLCPCB), and solder some parts yourself (sensor must be self soldered as it must be bought seperately from aliexpress)
3. Flash the firmware to the board, and assemble the mouse.
4. Config however you like and save as many profiles as you like
