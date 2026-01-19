cmake_minimum_required(VERSION 3.10)
project(RType-Servers)

# C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Common server sources
set(SERVER_COMMON_SOURCES
    ../Network/TransportLayer/Packet.cpp
    ../Network/TransportLayer/ASocket.cpp
    ../Network/TransportLayer/Protocol.cpp
    ../Network/SessionManager.cpp
    IpcChannel.cpp
)

# TCP-specific sources
set(TCP_SOURCES
    server.cpp
    ../Network/TransportLayer/TCP/TCPServer.cpp
    ../Network/TransportLayer/TCP/TCPSocket.cpp
    ChildProcessManager.cpp
)

# UDP-specific sources
set(UDP_SOURCES
    game.cpp
    ../Network/TransportLayer/UDP/UDPGameServer.cpp
    ../Network/TransportLayer/UDP/UDPSocket.cpp
    ../Network/TransportLayer/UDP/GameWorld.cpp
)

# TCP Server executable
add_executable(rtype-tcp-server
    ${SERVER_COMMON_SOURCES}
    ${TCP_SOURCES}
)

# UDP Server executable
add_executable(rtype-udp-server
    ${SERVER_COMMON_SOURCES}
    ${UDP_SOURCES}
)

# Platform-specific linking for TCP server
if(WIN32)
    target_link_libraries(rtype-tcp-server PRIVATE ws2_32)
    target_link_libraries(rtype-udp-server PRIVATE ws2_32)
else()
    find_package(Threads REQUIRED)
    target_link_libraries(rtype-tcp-server PRIVATE Threads::Threads)
    target_link_libraries(rtype-udp-server PRIVATE Threads::Threads)
endif()

# Include directories
target_include_directories(rtype-tcp-server PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
target_include_directories(rtype-udp-server PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})

# Output directory
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)