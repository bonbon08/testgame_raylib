# The game sources are shared with the desktop build
set(GAME_SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../../../../../src)

# Gather common source files
file(GLOB SHARED_SRC ${GAME_SOURCE_DIR}/shared/*.c)

# Android entry point (replaces old main.c)
set(ANDROID_MAIN ${GAME_SOURCE_DIR}/platform/android/android_main.c)

add_library(jumperchicken SHARED ${SHARED_SRC} ${ANDROID_MAIN})
target_compile_definitions(jumperchicken PRIVATE PLATFORM_ANDROID)
target_include_directories(jumperchicken PRIVATE
    ${GAME_SOURCE_DIR}
    ${GAME_SOURCE_DIR}/shared
    ${ANDROID_NDK}/sources/android/native_app_glue
)
target_link_libraries(jumperchicken PRIVATE raylib android log EGL GLESv2 OpenSLES m)