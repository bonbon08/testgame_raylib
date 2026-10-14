cmake_minimum_required(VERSION 3.22)
project(JumperChickenAndroid C)

set(CMAKE_C_STANDARD 99)

# raylib is pinned to the version the game was tested with
include(FetchContent)
set(PLATFORM "Android" CACHE STRING "" FORCE)
set(BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
FetchContent_Declare(
    raylib
    GIT_REPOSITORY https://github.com/raysan5/raylib.git
    GIT_TAG        30fa673ef2fa137588cf8b5732c65d76d86c1570
)
FetchContent_MakeAvailable(raylib)

# The game sources are shared with the desktop build
set(GAME_SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../../../../../src)

add_library(jumperchicken SHARED ${GAME_SOURCE_DIR}/main.c)
target_compile_definitions(jumperchicken PRIVATE PLATFORM_ANDROID)
target_include_directories(jumperchicken PRIVATE
    ${GAME_SOURCE_DIR}
    ${ANDROID_NDK}/sources/android/native_app_glue)
target_link_libraries(jumperchicken PRIVATE raylib android log EGL GLESv2 OpenSLES m)

# NativeActivity looks up this symbol from native_app_glue at startup
target_link_options(jumperchicken PRIVATE -u ANativeActivity_onCreate)
