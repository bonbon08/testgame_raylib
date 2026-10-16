cmake_minimum_required(VERSION 3.11)
project(JumperChicken C)

set(CMAKE_C_STANDARD 99)

include(FetchContent)
FetchContent_Declare(
    raylib
    GIT_REPOSITORY https://github.com/raysan5/raylib.git
    GIT_TAG        master
)

set(BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(raylib)

# Sammle alle Quelldateien im shared/-Verzeichnis
file(GLOB SHARED_SRC "src/shared/*.c")

# Plattform-spezifischer Einstiegspunkt
if(PLATFORM_ANDROID)
    set(PLATFORM_SRC "src/platform/android/android_main.c")
else()
    set(PLATFORM_SRC "src/platform/desktop/main.c")
endif()

add_executable(spiel ${SHARED_SRC} ${PLATFORM_SRC})
target_include_directories(spiel PRIVATE include include/shared)
target_link_libraries(spiel PRIVATE raylib)