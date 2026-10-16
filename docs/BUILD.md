# Erstellen und Installieren (Desktop + Android)

Dies ist eine detaillierte Anleitung zum Erstellen und Installieren des Spiels auf Desktop-Plattformen (Linux/macOS/Windows) und Android (Debug-Build). Die neuen Header-Dateien und Ordnerstrukturen benötigen in den Build-Skripten keine Änderungen, abgesehen von den Pfaden in CMake.

## 1. Desktop (CMake)

### Voraussetzungen

- CMake ≥ 3.11 (später können Sie die System-Installation verwenden, später CMake 3.22 herunterladen)
- Ein C99-kompatibler Compiler (gcc, clang, MSVC)
- Eine Internetverbindung (raylib wird automatisch heruntergeladen)

### Schritte zum Erstellen des Projekts

Gehen Sie zur Projektwurzel (wo sich die Datei `CMakeLists.txt` befindet) und öffnen Sie ein Terminal.

#### Standardsetup

```bash
# Generiert den Makefile (oder NMake, Visual Studio)
cmake .

# Baut die ausführbare Datei (Standardname: "spiel")
make
```

Das Standardausgabebinary:
- Linux/macOS: `./spiel`
- Windows (msvc): `Debug\spiel.exe` oder `Release\spiel.exe`

#### Reiner Build (Beseitigt versehentliche Build-Artefakte)

Die bereitgestellte `run.sh` leert Build-Artefakte, generiert neu und erstellt:

```bash
./run.sh
```

#### Anpassung der Build-Konfiguration

Sie können `CMAKE_BUILD_TYPE` über die Befehlszeile oder mit `-D`-Optionen setzen:

```bash
cmake -DCMAKE_BUILD_TYPE=Debug .
make
```

Mögliche Werte: `Debug`, `Release`, `RelWithDebInfo`, `MinSizeRel`.

#### Anpassen der Fenstergröße

Die Desktop-Spielfenstergröße wird in `src/shared/game.c` (Desktop-Fallback) auf 1080×720 festgelegt.

Sie können dies über `cmake` anpassen, indem Sie die Variable `-DGAME_SCREEN_WIDTH=1920 -DGAME_SCREEN_HEIGHT=1080` verwenden und dann in `main.c` überprüfen (Desktop) bzw. `android_main.c` für Android.

#### Anpassen der Raylib-Quelldateien

Wenn Sie raylib aus Ihrer eigenen lokalen Kopie verwenden möchten:

1. Clone die raylib-Repo (`https://github.com/raysan5/raylib.git`)
2. Passen Sie `CMAKE_TOOLCHAIN_FILE` oder `CMAKE_PREFIX_PATH` für `FetchContent` an

Siehe `CMakeLists.txt` (Desktop) und `android/app/src/main/cpp/CMakeLists.txt`.

## 2. Android (Gradle + CMake)

### Voraussetzungen

- Android Studio Arctic Fox oder neuer
- OpenJDK 17+
- Android SDK mit unten aufgeführten SDK-Versionen
- Android NDK r29 oder neuer
- Eine Internetverbindung (Gradle lädt Abhängigkeiten herunter)

### Übersicht

Das Android-Projekt verwendet einen hybriden Build:

- **Gradle** erstellt die APK (`app/build/outputs/apk/debug/app-debug.apk`)
- **CMake** innerhalb des `cpp`-Moduls baut die native Shared Library (`libjumperchicken.so`) aus den selben Quelldateien wie das Desktop-Binary.

### Installieren des Android SDK/NDK (erste Schritte)

Wenn Sie noch kein Android SDK haben, verwenden Sie das Android Studio SDK-Manager:

```bash
# Beispielbefehl (Linux)
/…/Android/Sdk/cmdline-tools/latest/bin/sdkmanager \
    "platforms;android-36" \
    "build-tools;36.0.0" \
    "ndk;29.0.14206865" \
    "cmake;3.31.6"
```

Stellen Sie sicher, dass `ANDROID_HOME` (`$HOME/Android/Sdk`) definiert ist und den `cmdline-tools`-Pfad enthält.

### Erstellen des Android-Projekts

1. Gehen Sie zum `android/`-Ordner.

2. Erstellen Sie eine lokale.properties-Datei (falls Sie es noch nicht getan haben):

```properties
sdk.dir=$HOME/Android/Sdk
```

3. Führen Sie `gradlew clean assembleDebug` aus.

```bash
cd android
./gradlew clean assembleDebug
```

Die APK befindet sich unter:
`android/app/build/outputs/apk/debug/app-debug.apk`

4. Installieren Sie sie auf einem angeschlossenen Gerät oder Emulator:

```bash
adb install -r app/build/outputs/apk/debug/app-debug.apk
```

### Anpassen der Gradle-Build-Konfiguration

Alle Konfigurationen befinden sich in `android/build.gradle.kts` (Project) und `android/app/build.gradle.kts` (Module).

#### Ändern der Minimum API-Stufe

Bearbeiten Sie `defaultConfig.minSdk` in `app/build.gradle.kts`.

#### Version und Größe

Passen Sie `versionCode`/`versionName`, `minSdk`, `targetSdk`, `compileSdk` und `ndk` an.

#### Signieren des Releases

Um ein Release zu erstellen, benötigen Sie einen JKS-Signing-Key. Legen Sie `jumperchicken.keystore`, `jumperchicken.keystorePassword`, `jumperchicken.keyAlias`, `jumperchicken.keyPassword` in `~/.gradle/gradle.properties` (`android/gradle.properties`) fest:

```properties
jumperchicken.keystore=/pfad/zum/release.jks
jumperchicken.keystorePassword=geheim
jumperchicken.keyAlias=schlüssel0
jumperchicken.keyPassword=geheim
```

