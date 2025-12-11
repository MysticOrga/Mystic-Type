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

/**
 * @brief Simple UDP authoritative game server that manages players and snapshots.
 *
 * The server listens on a UDP port, accepts player "hello" packets, tracks their
 * positions/velocities, updates a small simulation loop, and periodically broadcasts
 * snapshots to all connected players.
 */
class UDPGameServer {
    public:
        enum class MonsterKind : uint8_t { Sine = 0, Cosine = 1 };

        /**
         * @brief Construct a new UDPGameServer.
         *
         * @param port UDP port the server binds to.
         * @param snapshotIntervalMs Interval in milliseconds between state snapshots.
         */
        explicit UDPGameServer(uint16_t port, long long snapshotIntervalMs = 500);

        /**
         * @brief Start the main server loop (blocking).
         */
        void run();

    private:
        /**
         * @brief Minimal state tracked for each connected player.
         */
        struct PlayerState {
            int id = 0;
            uint8_t x = 0;
            uint8_t y = 0;
            sockaddr_in addr{};
            int8_t velX = 0;
            int8_t velY = 0;
            uint8_t dir = 0;
        };

        struct BulletState {
            int id = 0;
            uint8_t x = 0;
            uint8_t y = 0;
            int8_t velX = 0;
            int8_t velY = 0;
        };

        struct MonsterState {
            int id = 0;
            float x = 0;
            float y = 0;
            float baseY = 0;
            float amplitude = 0;
            float phase = 0;
            float freq = 0;
            float speedX = 0;
            int8_t hp = 0;
            MonsterKind kind = MonsterKind::Sine;
        };

        /**
         * @brief Route an incoming packet to the appropriate handler.
         *
         * @param packet Parsed packet contents.
         * @param from Sender endpoint.
         */
        void handlePacket(const Packet &packet, const sockaddr_in &from);

        /**
         * @brief Register a new player on receipt of a hello message.
         *
         * @param packet Incoming hello packet.
         * @param from Sender endpoint.
         */
        void handleHello(const Packet &packet, const sockaddr_in &from);

        /**
         * @brief Update player inputs (movement/direction).
         *
         * @param packet Incoming input packet.
         * @param from Sender endpoint.
         */
        void handleInput(const Packet &packet, const sockaddr_in &from);

        /**
         * @brief Handle a shoot command (spawn a bullet).
         */
        void handleShoot(const Packet &packet);

        /**
         * @brief Build a snapshot packet representing the current world state.
         */
        Packet buildSnapshotPacket() const;

        /**
         * @brief Broadcast the current snapshot to all connected players.
         */
        void broadcastSnapshot();

        /**
         * @brief Send a packet to a single endpoint.
         *
         * @param packet Packet to send.
         * @param to Destination endpoint.
         * @return true on success, false otherwise.
         */
        bool sendPacketTo(const Packet &packet, const sockaddr_in &to);

        /**
         * @brief Monotonic time helper in milliseconds.
         */
        long long nowMs() const;

        /**
         * @brief Advance the local simulation based on elapsed time.
         *
         * @param nowMs Current timestamp in milliseconds.
         */
        void updateSimulation(long long nowMs, long long deltaMs);

        void spawnMonster(long long nowMs);

        Network::TransportLayer::UDPSocket _socket;
        std::unordered_map<int, PlayerState> _players;
        std::vector<BulletState> _bullets;
        std::vector<MonsterState> _monsters;
        long long _lastSnapshotMs = 0;
        long long _lastTickMs = 0;
        const uint16_t _port;
        const long long _snapshotIntervalMs;
        const long long _tickIntervalMs = 32; // 16 = ~60 hz (les grand jeux c'est environ 100 ticks/d)
        int _nextBulletId = 1;
        int _nextMonsterId = 1;
        long long _lastMonsterSpawnMs = 0;
        long long _monsterSpawnIntervalMs = 1800;
};
