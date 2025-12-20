cmake_minimum_required(VERSION 3.10)
project(Mystic-Type)

# Set C++ standard
set(CMAKE_CXX_STDANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)


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

# Common source shared between client and server
set(COMMON_SOURCES
    ./src/Network/Client/NetworkClient.cpp
    ./src/Network/TransportLayer/Packet.cpp
    ./src/Network/TransportLayer/ASocket.cpp
    ./src/Network/TransportLayer/UDP/UDPSocket.cpp
    ./src/Network/TransportLayer/Protocol.cpp
)

# Source code only used by the server
set(SERVER_SOURCES
    server.cpp
    ./src/Network/SessionManager.cpp
    ./src/Network/TransportLayer/UDP/GameWorld.cpp
    ./src/Network/TransportLayer/UDP/UDPGameServer.cpp
    ./src/Network/TransportLayer/TCP/TCPServer.cpp
    ./src/Network/TransportLayer/TCP/TCPSocket.cpp
    ${COMMON_SOURCES}
)

# Source code only used by the client
set(CLIENT_SOURCES
    client.cpp
    ./src/graphical-client/GraphicClient/GraphicClient.cpp
    ./src/graphical-client/Raylib/Raylib.cpp
    ./src/graphical-client/Graphic/Graphic.cpp
    ${COMMON_SOURCES}
)

# Build client and server executable
add_executable(rtype-server ${SERVER_SOURCES})
add_executable(rtype-client ${CLIENT_SOURCES})

target_link_libraries(rtype-client PRIVATE raylib)

if(WIN32)
    target_link_libraries(rtype-server ws2_32)
    target_link_libraries(rtype-client PRIVATE ws2_32)
    set_target_properties(rtype-server PROPERTIES SUFFIX ".exe")
    set_target_properties(rtype-client PROPERTIES SUFFIX ".exe")
    if(MINGW)
        # target_compile_options(rtype-client PRIVATE -fno-stack-protector)
        target_compile_definitions(rtype-client PRIVATE _FORTIFY_SOURCE=0)
        target_link_libraries(rtype-server -static-libgcc -static-libstdc++ -static)
        target_link_libraries(rtype-client PRIVATE -lssp -lssp_nonshared -lpthread -static-libgcc -static-libstdc++)

        # target_compile_options(rtype-server PRIVATE
        #     -D_FORTIFY_SOURCE=0
        #     -fno-stack-protector
        # )
    endif()
else()
    find_package(Thre   ads REQUIRED)
    target_link_libraries(rtype-server PRIVATE Threads::Threads)
endif()

# Include for server
target_include_directories(rtype-server PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src
    ${CMAKE_CURRENT_SOURCE_DIR}/src
)

# Include for cleint
target_include_directories(rtype-client PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src
    ${CMAKE_CURRENT_SOURCE_DIR}/src
)

# Output directory
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)