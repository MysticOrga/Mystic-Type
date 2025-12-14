cmake_minimum_required(VERSION 3.14)

project(RTypeClient)

if(UNIX AND NOT APPLE)
    set(GLFW_BUILD_WAYLAND OFF CACHE BOOL "" FORCE)
    set(GLFW_BUILD_X11 ON CACHE BOOL "" FORCE)
endif()

include(FetchContent)
FetchContent_Declare(
    raylib
    GIT_REPOSITORY https://github.com/raysan5/raylib.git
    GIT_TAG        5.5
)

set(BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(raylib)

set(CLIENT_SOURCES
    main.cpp
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
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/..
)

target_link_libraries(rtype-client PRIVATE raylib)

if(WIN32)
    target_link_libraries(rtype-client PRIVATE ws2_32)
endif()
