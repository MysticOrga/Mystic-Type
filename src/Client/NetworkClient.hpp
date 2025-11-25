/*
** EPITECH PROJECT, 2025
** Mystic-Type
** File description:
** Network client (TCP + UDP)
*/

#pragma once

#include <cstdint>
#include <string>
#include <netinet/in.h>
#include <sys/select.h>
#include "../Network/TransportLayer/Packet.hpp"
#include "../Network/TransportLayer/UDP/UDPSocket.hpp"
#include "GameState.hpp"

class NetworkClient {
public:
    NetworkClient(const std::string &ip, uint16_t port);
    ~NetworkClient();

    bool connectToServer();
    bool performHandshake();
    bool sendPong();
    bool sendHelloUdp(uint8_t x = 0, uint8_t y = 0);
    bool sendInput(uint8_t x, uint8_t y);

    bool pollPackets();

    int getPlayerId() const { return _playerId; }
    const std::vector<PlayerState> &getLastSnapshot() const { return _lastSnapshot; }
    const std::vector<PlayerState> &getLastPlayerList() const { return _lastPlayerList; }
    const std::vector<std::string> &getEvents() const { return _events; }
    void clearEvents() { _events.clear(); }

private:
    bool readTcpPacket(Packet &p);
    bool readUdpPacket(Packet &p);
    bool sendPacketTcp(const Packet &p);
    bool sendPacketUdp(const Packet &p);
    void handleTcpPacket(const Packet &p);
    void handleUdpPacket(const Packet &p);

    int _tcpFd = -1;
    int _udpFd = -1;
    sockaddr_in _serverAddr{};
    int _playerId = -1;

    std::vector<PlayerState> _lastSnapshot;
    std::vector<PlayerState> _lastPlayerList;
    std::vector<std::string> _events;
};
