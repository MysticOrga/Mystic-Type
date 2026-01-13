cmake_minimum_required(VERSION 3.10)
project(Mystic-Type)

# C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Linux-only window config
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
set(COMMON_SOURCES
    src/Network/Client/NetworkClient.cpp
    src/Network/TransportLayer/Packet.cpp
    src/Network/TransportLayer/ASocket.cpp
    src/Network/TransportLayer/UDP/UDPSocket.cpp
    src/Network/TransportLayer/Protocol.cpp
)

set(SERVER_SOURCES
    server.cpp
    src/Network/SessionManager.cpp
    src/Network/TransportLayer/UDP/GameWorld.cpp
    src/Network/TransportLayer/UDP/UDPGameServer.cpp
    src/Network/TransportLayer/TCP/TCPServer.cpp
    src/Network/TransportLayer/TCP/TCPSocket.cpp
    ${COMMON_SOURCES}
)

set(CLIENT_SOURCES
    client.cpp
    src/graphical-client/GraphicClient/GraphicClient.cpp
    src/graphical-client/Raylib/Raylib.cpp
    src/graphical-client/Graphic/Graphic.cpp
    ${COMMON_SOURCES}
)

# Targets
add_executable(rtype-server ${SERVER_SOURCES})
add_executable(rtype-client ${CLIENT_SOURCES})

target_link_libraries(rtype-client PRIVATE raylib)

# Platform-specific linking
if(WIN32)
    target_link_libraries(rtype-server PRIVATE ws2_32)
    target_link_libraries(rtype-client PRIVATE ws2_32)
else()
    find_package(Threads REQUIRED)
    target_link_libraries(rtype-server PRIVATE Threads::Threads)
endif()

# Includes
target_include_directories(rtype-server PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src)
target_include_directories(rtype-client PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src)

# Output
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
