/*
** EPITECH PROJECT, 2025
** Mystic-Type
** File description:
** Client entrypoint
*/

#include <iostream>
#include <chrono>
#include "Client/NetworkClient.hpp"
#include "Client/GameState.hpp"

int main()
{
    NetworkClient net("127.0.0.1", 4242);
    GameState state;

    if (!net.connectToServer()) {
        std::cerr << "[CLIENT] Failed to connect\n";
        return 1;
    }
    if (!net.performHandshake()) {
        std::cerr << "[CLIENT] Handshake failed\n";
        return 1;
    }
    std::cout << "[CLIENT] Assigned ID " << net.getPlayerId() << "\n";

    net.sendHelloUdp(0, 0);
    uint8_t posX = 0;
    uint8_t posY = 0;
    auto lastInputSend = std::chrono::steady_clock::now();

    while (true) {
        net.pollPackets();

        for (const auto &ev : net.getEvents()) {
            if (ev == "PLAYER_LIST" || ev == "NEW_PLAYER") {
                for (const auto &p : net.getLastPlayerList())
                    state.upsertPlayer(p.id, p.x, p.y);
            } else if (ev == "SNAPSHOT") {
                for (const auto &p : net.getLastSnapshot())
                    state.upsertPlayer(p.id, p.x, p.y);
                std::cout << "[CLIENT] Snapshot: ";
                for (const auto &p : state.listPlayers())
                    std::cout << p.id << "(" << static_cast<int>(p.x) << "," << static_cast<int>(p.y) << ") ";
                std::cout << "\n";
            }
        }
        net.clearEvents();

        auto now = std::chrono::steady_clock::now();
        if (std::chrono::duration_cast<std::chrono::milliseconds>(now - lastInputSend).count() >= 1000) {
            posX = static_cast<uint8_t>(posX + 1);
            net.sendInput(posX, posY);
            lastInputSend = now;
        }
    }
}
