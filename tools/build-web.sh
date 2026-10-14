#!/usr/bin/env bash
# Builds the browser version for itch.io ("HTML" upload) into dist/JumperChicken-web.zip.
#
# Usage: tools/build-web.sh
#   EMSDK=/path/to/emsdk    Emscripten SDK (default: emcc from PATH)
#   RAYLIB_SRC=/path/to/src raylib's src/ folder (default: _deps/raylib-src/src from the CMake build)
set -euo pipefail

ROOT="$(cd "$(dirname "$0")/.." && pwd)"
RAYLIB_SRC="${RAYLIB_SRC:-$ROOT/_deps/raylib-src/src}"
if [ -n "${EMSDK:-}" ]; then source "$EMSDK/emsdk_env.sh" > /dev/null; fi

OUT="$ROOT/dist/.build/web"
rm -rf "$OUT" && mkdir -p "$OUT/obj" "$OUT/site" "$OUT/assets/data"
cp "$ROOT"/data/*.png "$ROOT"/data/*.wav "$OUT/assets/data/"

FLAGS=(-O2 -std=gnu99 -DPLATFORM_WEB -DGRAPHICS_API_OPENGL_ES2 -I"$RAYLIB_SRC" -I"$ROOT/src")

pids=()
for file in rcore.c rshapes.c rtextures.c rtext.c rmodels.c raudio.c; do
    emcc "${FLAGS[@]}" -w -c "$RAYLIB_SRC/$file" -o "$OUT/obj/${file%.c}.o" &
    pids+=($!)
done
for pid in "${pids[@]}"; do wait "$pid"; done
emcc "${FLAGS[@]}" -Wall -c "$ROOT/src/main.c" -o "$OUT/obj/main.o"

emcc "$OUT"/obj/*.o -o "$OUT/site/jumperchicken.html" -O2 \
    -sUSE_GLFW=3 -sALLOW_MEMORY_GROWTH=1 -lidbfs.js \
    --preload-file "$OUT/assets/data@data" \
    --shell-file "$ROOT/tools/web-shell.html"

mv "$OUT/site/jumperchicken.html" "$OUT/site/index.html"   # itch.io starts index.html
(cd "$OUT/site" && rm -f "$ROOT/dist/JumperChicken-web.zip" && zip -qr "$ROOT/dist/JumperChicken-web.zip" .)
echo "-> dist/JumperChicken-web.zip"
