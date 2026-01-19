cmake_minimum_required(VERSION 3.10)
project(rtype-client LANGUAGES CXX)

# Standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Linux window system (raylib / GLFW)
if(UNIX AND NOT APPLE)
    set(GLFW_BUILD_WAYLAND OFF CACHE BOOL "" FORCE)
    set(GLFW_BUILD_X11 ON CACHE BOOL "" FORCE)
endif()

# Dependencies
include(FetchContent)
set(BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)

FetchContent_Declare(
    raylib
    GIT_REPOSITORY https://github.com/raysan5/raylib.git
    GIT_TAG 5.5
)
FetchContent_MakeAvailable(raylib)

# Sources
set(CLIENT_SOURCES
    client.cpp
    GraphicClient/GraphicClient.cpp
    Raylib/Raylib.cpp
    Graphic/Graphic.cpp
    ../Network/Client/NetworkClient.cpp
    ../Network/TransportLayer/Packet.cpp
    ../Network/TransportLayer/ASocket.cpp
    ../Network/TransportLayer/UDP/UDPSocket.cpp
    ../Network/TransportLayer/Protocol.cpp
)

add_executable(rtype-client ${CLIENT_SOURCES})

target_include_directories(rtype-client PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/../src
)

target_link_libraries(rtype-client PRIVATE raylib)

# Platform-specific
if(WIN32)
    target_link_libraries(rtype-client PRIVATE ws2_32)
endif()

target_include_directories(rtype-client PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})

set_target_properties(rtype-client PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin
)