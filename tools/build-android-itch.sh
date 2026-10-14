#!/usr/bin/env bash
# Builds the ad-free, signed Android APK for itch.io into dist/JumperChicken-android.apk.
#
# Usage: tools/build-android-itch.sh
#   SIGNING=/path/to/itch-signing.properties   (default: ~/.jumperchicken/itch-signing.properties)
#   The file contains jumperchicken.keystore, .keystorePassword, .keyAlias and .keyPassword.
set -euo pipefail

ROOT="$(cd "$(dirname "$0")/.." && pwd)"
SIGNING="${SIGNING:-$HOME/.jumperchicken/itch-signing.properties}"

if [ ! -f "$SIGNING" ]; then
    echo "Signing file $SIGNING not found." >&2
    exit 1
fi

# Hand the signing properties to Gradle as environment variables, so passwords don't show up
# in the process list
args=()
while IFS='=' read -r key value; do
    [[ -z "$key" || "$key" == \#* ]] && continue
    args+=("ORG_GRADLE_PROJECT_$key=$value")
done < "$SIGNING"

extra=()
[ -d "$ROOT/_deps/raylib-src" ] && extra+=("-Pjumperchicken.raylibDir=$ROOT/_deps/raylib-src")

cd "$ROOT/android"
env "${args[@]}" ./gradlew assembleRelease -q -Pjumperchicken.adsEnabled=false "${extra[@]}"

mkdir -p "$ROOT/dist"
cp app/build/outputs/apk/release/app-release.apk "$ROOT/dist/JumperChicken-android.apk"
echo "-> dist/JumperChicken-android.apk"
