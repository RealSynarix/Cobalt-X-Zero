#!/usr/bin/env bash
set -euo pipefail
ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
CLI="${STM32_PROGRAMMER_CLI:-STM32_Programmer_CLI}"
PORT="${PORT:-usb1}"
BIN="$ROOT/build/cobalt_x_zero_combined.bin"
if [[ ! -f "$BIN" ]]; then
    exit 1
fi
"$CLI" -c port="$PORT" -w "$BIN" 0x08000000 -v -rst
