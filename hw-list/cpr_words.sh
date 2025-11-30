#!/usr/bin/env bash
set -euo pipefail

if [ "$#" -ne 2 ]; then
    echo "Usage: $0 <executable1> <executable2>"
    exit 1
fi

EXE1="$1"
EXE2="$2"
INPUT_DIR="./gutenberg"
TMP_DIR=$(mktemp -d)

# Run first executable
"$PWD/$EXE1" "$INPUT_DIR"/* > "$TMP_DIR/exe1.out"

# Run second executable
"$PWD/$EXE2" "$INPUT_DIR"/* > "$TMP_DIR/exe2.out"

# Compare outputs
if diff -uw "$TMP_DIR/exe1.out" "$TMP_DIR/exe2.out"; then
    echo "Outputs are identical ✅"
else
    echo "Outputs differ ❌"
    echo "Temp outputs kept in $TMP_DIR for inspection"
fi

# Clean up
# rm -rf "$TMP_DIR"
