cmake_minimum_required(VERSION 3.21)

project(rtype LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Fix de compatibilité : raylib utilise cmake_minimum_required(VERSION 3.0)
set(CMAKE_POLICY_VERSION_MINIMUM 3.5)

# Force Apple Silicon
if(APPLE AND CMAKE_SYSTEM_PROCESSOR MATCHES "arm64")
    set(CMAKE_OSX_ARCHITECTURES "arm64" CACHE STRING "" FORCE)
endif()

# CPM configuration
set(CPM_DOWNLOAD_VERSION 0.38.6)
set(CPM_DOWNLOAD_LOCATION "${CMAKE_BINARY_DIR}/cmake/CPM.cmake")

if(NOT EXISTS ${CPM_DOWNLOAD_LOCATION})
    file(DOWNLOAD
        "https://github.com/cpm-cmake/CPM.cmake/releases/download/v${CPM_DOWNLOAD_VERSION}/CPM.cmake"
        ${CPM_DOWNLOAD_LOCATION}
        EXPECTED_HASH SHA256=11c3fa5f1ba14f15d31c2fb63dbc8628ee133d81c8d764caad9a8db9e0bacb07
    )
endif()

include(${CPM_DOWNLOAD_LOCATION})

set(BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
set(BUILD_GAMES OFF CACHE BOOL "" FORCE)

# ---- raylib ----
CPMAddPackage(
    NAME raylib
    GITHUB_REPOSITORY raysan5/raylib
    GIT_TAG 5.0
)

if(raylib_ADDED)
    message(STATUS "raylib fetched via CPM")
endif()

# ---- Source files ----
file(GLOB_RECURSE RTYPE_SOURCES CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp")

add_executable(rtype ${RTYPE_SOURCES})
target_include_directories(rtype PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/src")

target_link_libraries(rtype PRIVATE raylib)

# ---- macOS Specific Frameworks ----
if(APPLE)
    target_link_libraries(rtype PRIVATE
        "-framework Cocoa"
        "-framework IOKit"
        "-framework CoreVideo"
        "-framework CoreAudio"
        "-framework AudioToolbox"
        "-framework QuartzCore"
        "-framework Metal"
    )
endif()

# ---- Linux ----
if(UNIX AND NOT APPLE)
    target_link_libraries(rtype PRIVATE m pthread dl rt)
endif()

# ---- Windows ----
if(WIN32)
    target_link_libraries(rtype PRIVATE opengl32 gdi32 winmm)
endif()
