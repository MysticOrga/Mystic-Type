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
#include "UDPSocket.hpp"
#include "../Packet.hpp"
#include "GameWorld.hpp"
#include "../../SessionManager.hpp"

/**
 * @brief Simple UDP authoritative game server that manages players and snapshots.
 *
 * The server listens on a UDP port, accepts player "hello" packets, tracks their
 * positions/velocities, updates a small simulation loop, and periodically broadcasts
 * snapshots to all connected players.
 */
class UDPGameServer {
    public:
        explicit UDPGameServer(uint16_t port, SessionManager &sessions, long long snapshotIntervalMs = 500);

        /**
         * @brief Start the main server loop (blocking).
         */
        void run();

    private:
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

        Network::TransportLayer::UDPSocket _socket;
        GameWorld _world;
        SessionManager &_sessions;
        long long _lastSnapshotMs = 0;
        long long _lastTickMs = 0;
        const uint16_t _port;
        const long long _snapshotIntervalMs;
        const long long _tickIntervalMs = 32; // 16 = ~60 hz (les grand jeux c'est environ 100 ticks/d)
};
