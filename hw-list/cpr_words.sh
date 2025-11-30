#!/usr/bin/env bash
set -euo pipefail

if [ "$#" -lt 2 ] || [ "$#" -gt 3 ]; then
    echo "Usage: $0 <executable1> <executable2> [N]"
    echo "  N = number of input files to sample (with replacement; default 10)"
    exit 1
fi

EXE1="$1"
EXE2="$2"
N="${3:-10}"

INPUT_DIR="./gutenberg"
TMP_DIR=$(mktemp -d)

# Sample N filenames from INPUT_DIR/* with replacement
# Safe even for large N because it's only producing a list, not passing yet.
mapfile -t FILES < <(ls "$INPUT_DIR"/* | shuf -r -n "$N")

##############################################
# Run EXE1 with timing
##############################################
START1=$(date +%s%3N)
"$PWD/$EXE1" "${FILES[@]}" > "$TMP_DIR/exe1.out"
END1=$(date +%s%3N)
TIME1=$((END1 - START1))

##############################################
# Run EXE2 with timing
##############################################
START2=$(date +%s%3N)
"$PWD/$EXE2" "${FILES[@]}" > "$TMP_DIR/exe2.out"
END2=$(date +%s%3N)
TIME2=$((END2 - START2))


##############################################
# Compare outputs
##############################################
if diff -uw "$TMP_DIR/exe1.out" "$TMP_DIR/exe2.out" > /dev/null; then
    echo "Outputs are identical ✅"
    echo ""
    echo "Timing:"
    printf "  %-15s %8d ms\n" "$EXE1" "$TIME1"
    printf "  %-15s %8d ms\n" "$EXE2" "$TIME2"

    # Clean up on success
    rm -rf "$TMP_DIR"
else
    echo "Outputs differ ❌"
    echo "Temp outputs kept in: $TMP_DIR"
    exit 1
fi

# Clean up
# rm -rf "$TMP_DIR"
