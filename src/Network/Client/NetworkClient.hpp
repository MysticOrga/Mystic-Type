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
#include "../TransportLayer/Packet.hpp"
#include "../TransportLayer/UDP/UDPSocket.hpp"
#include "GameState.hpp"

class NetworkClient {
public:
    NetworkClient(const std::string &ip, uint16_t port);
    ~NetworkClient();

    bool connectToServer();
    bool performHandshake();
    bool sendPong();
    bool sendHelloUdp(uint8_t x = 0, uint8_t y = 0);
    enum class MoveCmd : uint8_t { Up = 0, Down = 1, Left = 2, Right = 3 };
    bool sendInput(uint8_t posX, uint8_t posY, int8_t velX, int8_t velY, MoveCmd dir);
    bool sendShoot(uint8_t posX, uint8_t posY, int8_t velX, int8_t velY);

    bool pollPackets();
    void disconnect();
    bool isConnected() const { return _tcpConnected; }

    int getPlayerId() const { return _playerId; }
    const std::vector<PlayerState> &getLastSnapshot() const { return _lastSnapshot; }
    const std::vector<BulletState> &getLastSnapshotBullets() const { return _lastSnapshotBullets; }
    const std::vector<MonsterState> &getLastSnapshotMonsters() const { return _lastSnapshotMonsters; }
    const std::vector<PlayerState> &getLastPlayerList() const { return _lastPlayerList; }
    const std::vector<std::string> &getEvents() const { return _events; }
    void clearEvents() { _events.clear(); }

private:
    enum class RecvResult { Disconnected, Incomplete, Ok };
    bool readTcpPacket(Packet &p);
    bool readUdpPacket(Packet &p);
    bool sendPacketTcp(const Packet &p);
    bool sendPacketUdp(const Packet &p);
    void handleTcpPacket(const Packet &p);
    void handleUdpPacket(const Packet &p);
    bool writeAll(int fd, const uint8_t *data, std::size_t size);
    RecvResult receiveTcpFramed(Packet &p);

    int _tcpFd = -1;
    int _udpFd = -1;
    bool _tcpConnected = false;
    bool _udpConnected = false;
    sockaddr_in _serverAddr{};
    int _playerId = -1;

    std::vector<PlayerState> _lastSnapshot;
    std::vector<BulletState> _lastSnapshotBullets;
    std::vector<MonsterState> _lastSnapshotMonsters;
    std::vector<PlayerState> _lastPlayerList;
    std::vector<std::string> _events;
    std::vector<uint8_t> _tcpRecvBuffer;
};
