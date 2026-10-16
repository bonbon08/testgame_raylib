# Architektur

Dieses Dokument beschreibt die Modulstruktur des Jumper Chicken Spiels nach dem Refactoring (Version 2.0).

## Überblick

Das Spiel folgt einer geschichteten Architektur:
1. Plattform-Eintrittspunkt (Desktop: `main.c`, Android: `android_main.c`)
2. Spiel-Logik und -Zustand (gemeinsam: `game.c`)
3. Kernmodule (gemeinsam, plattformunabhängig)
4. Plattform-abstrahierende Schnittstellen (Header)

## Modulübersicht

| Modul | Dateien | Verantwortlichkeiten |
|---|---|---|
| **Eintrittspunkt** | `src/platform/desktop/main.c`, `src/platform/android/android_main.c` | Fensterinitialisierung, OpenGL/Kontext, Hauptloop |
| **Spiel-Zustand** | `src/shared/game.c` | Spielschleife, Zustandsübergänge (Menü, Spiel, Tod, Shop) |
| **Eingabe** | `src/shared/input.c` | Tastatur, Maus, Touch-Abilität, virtuelle Skalierung |
| **Objekte** | `src/shared/objects.c` | Huhn, Spikes, Münze, Schwierigkeitsberechnung |
| **Menü** | `src/shared/menu.c` | Hauptmenü, Todesbildschirm, UI-Zeichnung |
| **Shop** | `src/shared/shop.c` | Ingame-Shop, Charaktere, Hintergründe, Musik, Münzverwaltung |
| **Audio** | `src/shared/audio.c` | Chiptune-Synthese, Jump-Sound, Musikwiedergabe |
| **Hintergrund** | `src/shared/background.c` | Themen (Day, Sunset, Night, Candy), Wolken, Berge, Sterne |
| **Effekte** | `src/shared/effects.c` | Partikel, Popups, Bildschirmschüttelung |
| **Services** | `src/shared/services.c` | Werbung (AdMob), Leaderboard (Play Games), I/O-Abstraktion |
| **Dateisystem** | `src/shared/filerw.c` | Spielstand speichern/laden (key=value Format) |
| **Ressourcen** | `data/*` | Bilder (PNG), Sounds (WAV) |

## Plattformabstraktion

### Gemeinsame Schnittstellen (`include/shared/`)
- `input.h`: `PointerPosition()`, `PointerPressedIn()`, `BackPressed()`, `InitVirtualScreen()`
- `audio.h`: `InitGameAudio()`, `UpdateGameAudio()`, `PlayJumpSound()`
- `background.h`: `InitBackground()`, `UpdateBackground()`, `DrawBackground()`, `DrawThemePreview()`
- `menu.h`: `DrawMainMenu()`, `DrawDeathScreen()`
- `shop.h`: `LoadGame()`, `SaveGame()`, `UpdateShop()`, `DrawShop()`, `CurrentCharacterTexture()`
- `services.h`: `InitAds()`, `ShowRewardedAd()`, `ConsumeAdReward()`, `ShowLeaderboard()`
- `filerw.h`: `LoadSave()`, `WriteSave()`
- `objects.h`: Zugriff auf Spielzustand (`GetChicken()`, `GetGamevars()`, etc.)
- `effects.h`: `SpawnParticles()`, `SpawnPopup()`, `StartShake()`, `UpdateEffects()`, `UpdateShake()`, `DrawEffects()`

### Plattformspezifische Implementierungen
Die C-Dateien in `src/shared/` enthalten `#if defined(PLATFORM_ANDROID)` Blöcke, die je nach Build-Konfiguration (`PLATFORM_ANDROID` definiert durch CMake) die richtige Implementierung wählen.

#### Android
- Nutzt `android_native_app_glue` für Input, Fenster und Lebenszyklus
- Werbung und Leaderboard über JNI-Brücke zu `MainActivity.java`
- Interner Speicher für Dateizugriff (`chdir()` zum App-internen Speicher)

#### Desktop
- Normales Fenster über raylib `InitWindow()`
- Virtueller Bildschirm für skalierbare Auflösung
- Lokale Dateisystemzugriffe

## Datenfluss

1. **Start**: `main()` bzw. `android_main()` initialisiert Fenster, Audio, Input
2. **Spielzustand**: Lädt Savegame, setzt Anfangszustand (Menü aktiv)
3. **Hauptloop** (60 FPS):
   - Eingabe verarbeiten
   - Spielzustand updaten (je nach Modus: Spiel, Menü, Tod, Shop)
   - Hintergrund, Objekte, Effekte zeichnen
   - UI über Zeichnen (Menü, Shop, Punkteanzeige)
4. **Beenden**: Ressourcen freigeben, Fenster schließen, Audio beenden

## Abhängigkeiten

- **raylib 4.5**: Fenster, OpenGL-Kontext, Input, Audio, Zeichnung, Zeitmanagement
- **std**: `malloc`, `stdio` (nur für Logging)
- **Android-only**: `android_native_app_glue`, `jni` (für Werbung/Leaderboard)

## Thread-Safety

- Der Spiel- und Renderloop läuft auf dem Hauptthread (einzelthreadig)
- Audio-Callback läuft auf einem separaten Audio-Thread (raylib intern)
- Werbung/Leaderboard-Callbacks laufen auf dem Android UI-Thread (über JNI)

Die gemeinsamen Module sind nicht threadsicher, da sie nur vom Hauptthread aufgerufen werden.

## Erweiterungspunkte

- Neues Hintergrund-Thema: `themes[]` in `background.c` ergänzen, `NUM_THEMES` erhöhen
- Neue Musik: `tracks[]` in `audio.c` ergänzen, `NUM_TRACKS` erhöhen
- Neuer Charakter: `characters[]` in `shop.c` ergänieren, `NUM_CHARACTERS` erhöhen
- Neue Sprite/Grafik: Unter `data/` platzieren, Pfad in Quellcode anpassen
- Neue Spielmechanik: In `game.c` oder neuen Modulen hinzufügen