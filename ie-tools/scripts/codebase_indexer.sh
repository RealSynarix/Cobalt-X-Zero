#!/bin/bash

# Configuration:
# - BASE_DIR: Base path to start indexing from (default is script's directory, can use ".." or absolute paths)
# - OUTPUT_SUBDIR: Subfolder relative to BASE_DIR to save codebase.txt (leave empty to save directly in BASE_DIR)
BASE_DIR="/home/alex/LocalProgramming/Cobalt-X Zero/"
OUTPUT_SUBDIR=""

readonly SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"

# Resolve Base Directory
if [ -z "$BASE_DIR" ]; then
    readonly TARGET_BASE="$SCRIPT_DIR"
else
    # Handle relative paths based on script dir or absolute paths
    readonly TARGET_BASE="$(cd "$SCRIPT_DIR" && cd "$BASE_DIR" && pwd)"
fi

# Resolve Output Directory
if [ -n "$OUTPUT_SUBDIR" ]; then
    readonly OUTPUT_DIR="${TARGET_BASE}/${OUTPUT_SUBDIR}"
else
    readonly OUTPUT_DIR="${TARGET_BASE}"
fi

readonly OUTPUT_FILE="$OUTPUT_DIR/codebase.txt"
readonly SCRIPT_NAME=$(basename "$0")

notify() {
    local title="$1"
    local message="$2"
    if command -v notify-send &> /dev/null; then
        notify-send "$title" "$message"
    elif command -v osascript &> /dev/null; then
        osascript -e "display notification \"$message\" with title \"$title\""
    elif command -v powershell.exe &> /dev/null; then
        powershell.exe -Command "New-BTPersonalNotification -NotificationTitle '$title' -NotificationText '$message'" &> /dev/null || \
        powershell.exe -Command "Add-Type -AssemblyName System.Windows.Forms; [System.Windows.Forms.MessageBox]::Show('$message', '$title')" &> /dev/null
    fi
}

main() {
    trap 'notify "Archive Failed" "The script encountered an error."; exit 1' ERR

    if ! command -v tree &> /dev/null; then
        printf "[ERROR] 'tree' command not found.\n" >&2
        exit 1
    fi

    mkdir -p "$OUTPUT_DIR"
    cd "$TARGET_BASE"

    {
        printf "PROJECT ARCHIVE: %s\n" "$(date)"
        printf -- "------------------------------------------------\n\n"
        printf "I. DIRECTORY STRUCTURE\n"
        tree -I '.*|node_modules|*ie-*'
        printf "\n--\n\n"
        printf "II. FILE CONTENTS\n\n"
    } > "$OUTPUT_FILE"

    find . -path '*/node_modules' -prune -o -path '*/.*' -prune -o -name '*ie-*' -prune -o -type f -print | while read -r file; do
        local normalized_file=$(echo "$file" | sed 's|^\./||')

        if [[ "$normalized_file" == *"$SCRIPT_NAME" || "$file" == "$OUTPUT_FILE" ]]; then
            continue
        fi

        {
            printf "================================================\n"
            printf " PATH: %s\n" "$normalized_file"
            printf "================================================\n"
        } >> "$OUTPUT_FILE"

        if [[ "$file" =~ \.png$ || "$(basename "$file")" == "LICENSE" ]]; then
            printf "[Content intentionally omitted for documentation/image format]\n" >> "$OUTPUT_FILE"
        else
            if file "$file" | grep -qE 'text|JSON|source|empty|XML|script'; then
                cat "$file" >> "$OUTPUT_FILE"
            else
                printf "[Binary or non-standard text format skipped]\n" >> "$OUTPUT_FILE"
            fi
        fi
        printf "\n\n" >> "$OUTPUT_FILE"
    done

    notify "Archive Complete" "Process finished successfully."
    printf "[SUCCESS] Archive generated at %s\n" "$OUTPUT_FILE"
}

main "$@"
