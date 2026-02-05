#!/usr/bin/env bash
set -e

# Copy this script to your 'build' folder and run it from there.
# Requires: entr (install via your package manager)

# Configuration
SOURCE_DIR=".."
BUILD_DIR="."

# Find all .c files in the source directory and all files in assets/
SOURCE_FILES=$(find "$SOURCE_DIR" -maxdepth 1 -name "*.c" -type f)
ASSET_FILES=$(find "$SOURCE_DIR/assets" -type f 2>/dev/null || true)

echo "Watching game library sources and assets..."
echo "Sources:"
echo "$SOURCE_FILES" | sed 's/^/  /'
echo "Assets:"
echo "$ASSET_FILES" | sed 's/^/  /'
echo ""
echo "Press Ctrl+C to stop."
echo ""

# Build the list of files to watch
WATCH_FILES=$(printf '%s\n' $SOURCE_FILES $ASSET_FILES)

# Use entr to watch files. The /_ placeholder is replaced with the changed file path.
echo "$WATCH_FILES" | entr -c sh -c '
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

