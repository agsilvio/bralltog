#!/usr/bin/env bash
set -e

# copy this script to your 'build' folder and run it from there.

echo "Watching game library sources…"
echo "Rebuilding target: game"
echo "Press Ctrl+C to stop."

# Watch only game-related source/header files
find ../ -maxdepth 1 \
    \( -name 'game.c' -o -name 'game.h' \) | \
    entr -c sh -c '
        echo "=== Rebuilding libgame.so ==="
        make game
    '

