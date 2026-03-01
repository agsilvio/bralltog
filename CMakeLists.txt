cmake_minimum_required(VERSION 3.12)
project(bralltog)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# ------------------------------------------------------------------
# SDL configuration flags
# ------------------------------------------------------------------

set(SDLTTF_FREETYPE_VENDORED ON)
set(SDLTTF_VENDORED ON)
set(SDLTTF_HARFBUZZ OFF)
set(SDLTTF_PLUTOSVG OFF)

include(FetchContent)

FetchContent_Declare(
        remoteSdl3
        GIT_REPOSITORY https://github.com/libsdl-org/SDL.git
        GIT_TAG prerelease-3.3.4
        GIT_SHALLOW TRUE
        EXCLUDE_FROM_ALL
)

FetchContent_Declare(
        remoteSdl3Ttf
        GIT_REPOSITORY https://github.com/libsdl-org/SDL_ttf.git
        GIT_TAG release-3.2.2
        GIT_SHALLOW TRUE
        EXCLUDE_FROM_ALL
)

FetchContent_Declare(
        remoteSdl3Image
        GIT_REPOSITORY https://github.com/libsdl-org/SDL_Image.git
        GIT_TAG release-3.2.4
        GIT_SHALLOW TRUE
        EXCLUDE_FROM_ALL
)

FetchContent_Declare(
        remoteSdl3Mixer
        GIT_REPOSITORY https://github.com/libsdl-org/SDL_Mixer.git
        GIT_TAG prerelease-3.1.2
        GIT_SHALLOW TRUE
        EXCLUDE_FROM_ALL
)

FetchContent_MakeAvailable(
    remoteSdl3
    remoteSdl3Ttf
    remoteSdl3Image
    remoteSdl3Mixer
)

# ------------------------------------------------------------------
# Source collection (all .c files in src/)
# ------------------------------------------------------------------

file(GLOB_RECURSE GAME_SOURCES CONFIGURE_DEPENDS
     ${CMAKE_SOURCE_DIR}/src/*.c)

add_executable(bralltog ${GAME_SOURCES})

# ------------------------------------------------------------------
# Header support
# ------------------------------------------------------------------

target_include_directories(bralltog PRIVATE
    ${CMAKE_SOURCE_DIR}/src
)

# ------------------------------------------------------------------
# Link SDL libraries
# ------------------------------------------------------------------

target_link_libraries(bralltog PRIVATE
    SDL3::SDL3
    SDL3_mixer::SDL3_mixer
    SDL3_ttf::SDL3_ttf
    SDL3_image::SDL3_image
    m
)

# ------------------------------------------------------------------
# Copy assets folder to build folder
# ------------------------------------------------------------------

add_custom_command(TARGET bralltog POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
            ${CMAKE_SOURCE_DIR}/assets
            $<TARGET_FILE_DIR:bralltog>/assets
)

# ------------------------------------------------------------------
# Emscripten specific settings
# ------------------------------------------------------------------

if (EMSCRIPTEN)

    target_link_options(bralltog PRIVATE
        -sINITIAL_MEMORY=16777216
        --shell-file ${CMAKE_SOURCE_DIR}/shell.html
        --preload-file ${CMAKE_SOURCE_DIR}/assets@/assets
    )

    set_target_properties(bralltog PROPERTIES
        OUTPUT_NAME "index"
        SUFFIX ".html"
    )

endif()
