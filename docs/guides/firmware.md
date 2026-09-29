# Firmware Guide

### Where firmware lives

| Path | What it is |
| --- | --- |
| `firmware/source` | Full C++ source version for development. Flash this if you want to edit, debug, or build custom features. Recommended for dev. |
| `firmware/build/` | Built binaries inside this folder. Flash a binary from here for normal consumer use, no toolchain needed. Recommended for consumer. |

Either works. Dev should flash source, consumer should flash a binary from `firmware/build/`.

### Requirements

| Item | Detail |
| --- | --- |
| For source | Arduino IDE 2.x, STM32 core 3.0.0+, Generic STM32G4 G431KBT6 170MHz |
| For binary | STM32CubeProgrammer or dfu-util |
| Cable | USB-C data cable |

### Flash source

| Step | Action |
| --- | --- |
| 1 | Open `current.ino` from `firmware/source` |
| 2 | Check `build_opt.h` has `-DHAL_PCD_MODULE_ENABLED -DUSBD_USE_HID_COMPOSITE=1 -DUSBD_USE_CUSTOM_HID=1 -DUSBD_VID=0x1209 -DUSBD_PID=0xC0BA -include src/USB-Module/descriptors.h` |
| 3 | Put board in DFU if needed, upload |

### Flash binary from firmware/build/

| Step | Action |
| --- | --- |
| 1 | DFU mode: hold BOOT0 while plugging |
| 2 | Open STM32CubeProgrammer, select USB |
| 3 | Browse to a binary inside `firmware/build/`, e.g. `Cobalt-X_Zero.bin` or `firmware_here.bin` |
| 4 | Start address `0x08000000`, check Verify + Run after programming |
| 5 | Flash, unplug/replug |

### Verification

| Check | Expected |
| --- | --- |
| lsusb | VID 0x1209 PID 0xC0BA Synarix Cobalt-X Zero |
| Chrome HID | Shows Cobalt-X Zero usage page 0xFF00 at https://realsynarix.github.io/Cobalt-X-Zero/ |
| Config site | Connect -> Ping -> Re-Read All works |
| LEDs | Blue breathing after flash |

### Troubleshooting

| Issue | Fix |
| --- | --- |
| Generic / HyprX Device | descriptors.h not included |
| No device | Check HSI48 + CRS, data cable |
| Chrome not showing | Use Chrome/Edge desktop, check navigator.hid |