# Build Requirements

This document lists the software and packages needed to build Jumper Chicken from source.

## Desktop (Linux, macOS, Windows)

### Core Requirements

| Requirement | Minimum Version | Notes |
|-------------|-----------------|-------|
| CMake | 3.11 | Used to generate build files |
| C99 Compiler | GCC 5+, Clang 3.5+, MSVC 2015+ | Must support C99 standard |
| Internet Connection | - | Required for downloading raylib via FetchContent (first build only) |

### Linux (Debian/Ubuntu)

```bash
# Install build dependencies
sudo apt-get update
sudo apt-get install -y build-essential cmake git libasound2-dev libx11-dev libxrandr-dev libxi-dev libgl1-mesa-dev libglu1-mesa-dev libxcursor-dev libxinerama-dev libwayland-dev libxkbcommon-dev
```

### macOS (using Homebrew)

```bash
# Install build dependencies
brew install cmake pkg-config
# Optional: Install raylib via Homebrew (but FetchContent will download it)
brew install raylib
```

### Windows

1. Install [Visual Studio 2022](https://visualstudio.microsoft.com/vs/) (with Desktop development with C++ workload)
2. Install [CMake](https://cmake.org/download/)
3. Ensure Git is in PATH (for FetchContent)

## Android

### Core Requirements

| Requirement | Minimum Version | Notes |
|-------------|-----------------|-------|
| Android Studio | Arctic Fox (2020.3.1) or newer | Includes SDK, NDK, and CMake |
| JDK | 17 (OpenJDK) | Must be installed and configured in Android Studio |
| Android SDK | API 36 (Android 12.0) | Set via SDK Manager |
| Android NDK | r29 or newer | Set via SDK Manager |
| CMake | 3.22 or newer | Set via SDK Manager |
| Internet Connection | - | Required for Gradle dependency downloads |

### SDK Manager Components

In Android Studio, open SDK Manager and install:

- **SDK Platforms**: Android 12.0 (API 36)
- **SDK Tools**:
  - Android SDK Build-Tools (36.0.0)
  - Android SDK Command-line Tools (latest)
  - Android SDK Platform-Tools (latest)
  - Android NDK (r29 or higher)
  - CMake (3.22.0 or higher)

### Environment Variables

Ensure the following are set (Android Studio usually does this automatically):

- `ANDROID_SDK_ROOT` (or `ANDROID_HOME`): Path to Android SDK
- `ANDROID_NDK_HOME`: Path to Android NDK
- `JAVA_HOME`: Path to JDK 17 installation

## Optional Features

### Google AdMob (Android only)

To enable rewarded ads in release builds, you need:

1. An [AdMob](https://admob.google.com/) account
2. An app and rewarded ad unit created in AdMob
3. Your app ID and ad unit ID added to `android/gradle.properties`:

```properties
jumperchicken.admobAppId=ca-app-pub-XXXXXXXXXXXXXXXX~YYYYYYYYYY
jumperchicken.admobRewardedId=ca-app-pub-XXXXXXXXXXXXXXXX/ZZZZZZZZZZ
```

### Google Play Games (Android only)

To enable leaderboards in release builds, you need:

1. A [Google Play Console](https://play.google.com/console) developer account
2. A game created with Play Games Services enabled
3. A leaderboard created (format: Integer, higher is better)
4. Your Play Games app ID and leaderboard ID added to `android/gradle.properties`:

```properties
jumperchicken.playGamesAppId=123456789012
jumperchicken.leaderboardId=CgkIxxxxxxxxxxxxEAIQAQ
```

## Continuous Integration

The project includes a GitHub Actions workflow (`.github/workflows/cmake-single-platform.yml`) that builds the desktop version on Ubuntu-latest using:

- `sudo apt-get install -y cmake`
- FetchContent for raylib

Android builds are not currently automated in CI due to SDK/NDK size constraints, but can be added using the `android-actions/setup-android` action.

## Troubleshooting

### "raylib not found" during CMake configuration

- Ensure you have an internet connection (FetchContent downloads raylib on first build)
- Try deleting the `CMakeCache.txt` and `CMakeFiles/` directory and re-running CMake
- Check that FetchContent is able to reach `https://github.com/raysan5/raylib.git`

### "No CMAKE_C_COMPILER could be found"

- Install a C99-compatible compiler (GCC, Clang, or MSVC)
- Ensure the compiler is in your PATH
- On Windows, install the "Desktop development with C++" workload in Visual Studio

### Android build fails with "Namespace not found" or "Unable to locate tools"

- Verify that `ANDROID_SDK_ROOT` and `ANDROID_NDK_HOME` are set correctly
- Ensure you have installed the correct SDK/NDK versions via Android Studio SDK Manager
- Invalidate caches and restart Android Studio (`File > Invalidate Caches > Invalidate and Restart`)

---
*This document is part of the documentation suite for Jumper Chicken v2.0 (refactored).*