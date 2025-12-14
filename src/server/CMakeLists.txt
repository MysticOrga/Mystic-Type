cmake_minimum_required(VERSION 3.14)

project(RTypeServer)

set(SERVER_SOURCES
    main.cpp

    ../Network/TransportLayer/Packet.cpp
    ../Network/TransportLayer/ASocket.cpp
    ../Network/TransportLayer/TCP/TCPServer.cpp
    ../Network/TransportLayer/TCP/TCPSocket.cpp
    ../Network/TransportLayer/UDP/UDPGameServer.cpp
    ../Network/TransportLayer/UDP/UDPSocket.cpp
    ../Network/TransportLayer/UDP/GameWorld.cpp
    ../Network/TransportLayer/Protocol.cpp
    ../Network/SessionManager.cpp
)

add_executable(rtype-server ${SERVER_SOURCES})

target_include_directories(rtype-server PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/..
)

find_package(Threads REQUIRED)
target_link_libraries(rtype-server PRIVATE Threads::Threads)

if(WIN32)
    target_link_libraries(rtype-server PRIVATE ws2_32)
endif()
