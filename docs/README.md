# Jumper Chicken

Ein Jump-and-Run-Spiel im Stil des alten Mobilspiels Jumpstun, geschrieben in C mit raylib. Diese Dokumentation beschreibt den refaktorierten Code (Version 2.0) mit klarer Trennung von Desktop- und Android-Plattform, Header-Dateien und modularer Architektur.

## Aufbau des Projekts

| Verzeichnis / Datei | Zweck |
|---|---|
| `android/` | Android-Gradle-Projekt (Shared Library + Java-Brücke) |
| `data/` | Spielressourcen (Bilder, Sounds, Schriftarten) |
| `docs/` | Dokumentation (dieser Ordner) |
| `include/shared/` | Gemeinsame C-Header (`audio.h`, `background.h`, `objects.h`, `menu.h`, `shop.h`, `services.h`, `effects.h`, `game.h`, `filerw.h`, `input.h`) |
| `include/platform/` | Plattformspezifische Header (`android.h`, `desktop.h`) |
| `src/shared/` | Plattformunabhängiger Quellcode (`audio.c`, `background.c`, `effects.c`, `game.c`, `objects.c`, `services.c`, `menu.c`, `shop.c`, `filerw.c`) |
| `src/platform/desktop/` | Desktop-Eintrittspunkt (`main.c`) |
| `src/platform/android/` | Android-Native-Entry (`android_main.c`) |
| `CMakeLists.txt` | Desktop-Build-Konfiguration |
| `android/app/src/main/cpp/CMakeLists.txt` | Android-Build-Konfiguration |

### Neue Struktur (nach Refactoring)

Die alte Struktur hatte alle `.c`-Dateien direkt unter `src/`; `main.c` inkludierte die anderen `.c`-Dateien direkt (`#include "objects.c"`). Das führte zu potenziellen Problemen bei mehrfachen Definitionen und erschwerte die Wartung.

Die neue Struktur:
- Trennung in `include/` und `src/`
- Jede `.c`-Datei hat einen passenden `.h`-Header
- `main.c` wurde in `game.c` (Logik) und `platform/desktop/main.c` (Einstieg) aufgeteilt
- `android_main.c` enthält nur den plattformspezifischen Einstieg (`android_main()`)
- `CMakeLists.txt` und Android-CMake referenzieren die neuen Pfade

## Anforderungen und Build-Anweisungen

### Desktop (CMake 3.11+, C99-Compiler)

```bash
cmake . -DCMAKE_POLICY_VERSION_MINIMUM=3.5
make
./spiel
```

Das Build-Skript `run.sh` säubert und baut neu:

```bash
./run.sh
```

### Android (Android Studio, JDK 17+, SDK 36, NDK 29.0.14206865)

```bash
cd android
echo "sdk.dir=$HOME/Android/Sdk" > local.properties
./gradlew assembleDebug
adb install -r app/build/outputs/apk/debug/app-debug.apk
```

Die APK befindet sich unter `android/app/build/outputs/apk/debug/app-debug.apk`. Für Release benötigst du die Gradle-Properties (siehe `README` im `android/`-Ordner).

## Dokumentation parallel zur Codebase

Jedes Modul verfügt über eine Dokumentation im `docs/`-Ordner und über Header-Kommentare im Quellcode:

- `docs/ARCHITEKTUR.md` – Modul-Übersicht und Datenfluss
- `docs/BUILD.md` – Detaillierte Build-Anweisungen
- `docs/CONTROLS.md` – Steuerung für Desktop und Android
- `docs/ADS.md` – Einrichtung von AdMob und Google Play Games (nur Android)
- `docs/REFACTOR.md` – Was sich beim Refactoring geändert hat

## Steuerung

Siehe `docs/CONTROLS.md` und die kurze Übersicht in der ursprünglichen `README`.
