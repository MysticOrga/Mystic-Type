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
#include <string>
#include <queue>
#include <mutex>
#include <condition_variable>
#include <atomic>
#include <thread>
#include <netinet/in.h>
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
        explicit UDPGameServer(uint16_t port, SessionManager &sessions, long long snapshotIntervalMs = 500, std::string lobbyCode = "PUBLIC");
        ~UDPGameServer();

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

        /**
         * @brief Thread loop to read incoming UDP packets without blocking the simulation tick.
         */
        void networkLoop();
        std::string logPrefix() const;

        Network::TransportLayer::UDPSocket _socket;
        std::unordered_map<std::string, GameWorld> _worlds;
        std::unordered_map<int, std::string> _playerLobby;
        SessionManager &_sessions;
        long long _lastSnapshotMs = 0;
        long long _lastTickMs = 0;
        const uint16_t _port;
        const long long _snapshotIntervalMs;
        std::string _expectedLobby;
        const long long _tickIntervalMs = 32; // 16 = ~60 hz (les grand jeux c'est environ 100 ticks/d)
        struct Incoming {
            Packet pkt;
            sockaddr_in from{};
        };
        std::queue<Incoming> _incoming;
        std::mutex _queueMutex;
        std::atomic<bool> _running{false};
        std::thread _networkThread;
};
