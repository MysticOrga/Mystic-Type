cmake_minimum_required(VERSION 3.14)

project(RTypeServer)

set(COMMON_SOURCES
    ../Network/TransportLayer/Packet.cpp
    ../Network/TransportLayer/ASocket.cpp
    ../Network/TransportLayer/Protocol.cpp
    ../Network/SessionManager.cpp
    IpcChannel.cpp
)

set(TCP_SOURCES
    main.cpp
    ../Network/TransportLayer/TCP/TCPServer.cpp
    ../Network/TransportLayer/TCP/TCPSocket.cpp
)

set(UDP_SOURCES
    udp_main.cpp
    ../Network/TransportLayer/UDP/UDPGameServer.cpp
    ../Network/TransportLayer/UDP/UDPSocket.cpp
    ../Network/TransportLayer/UDP/GameWorld.cpp
)

add_executable(rtype-tcp-server
    ${TCP_SOURCES}
    ${COMMON_SOURCES}
)

add_executable(rtype-udp-server
    ${UDP_SOURCES}
    ${COMMON_SOURCES}
)

target_include_directories(rtype-tcp-server PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/..
)

target_include_directories(rtype-udp-server PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/..
)

find_package(Threads REQUIRED)
target_link_libraries(rtype-tcp-server PRIVATE Threads::Threads)
target_link_libraries(rtype-udp-server PRIVATE Threads::Threads)

if(WIN32)
    target_link_libraries(rtype-tcp-server PRIVATE ws2_32)
    target_link_libraries(rtype-udp-server PRIVATE ws2_32)
endif()
