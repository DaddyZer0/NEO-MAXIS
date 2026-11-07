#!/usr/bin/env bash

set -e

GAME="$HOME/Games/SimCity2000/SC2K"
PROJECT="$HOME/Projects/NeoMaxis"
PREFIX="$HOME/.local/share/wineprefixes/neomaxis-test"

echo "======================================"
echo "       NEO-MAXIS LAUNCHER"
echo "======================================"

# Verify game installation
if [ ! -f "$GAME/SIMCITY.EXE" ]; then
    echo "ERROR: SIMCITY.EXE not found."
    exit 1
fi

# Verify sc2kfix
if [ ! -f "$GAME/winmm.dll" ]; then
    echo "ERROR: sc2kfix winmm.dll not found."
    exit 1
fi

# Verify NEO-MAXIS mod
if [ ! -f "$GAME/mods/neomaxis.dll" ]; then
    echo "ERROR: NEO-MAXIS DLL not found in mods directory."
    exit 1
fi

# Verify Wine prefix
if [ ! -d "$PREFIX" ]; then
    echo "ERROR: Wine prefix not found."
    exit 1
fi

# Verify project build
if [ ! -f "$PROJECT/build-cmake/neomaxis.dll" ]; then
    echo "ERROR: NEO-MAXIS build not found."
    exit 1
fi

# Verify installed mod matches current build
if ! cmp -s "$PROJECT/build-cmake/neomaxis.dll" "$GAME/mods/neomaxis.dll"; then
    echo "ERROR: Installed mod differs from current build."
    echo "Reinstall the latest DLL before launching."
    exit 1
fi

# Configure Wine
export WINEPREFIX="$PREFIX"
export WINEDLLOVERRIDES="winmm=n,b"
export WINEDEBUG=-all

# Set working directory
cd "$GAME"

# Preserve previous logs
for LOG in neomaxis-calendar.log neomaxis-economy.log; do
    if [ -f "$LOG" ]; then
        cp "$LOG" "$LOG.previous"
        rm "$LOG"
    fi
done

echo
echo "Game:       SimCity 2000 Special Edition"
echo "Mod:        NEO-MAXIS"
echo "Wine:       $WINEPREFIX"
echo "DLL:        mods/neomaxis.dll"
echo
echo "Launching game..."

nohup wine SIMCITY.EXE -skipintro \
    > "$HOME/neomaxis-mod-test.log" 2>&1 < /dev/null &

echo "Launch command submitted."
echo "Logs:"
echo "  $GAME/neomaxis-calendar.log"
echo "  $GAME/neomaxis-economy.log"
echo
echo "======================================"
