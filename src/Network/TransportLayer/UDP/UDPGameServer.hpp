/*
** EPITECH PROJECT, 2025
** Mystic-Type
** File description:
** UDP Game Server
*/

#pragma once

#include <unordered_map>
#include <vector>
#include <chrono>
#include <netinet/in.h>
#include "UDPSocket.hpp"
#include "../Packet.hpp"

class UDPGameServer {
    public:
        explicit UDPGameServer(uint16_t port, long long snapshotIntervalMs = 500);
        void run();

    private:
        struct PlayerState {
            int id = 0;
            uint8_t x = 0;
            uint8_t y = 0;
            sockaddr_in addr{};
        };

        void handlePacket(const Packet &packet, const sockaddr_in &from);
        void handleHello(const Packet &packet, const sockaddr_in &from);
        void handleInput(const Packet &packet, const sockaddr_in &from);
        Packet buildSnapshotPacket() const;
        void broadcastSnapshot();
        bool sendPacketTo(const Packet &packet, const sockaddr_in &to);
        long long nowMs() const;

        Network::TransportLayer::UDPSocket _socket;
        std::unordered_map<int, PlayerState> _players;
        long long _lastSnapshotMs = 0;
        const uint16_t _port;
        const long long _snapshotIntervalMs;
};
