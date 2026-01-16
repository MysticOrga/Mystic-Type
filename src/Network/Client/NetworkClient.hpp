/*
** EPITECH PROJECT, 2025
** Mystic-Type
** File description:
** Network client (TCP + UDP)
*/

#pragma once

#include <cstdint>
#include <string>
// #include <netinet/in.h>
#ifndef _WIN32
    #include <sys/select.h>
#endif
#include "../TransportLayer/Packet.hpp"
#include "../TransportLayer/UDP/UDPSocket.hpp"
#include "GameState.hpp"

/**
 * @brief TCP+UDP client responsible for handshake, lobby, and gameplay packets.
 */
class NetworkClient {
public:
    /**
     * @brief Build a client targeting a server address.
     * @param ip IPv4 address as string.
     * @param port TCP port.
     */
    NetworkClient(const std::string &ip, uint16_t port);
    /**
     * @brief Disconnect on destruction.
     */
    ~NetworkClient();

    /**
     * @brief Create a TCP connection and ensure UDP socket exists.
     */
    bool connectToServer();
    /**
     * @brief Perform the application handshake over TCP.
     */
    bool performHandshake();
    /**
     * @brief Send TCP PONG in response to server PING.
     */
    bool sendPong();
    /**
     * @brief Send UDP hello to register address/port on server.
     */
    bool sendHelloUdp(uint8_t x = 0, uint8_t y = 0);
    /**
     * @brief Input directions for UDP INPUT packets.
     */
    enum class MoveCmd : uint8_t { Up = 0, Down = 1, Left = 2, Right = 3 };
    /**
     * @brief Send player input over UDP.
     */
    bool sendInput(uint8_t posX, uint8_t posY, int8_t velX, int8_t velY, MoveCmd dir);
    /**
     * @brief Send a shoot command over UDP.
     */
    bool sendShoot(uint8_t posX, uint8_t posY, int8_t velX, int8_t velY);
    /**
     * @brief Send a chat message over TCP.
     */
    bool sendChat(const std::string &text);
    /**
     * @brief Ask the server to create a private lobby.
     */
    bool sendCreateLobby();
    /**
     * @brief Ask the server to join an existing lobby.
     */
    bool sendJoinLobby(const std::string &code);
    /**
     * @brief Set the preferred pseudo for the handshake payload.
     */
    void setPseudo(const std::string &pseudo) { _pseudo = pseudo; }
    /**
     * @brief Send a UDP ping with client timestamp.
     */
    bool sendUdpPing();

    /**
     * @brief Poll TCP/UDP sockets and handle any received packets.
     */
    bool pollPackets();
    /**
     * @brief Close TCP and UDP sockets and reset state.
     */
    void disconnect();
    /**
     * @brief Close UDP socket only.
     */
    void disconnectUdp();
    /**
     * @brief Ensure UDP socket exists.
     */
    bool ensureUdp();
    /**
     * @brief Reset state when entering a lobby.
     */
    void resetForLobby();
    /**
     * @brief Reset state after a full reconnect.
     */
    void resetForReconnect();
    /**
     * @brief Return true when TCP is connected.
     */
    bool isConnected() const { return _tcpConnected; }
    /**
     * @brief Update remote server address for TCP and UDP.
     */
    void updateServerAddress(const std::string &ip, uint16_t port);

    /**
     * @brief Get server-assigned player id.
     */
    int getPlayerId() const { return _playerId; }
    /**
     * @brief Get current lobby code.
     */
    const std::string &getLobbyCode() const { return _lobbyCode; }
    /**
     * @brief Get current pseudo.
     */
    const std::string &getPseudo() const { return _pseudo; }
    /**
     * @brief Get last received player snapshots.
     */
    const std::vector<PlayerState> &getLastSnapshot() const { return _lastSnapshot; }
    /**
     * @brief Get last received bullet snapshots.
     */
    const std::vector<BulletState> &getLastSnapshotBullets() const { return _lastSnapshotBullets; }
    /**
     * @brief Get last received monster snapshots.
     */
    const std::vector<MonsterState> &getLastSnapshotMonsters() const { return _lastSnapshotMonsters; }
    /**
     * @brief Get last received player list from TCP.
     */
    const std::vector<PlayerState> &getLastPlayerList() const { return _lastPlayerList; }
    /**
     * @brief Get pending network events.
     */
    const std::vector<std::string> &getEvents() const { return _events; }
    /**
     * @brief Clear queued events.
     */
    void clearEvents() { _events.clear(); }
    /**
     * @brief Last measured UDP RTT in ms.
     */
    int getUdpPingMs() const { return _udpPingMs; }
    /**
     * @brief Estimated snapshot loss percentage.
     */
    float getUdpLossPct() const;
    /**
     * @brief Get current UDP receive rate in Kbps.
     */
    float getUdpRxKbps();
    /**
     * @brief Get current UDP transmit rate in Kbps.
     */
    float getUdpTxKbps();

private:
    /**
     * @brief Result of a framed TCP receive attempt.
     */
    enum class RecvResult { Disconnected, Incomplete, Ok };
    bool readTcpPacket(Packet &p);
    bool readUdpPacket(Packet &p);
    bool sendPacketTcp(const Packet &p);
    bool sendPacketUdp(const Packet &p);
    void handleTcpPacket(const Packet &p);
    void handleUdpPacket(const Packet &p);
    bool writeAll(socket_t fd, const uint8_t *data, std::size_t size);
    RecvResult receiveTcpFramed(Packet &p);

    socket_t _tcpFd = -1;
    socket_t _udpFd = -1;
    bool _tcpConnected = false;
    bool _udpConnected = false;
    sockaddr_in _tcpAddr{};
    sockaddr_in _udpAddr{};
    int _playerId = -1;
    std::string _lobbyCode;
    std::string _pseudo;

    std::vector<PlayerState> _lastSnapshot;
    std::vector<BulletState> _lastSnapshotBullets;
    std::vector<MonsterState> _lastSnapshotMonsters;
    std::vector<PlayerState> _lastPlayerList;
    std::vector<std::string> _events;
    std::vector<uint8_t> _tcpRecvBuffer;
    uint16_t _lastSnapshotSeq = 0;
    bool _hasSnapshotSeq = false;
    uint64_t _snapshotReceived = 0;
    uint64_t _snapshotLost = 0;
    int _udpPingMs = -1;
    uint32_t _udpLastPingSentMs = 0;
    uint64_t _udpBytesInWindow = 0;
    uint64_t _udpBytesOutWindow = 0;
    long long _udpRateWindowStartMs = 0;
    float _udpRxKbps = 0.0f;
    float _udpTxKbps = 0.0f;

    long long nowMs() const;
    void updateUdpRates(long long nowMs);
};
