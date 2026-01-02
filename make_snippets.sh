#!/bin/sh

SRC_DIR="src/snippets"
OUT_DIR="include"
OUT_FILE="$OUT_DIR/snippets.hpp"

mkdir -p "$OUT_DIR"

echo "/* Auto-generated file. Do not edit manually. */" > "$OUT_FILE"
echo "#pragma once" >> "$OUT_FILE"
echo "" >> "$OUT_FILE"

for file in "$SRC_DIR"/*.asm; do
    [ -e "$file" ] || continue

    name=$(basename "$file" .asm)
    const_name="snippet_$name"

    echo "const char* $const_name =" >> "$OUT_FILE"

    sed \
        -e 's/\\/\\\\/g' \
        -e 's/"/\\"/g' \
        -e 's/.*/"&\\n"/' \
        "$file" >> "$OUT_FILE"

    echo ";" >> "$OUT_FILE"
    echo "" >> "$OUT_FILE"
done

echo "Generated $OUT_FILE"
