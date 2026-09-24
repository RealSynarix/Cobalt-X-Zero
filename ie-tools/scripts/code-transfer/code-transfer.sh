#!/bin/bash

set -euo pipefail

DOWNLOADS="/home/alex/Downloads"
TARGET="/home/alex/LocalProgramming/Cobalt-X Zero/firmware/wip-firmware/current/"
TEMP=$(mktemp -d)

trap 'rm -rf "$TEMP"' EXIT

FULL_ZIP="$DOWNLOADS/code-full.zip"
PARTIAL_ZIP="$DOWNLOADS/code-partial.zip"

if [[ -f "$FULL_ZIP" ]]; then
    MODE="full"
    ZIP="$FULL_ZIP"
elif [[ -f "$PARTIAL_ZIP" ]]; then
    MODE="partial"
    ZIP="$PARTIAL_ZIP"
else
    notify-send -u critical "Code Transfer" "Neither code-full.zip nor code-partial.zip found in Downloads"
    exit 1
fi

unzip -o -q "$ZIP" -d "$TEMP"
rm -f "$ZIP"

if [[ "$MODE" == "full" ]]; then
    cd "$TARGET"
    find . -mindepth 1 -maxdepth 1 ! -name '.project' -exec rm -rf {} +
fi

shopt -s dotglob nullglob
items=("$TEMP"/*)

if [[ ${#items[@]} -eq 1 && -d "${items[0]}" ]]; then
    cp -a "${items[0]}"/. "$TARGET"/
else
    cp -a "$TEMP"/. "$TARGET"/
fi
shopt -u dotglob nullglob

notify-send "Code Transfer" "Transfer complete ($MODE)"