Erstellen Sie dann mit `./gradlew bundleRelease` (APK mit Google Play Instant App-Modus).

Für die Veröffentlichung im Play Store benötigen Sie auch die AdMob-App-ID, die Werbeecke-ID und die Play Games-Konfiguration (siehe unten).

### Werbung und Google Play Games (Android)

Die Ads und der Leaderboard werden zur Laufzeit über JNI von der nativen `services.c`-Schicht mit `MainActivity.java` (und `JumperChickenApp.java`) verbunden.

#### Aktivieren der Werbung

Bearbeiten Sie `app/src/main/res/values/strings.xml` (oder fügen Sie neue Einträge hinzu), und fügen Sie die IDs in `android/gradle.properties` (oder `local.properties` für die Konfiguration) ein:

```properties
jumperchicken.admobAppId=ca-app-pub-XXXXXXXXXXXXXXXX~YYYYYYYYYY
jumperchicken.admobRewardedId=ca-app-pub-XXXXXXXXXXXXXXXX/ZZZZZZZZZZ
jumperchicken.playGamesAppId=123456789012
jumperchicken.leaderboardId=CgkIxxxxxxxxxxxxEAIQAQ
```

#### Schritte zur Einrichtung

1. Erstellen Sie eine App und einen rewarded Ad Unit in [AdMob](https://admob.google.com/)
2. Richten Sie die GDPR-Nachricht unter "Datenschutz & Nachrichten" ein (falls erforderlich)
3. Führen Sie `./gradlew clean assembleDebug` erneut aus (der Build bindet die IDs ein)
4. Installieren und führen Sie das Spiel aus. Der Ad-Bereich ist sichtbar, wenn ein Ad verfügbar ist.

#### Fehlerbehebung

- Testanzeigen: Verwenden Sie `ad Preview` mit den Debug-Build-Tools, stellen Sie sicher, dass `jumperchicken.admobAppId` und `-admobRewardedId` gesetzt sind
- Sign-in-Probleme: Stellen Sie sicher, dass `playGamesAppId` und `leaderboardId` korrekt sind
- Gradle-Caches: `./gradlew clean` oder löschen Sie `.gradle`-Ordner

## 3. Entwicklung und Tests

### Aktualisieren der Quelldateien

Alle `.c`- und `.h`-Dateien befinden sich unter:

- Desktop: `src/shared/` und `src/platform/desktop/`
- Android: dieselben Quelldateien (Native-Library) verwendet `src/platform/android/android_main.c`

Änderungen an Shared-Libraries erfordern ein Rebuild beider Plattformen (Desktop: `make`, Android: `./gradlew assembleDebug`).

### Tests

Das Projekt enthält keine Unit Tests. Verwenden Sie manuelle Tests auf beiden Plattformen:

- Desktop: Ausführen des Binar"s, Beobachten des Verhaltens
- Android: Ausführen der Debug-APK auf Emulator/Device

### Tools und Verbesserungen

- IDE-Unterstützung: Visual Studio Code (mit C/C++-Erweiterungen), CLion, Android Studio (Native-Development)
- Formatter: clang-format (CMakeLists.txt), optional .editorconfig
- Konfliktlösung: Verwenden Sie `git rebase` für die Historie

## 4. Bereitstellung für Produktion (Desktop)

#### Kopieren der benötigten Bibliotheken

- raylib ist eine Header-only-Bibliothek mit statischen Libraries (`libraylib.a`, `libraylib.so`), die innerhalb des Builds über `FetchContent` heruntergeladen werden; die Anwendungen sind eigenständig.

#### Nicht mehr benötigte Dateien bereinigen

- `.git` Verzeichnis auskoppeln (optional)
- `src/shared/objects.c` Kopieren und weitere Dateien (falls nicht benötigt)
- `data/` Ordner, `README.md`, `run.sh`
- `CMakeLists.txt` und `android/` kann weggelassen werden, wenn Sie nur das Desktop-Binary wollen

#### Bereitstellungspakete

- Windows: `spiel.exe` (kopieren Sie raylib-DLLs, falls benötigt)
- Linux: `./spiel` (benötigt raylib-DLL, falls statisch)
- macOS: `./spiel` (benötigt raylib .dylib, falls statisch)

## 5. Weiterführende Literatur / Nützliche Ressourcen

- raylib: `https://raylib.com/`
- Android NDK CMake: `https://developer.android.com/ndk/guides/cmake`
- Gradle: `https://gradle.org/`
- JNI: Standard Java Native Interface Dokumentation

## Versionshistorie

- **V1.0**: Ursprüngliche Struktur (monolithische `src/` mit `.c`-Includes)
- **V2.0**: Getrennt von Android/Desktop, Header-Dateien hinzugefügt, modulares Layout, Dokumentation hinzugefügt (dieser Ordner `docs/`)

# Lizenz

Dieses Projekt ist lizenziert unter der Apache 2.0 (für den Nativcode) und der Raylib-Lizenz (siehe `raylib/LICENSE`). Einzelne Drittanbieter-Komponenten unterliegen ihren eigenen Lizenzen (z.B. Google Play Games SDK, AdMob SDK).

# Autoren

- bonbon08: Originalkonzept, Entwicklung, Android-Java, Dokumentation
- Unterstützung: CI-Workflow (`.github/workflows/cmake-single-platform.yml`)

# Kontakt

- Fragen/Feature-Anfragen: GitHub Issues
- Meldungen über den Hauptzweck: Die vorhandenen Issue-Templates (bug, feature request)

---

*Die Datei wurde für das neue Refactoring (Version 2.0) erstellt, um die Build-Anweisungen auf jede Plattform zu konzentrieren.*