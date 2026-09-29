# Firmware Guide

### Where firmware lives

| Path | What it is |
| --- | --- |
| `/firmware/source` | Full C++ source version for development. Flash this if you want to edit, debug, or build custom features. Recommended for dev. |
| `/firmware/build` | Raw binary files ready to flash directly. NOTE: I have never flashed the firmware like this, so check online how todo it |

Either works. Dev should flash source, consumer should flash bin.

### Requirements

| Item | Detail |
| --- | --- |
| For source | Arduino IDE 2.x, STM32 core 3.0.0+, Generic STM32G4 G431KBT6 170MHz |
| For binary | STM32CubeProgrammer or dfu-util for CLI |
| Cable | USB-C data cable |

### Flash source

| Step | Action |
| --- | --- |
| 1 | Open `current.ino` from `/firmware/source` in the IDE|
| 2 | Ensure the IDE has libs installed (Mouse, Keyboard, PMW3360, STM32 Based Boards). Lib names may not be correct, these are the names tho IIRC, double check espcecially the last one.|
| 3 | Put in board info (mainly the MCU info and DFU upload method, aswell as HID support) |
| 4 | Hold down DFU button under RMB and plug into host device, then press flash |

### Flash binary NOTE: Take with a grain of salt

| Step | Action |
| --- | --- |
| 1 | DFU mode: hold BOOT0 while plugging |
| 2 | Load `/firmware/build/source.ini.bin` at 0x08000000 in CubeProgrammer |
| 3 | Flash, verify, unplug/replug |

### Verification

| Check | Expected |
| --- | --- |
| lsusb | VID 0x1209 PID 0xC0BA Synarix Cobalt-X Zero |
| Chrome HID | Shows Cobalt-X Zero usage page 0xFF00 |
| Config site | Connect -> Ping -> Re-Read All works |
| LEDs | Blue breathing after flash |

### Troubleshooting

| Issue | Fix |
| --- | --- |
| Generic / HyprX Device | descriptors.h not included |
| No device | Check HSI48 + CRS, data cable |
| Chrome not showing | Use Chrome/Edge desktop, check navigator.hid | Ask AI | pretty self explaitory |
| Send an Issue| send an issue to the github, and I can help |
