cmake_minimum_required(VERSION 3.21)

project(rtype LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

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

CPMAddPackage(
    NAME raylib
    GITHUB_REPOSITORY raysan5/raylib
    GIT_TAG 5.0
)

if(raylib_ADDED)
    message(STATUS "raylib fetched via CPM")
endif()

CPMAddPackage(
    NAME raylib_cpp
    GITHUB_REPOSITORY RobLoach/raylib-cpp
    GIT_TAG v5.0.1
)

if(raylib_cpp_ADDED)
    message(STATUS "raylib-cpp fetched via CPM")
endif()

file(GLOB_RECURSE RTYPE_SOURCES CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp")

add_executable(rtype ${RTYPE_SOURCES})
target_include_directories(rtype PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/src")

target_link_libraries(rtype PRIVATE raylib raylib_cpp)

if(WIN32)
    target_link_libraries(rtype PRIVATE opengl32 gdi32 winmm)
elseif(UNIX AND NOT APPLE)
    target_link_libraries(rtype PRIVATE m pthread dl rt)
endif()
