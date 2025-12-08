/*
** EPITECH PROJECT, 2025
** Mystic-Type
** File description:
** Client entrypoint
*/

#include <iostream>
#include <chrono>
#include <algorithm>
#include <sys/select.h>
#include <unistd.h>
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

    uint8_t posX = 100;
    uint8_t posY = 100;
    NetworkClient::MoveCmd lastDir = NetworkClient::MoveCmd::Up;
    net.sendHelloUdp(posX, posY);
    while (true) {
        net.pollPackets();

        for (const auto &ev : net.getEvents()) {
            if (ev == "PLAYER_LIST" || ev == "NEW_PLAYER") {
                for (const auto &p : net.getLastPlayerList())
                    state.upsertPlayer(p.id, p.x, p.y);
            } else if (ev == "SNAPSHOT") {
                for (const auto &p : net.getLastSnapshot())
                    state.upsertPlayer(p.id, p.x, p.y);
                for (const auto &b : net.getLastSnapshotBullets())
                    state.upsertBullet(b.id, b.x, b.y, b.vx, b.vy);
                std::cout << "[CLIENT] Snapshot: players ";
                for (const auto &p : state.listPlayers())
                    std::cout << p.id << "(" << static_cast<int>(p.x) << "," << static_cast<int>(p.y) << ") ";
                std::cout << "| bullets ";
                for (const auto &b : state.listBullets())
                    std::cout << b.id << "(" << static_cast<int>(b.x) << "," << static_cast<int>(b.y) << ") ";
                std::cout << "\n";
            }
        }
        net.clearEvents();

        fd_set rfds;
        FD_ZERO(&rfds);
        FD_SET(STDIN_FILENO, &rfds);
        struct timeval tv{0, 0};
        int r = select(STDIN_FILENO + 1, &rfds, nullptr, nullptr, &tv);
        if (r > 0 && FD_ISSET(STDIN_FILENO, &rfds)) {
            char c;
            if (std::cin.get(c)) {
                bool moved = false;
                NetworkClient::MoveCmd dir = NetworkClient::MoveCmd::Up;
                int8_t velX = 0;
                int8_t velY = 0;

                if (c == 'd')
                    { dir = NetworkClient::MoveCmd::Right; velX = 1; moved = true; }
                if (c == 'q')
                    { dir = NetworkClient::MoveCmd::Left; velX = -1; moved = true; }
                if (c == 'z')
                    { dir = NetworkClient::MoveCmd::Up; velY = -1; moved = true; }
                if (c == 's')
                    { dir = NetworkClient::MoveCmd::Down; velY = 1; moved = true; }
                if (c == ' ')
                    { dir = lastDir; }

                if (moved) {
                    int nx = std::clamp(static_cast<int>(posX) + velX, 0, 255);
                    int ny = std::clamp(static_cast<int>(posY) + velY, 0, 255);
                    posX = static_cast<uint8_t>(nx);
                    posY = static_cast<uint8_t>(ny);
                    lastDir = dir;
                    net.sendInput(posX, posY, velX, velY, dir);
                    std::cout << "[CLIENT] Sent MOVE pos=(" << static_cast<int>(posX) << "," << static_cast<int>(posY) << ") vel=(" << static_cast<int>(velX) << "," << static_cast<int>(velY) << ")\n";
                }

                if (c == ' ') {
                    int8_t bvx = 0;
                    int8_t bvy = 0;
                    switch (lastDir) {
                        case NetworkClient::MoveCmd::Right: bvx = 2; break;
                        case NetworkClient::MoveCmd::Left:  bvx = -2; break;
                        case NetworkClient::MoveCmd::Down:  bvy = 2; break;
                        case NetworkClient::MoveCmd::Up:    bvy = -2; break;
                    }
                    net.sendShoot(posX, posY, bvx, bvy);
                    std::cout << "[CLIENT] Sent SHOOT from (" << static_cast<int>(posX) << "," << static_cast<int>(posY)
                              << ") vel=(" << static_cast<int>(bvx) << "," << static_cast<int>(bvy) << ")\n";
                }
            } else {
                break;
            }
        }
    }
}
