# Refactoring-Protokoll (Version 2.0)

Dieser Abschnitt beschreibt die Änderungen, die beim Refactoring des Jumper Chicken Projekts vorgenommen wurden.

## Ziele des Refactorings

- Trennung von Plattform-spezifischem Code (Desktop / Android)
- Einführung von Header-Dateien und modularen Schnittstellen
- Verbesserung der Wartbarkeit und Erweiterbarkeit
- Dokumentation parallel zur Codebase (diese Dateien unter `docs/`)
- Klare Trennung von Spiel-Logik und Einstiegspunkt

## Änderungen

### 1. Ordnerstruktur

Vorher (V1):
```
src/
  main.c
  objects.c
  input.c
  services.c
  audio.c
  background.c
  effects.c
  menu.c
  shop.c
  Filerw.c
```

Nachher (V2):
```
src/
  shared/                  # Plattformunabhängiger Code
    objects.c, game.c, input.c, effects.c, ...
  platform/
    desktop/
      main.c              # Desktop-Einstieg
    android/
      android_main.c      # Android-Native-Einstieg
include/
  shared/                  # Gemeinsame Header
    audio.h, objects.h, game.h, ...
  platform/                # Plattformspezifische Header
    android.h, desktop.h
docs/
  README.md, BUILD.md, CONTROLS.md, ARCHITEKTUR.md, REFACTOR.md
```

### 2. Header-Dateien

Jeder `.c`-Modul hat jetzt einen passenden `.h`-Header in `include/shared/` oder `include/platform/`.

Beispiele:
- `objects.c` → `include/shared/objects.h`
- `audio.c` → `include/shared/audio.h`
- `services.c` → `include/shared/services.h`
- `game.c` (neu, aus `main.c` extrahiert) → `include/shared/game.h`

### 3. Plattformspezifische Trennung

- `src/platform/desktop/main.c`: Enthält nur die `main()`-Funktion, Fensterinitialisierung und Hauptloop
- `src/platform/android/android_main.c`: Enthält nur `android_main()` und die Native-Loop
- Gemeinsam: `src/shared/game.c` enthält die Spiel-Logik und die Hauptschleife (Platform-unabhängig)

### 4. Entfernung der `.c`-Includes

Vorher: `main.c` enthielt `#include "objects.c"` etc. Das führte zu Problemen bei der Kompilierung und zu möglichen doppelten Definitionen.

Nachher: Jedes Modul wird separat kompiliert und über den Linker verbunden. Das CMake-Link-Setup verbindet alle `.c`-Dateien.

### 5. Dokumentation

Neu erstellt:
- `docs/README.md`: Projektübersicht und Schnellstart
- `docs/BUILD.md`: Detaillierte Build-Anweisungen
- `docs/CONTROLS.md`: Steuerung
- `docs/ARCHITEKTUR.md`: Architektur und Modulübersicht
- `docs/REFACTOR.md`: Diese Datei

### 6. Requirements / Abhängigkeiten dokumentiert

- CMake 3.11+
- C99-Compiler
- raylib (automatisch per FetchContent)
- Android SDK/NDK für Mobile-Build
- AdMob / Play Games IDs für Android-Features (optional)

### 7. Android-Build angepasst

Die Datei `android/app/src/main/cpp/CMakeLists.txt` wurde aktualisiert:
- Pfad zu den Quelldateien verweist auf `src/shared/` und `src/platform/android/`
- `main.c` wurde durch `android_main.c` ersetzt (als Einstiegspunkt der Shared Library)
- Die Java-Brücke (`MainActivity.java`) bleibt unverändert

### 8. Desktop-Build angepasst

`CMakeLists.txt` (Wurzel) wurde aktualisiert:
- `add_executable(spiel ...)` verweist auf `src/platform/desktop/main.c`
- Zusätzliche `.c`-Dateien aus `src/shared/` werden automatisch eingebunden

## Bekannte Einschränkungen / Offene Punkte

- Die alten V1-Dateien (`src/main.c`, `src/objects.c`, etc.) wurden beim Refactoring versehentlich nicht gelöscht und lagen bis zu diesem Commit tot neben der V2-Struktur herum (von keinem Build eingebunden). Sie wurden entfernt.

## Nächste Schritte (für Entwickler)

1. Testen Sie Desktop-Build (`cmake .; make`)
2. Testen Sie Android-Build (`./gradlew assembleDebug`)
3. Aktualisieren Sie `docs/` bei weiteren Änderungen

---
*Version 2.0 – Refactoring abgeschlossen.*