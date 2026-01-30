#!/usr/bin/env bash
set -e

# Copy this script to your 'build' folder and run it from there.
# Requires: entr (install via your package manager)

# Configuration
SOURCE_DIR=".."
BUILD_DIR="."

# Source files to watch
SOURCE_FILES=(
    "game.c"
    "game.h"
)

# Asset files to watch (relative to SOURCE_DIR/assets/)
ASSET_FILES=(
    "font.ttf"
    "image.png"
    "music.wav"
    "sound.wav"
)

echo "Watching game library sources and assets..."
echo "Sources: ${SOURCE_FILES[*]}"
echo "Assets:  ${ASSET_FILES[*]}"
echo "Press Ctrl+C to stop."
echo ""

# Build the list of files to watch
WATCH_FILES=()
for src in "${SOURCE_FILES[@]}"; do
    WATCH_FILES+=("${SOURCE_DIR}/${src}")
done
for asset in "${ASSET_FILES[@]}"; do
    WATCH_FILES+=("${SOURCE_DIR}/assets/${asset}")
done

# Use entr to watch files. The /_ placeholder is replaced with the changed file path.
printf '%s\n' "${WATCH_FILES[@]}" | entr -c sh -c '
    CHANGED_FILE="$1"
    SOURCE_DIR=".."
    BUILD_DIR="."
    
    # Check if the changed file is an asset
    if echo "$CHANGED_FILE" | grep -q "/assets/"; then
        ASSET_NAME=$(basename "$CHANGED_FILE")
        echo "=== Asset changed: $ASSET_NAME ==="
        make copy_assets
    else
        echo "=== Source changed: $(basename "$CHANGED_FILE") ==="
        echo "Rebuilding libgame.so..."
        make game
        echo "Build complete."
    fi
' _ /_

