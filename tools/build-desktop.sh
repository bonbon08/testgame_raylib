#!/usr/bin/env bash
# Builds the desktop release zips for itch.io with Zig as cross compiler:
#   dist/JumperChicken-linux.zip    (x86_64, runs on glibc 2.28+, i.e. most distros since 2018)
#   dist/JumperChicken-windows.zip  (x86_64, Windows 10+)
#
# Usage: tools/build-desktop.sh
#   ZIG=/path/to/zig        Zig compiler (default: zig from PATH)
#   RAYLIB_SRC=/path/to/src raylib's src/ folder (default: _deps/raylib-src/src from the CMake build)
#   VERSION=1.0             version written into the zip names' README
set -euo pipefail

ROOT="$(cd "$(dirname "$0")/.." && pwd)"
ZIG="${ZIG:-zig}"
RAYLIB_SRC="${RAYLIB_SRC:-$ROOT/_deps/raylib-src/src}"
VERSION="${VERSION:-1.0}"
DIST="$ROOT/dist"
WORK="$DIST/.build"

if [ ! -f "$RAYLIB_SRC/raylib.h" ]; then
    echo "raylib sources not found in $RAYLIB_SRC (run the CMake build once, or set RAYLIB_SRC)" >&2
    exit 1
fi

RAYLIB_FILES=(rcore.c rshapes.c rtextures.c rtext.c rmodels.c raudio.c rglfw.c)
# Clipboard images would need libX11 at link time; the game does not use them
COMMON_FLAGS=(-std=gnu99 -O2 -DNDEBUG -DPLATFORM_DESKTOP -DGRAPHICS_API_OPENGL_33 -DSUPPORT_CLIPBOARD_IMAGE=0
              -I"$RAYLIB_SRC" -I"$RAYLIB_SRC/external/glfw/include" -I"$ROOT/src")

# GLFW only needs the X11 headers at compile time; the libraries are loaded at runtime
X11_INCLUDE="$WORK/x11-include"
mkdir -p "$X11_INCLUDE"
ln -sfn /usr/include/X11 "$X11_INCLUDE/X11"

build() {
    local name="$1" target="$2" exe="$3"
    shift 3
    local out="$WORK/$name"
    rm -rf "$out" && mkdir -p "$out/obj" "$out/JumperChicken/data"
    echo "== $name ($target)"
    local pids=()
    for file in "${RAYLIB_FILES[@]}"; do
        "$ZIG" cc -target "$target" "${COMMON_FLAGS[@]}" "${EXTRA_CFLAGS[@]}" -w -c "$RAYLIB_SRC/$file" -o "$out/obj/${file%.c}.o" &
        pids+=($!)
    done
    for pid in "${pids[@]}"; do wait "$pid"; done   # fails the script if any file failed
    "$ZIG" cc -target "$target" "${COMMON_FLAGS[@]}" "${EXTRA_CFLAGS[@]}" -Wall -c "$ROOT/src/main.c" -o "$out/obj/main.o"
    "$ZIG" cc -target "$target" -s "$out"/obj/*.o -o "$out/JumperChicken/$exe" "$@"   # -s: no debug symbols

    cp "$ROOT"/data/*.png "$ROOT"/data/*.wav "$out/JumperChicken/data/"
    sed "s/@VERSION@/$VERSION/" "$ROOT/tools/release-readme.txt" > "$out/JumperChicken/README.txt"
    rm -f "$out/JumperChicken/"*.pdb   # debug symbols from the Windows linker
    (cd "$out" && rm -f "$DIST/JumperChicken-$name.zip" && zip -qr "$DIST/JumperChicken-$name.zip" JumperChicken)
    echo "   -> dist/JumperChicken-$name.zip"
}

mkdir -p "$DIST"

EXTRA_CFLAGS=(-D_GNU_SOURCE -D_GLFW_X11 -isystem "$X11_INCLUDE")
build linux x86_64-linux-gnu.2.28 JumperChicken -lm -ldl -lpthread

EXTRA_CFLAGS=()
build windows x86_64-windows-gnu JumperChicken.exe -lopengl32 -lgdi32 -lwinmm -lshell32 -Wl,--subsystem,windows
