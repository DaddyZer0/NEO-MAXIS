#!/usr/bin/env bash

echo "===== SYSTEM ====="
uname -a

echo
echo "===== DEVELOPMENT TOOLS ====="

for cmd in git gcc g++ cmake ninja wine wine64 \
           winecfg mingw32-g++ mingw64-g++; do
    command -v "$cmd" 2>/dev/null || echo "$cmd: not found"
done

echo
echo "===== SIMCITY 2000 INSTALLATIONS ====="

SEARCH_PATHS=(
    "$HOME/Games"
    "$HOME/.wine/drive_c"
    "$HOME/.local/share/Steam"
    "$HOME/.steam/steam"
    "$HOME/.var/app/com.valvesoftware.Steam"
    "/mnt"
    "/run/media/$USER"
)

for path in "${SEARCH_PATHS[@]}"; do
    [ -d "$path" ] || continue

    find "$path" -maxdepth 7 -type f \
        \( -iname "simcity.exe" \
        -o -iname "scurk.exe" \
        -o -iname "winmm.dll" \) \
        -print 2>/dev/null
done

echo
echo "===== END ====="
