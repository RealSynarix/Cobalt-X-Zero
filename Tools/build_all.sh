#!/usr/bin/env bash
set -euo pipefail
ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
FQBN="${FQBN:-STMicroelectronics:stm32:GenG4}"
CLI="${ARDUINO_CLI:-arduino-cli}"
OUT="$ROOT/build"
HID_SRC="$ROOT/HID-Firmware"
BOOT_SRC="$ROOT/Boot-Select"
VDRIVE_SRC="$ROOT/vDrive-Firmware"
HID_OUT="$OUT/hid"
BOOT_OUT="$OUT/boot"
VDRIVE_OUT="$OUT/vdrive"
FLASH_BIN="$OUT/cobalt_x_zero_combined.bin"
rm -rf "$OUT"
mkdir -p "$HID_OUT" "$BOOT_OUT" "$VDRIVE_OUT"
"$CLI" compile -b "$FQBN" --output-dir "$BOOT_OUT" "$BOOT_SRC"
"$CLI" compile -b "$FQBN" --output-dir "$HID_OUT" "$HID_SRC"
"$CLI" compile -b "$FQBN" --output-dir "$VDRIVE_OUT" "$VDRIVE_SRC"
HID_BIN="$(find "$HID_OUT" -maxdepth 1 -name '*.bin' | head -n1)"
BOOT_BIN="$(find "$BOOT_OUT" -maxdepth 1 -name '*.bin' | head -n1)"
VDRIVE_BIN="$(find "$VDRIVE_OUT" -maxdepth 1 -name '*.bin' | head -n1)"
python3 - "$FLASH_BIN" "$BOOT_BIN" "$HID_BIN" "$VDRIVE_BIN" <<'PY'
import sys
from pathlib import Path
out_path = Path(sys.argv[1])
boot = Path(sys.argv[2]).read_bytes()
hid = Path(sys.argv[3]).read_bytes()
vdrive = Path(sys.argv[4]).read_bytes()
FLASH_SIZE = 128 * 1024
BASE_ADDR = 0x08000000
BOOT_ADDR   = 0x08000000
HID_ADDR    = 0x08004000
VDRIVE_ADDR = 0x08014000
image = bytearray([0xFF]) * FLASH_SIZE
def place(addr, data):
    offset = addr - BASE_ADDR
    if offset < 0 or offset + len(data) > FLASH_SIZE:
        sys.exit(1)
    image[offset:offset + len(data)] = data
place(BOOT_ADDR, boot)
place(HID_ADDR, hid)
place(VDRIVE_ADDR, vdrive)
out_path.write_bytes(image)
PY
