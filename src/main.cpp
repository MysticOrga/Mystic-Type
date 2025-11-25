/*
** EPITECH PROJECT, 2025
** Mystic-Type
** File description:
** main
*/

#include "raylib-cpp.hpp"
#include "./Graphic/Graphic.hpp"
#include "./ecs/Core.hpp"
#include "./ecs/Components.hpp"
#include "./ecs/System.hpp"
#include <iostream>
#include <memory>
#include <vector>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <errno.h>

#include "Network/TransportLayer/Packet.hpp"

int main()
{
    int fd = socket(AF_INET, SOCK_STREAM, 0);
    if (fd < 0) {
        std::cerr << "[CLIENT] Failed to create socket.\n";
        return 1;
    }

    sockaddr_in addr{};
    addr.sin_family = AF_INET;
    addr.sin_port = htons(4242);
    inet_pton(AF_INET, "127.0.0.1", &addr.sin_addr);

    if (connect(fd, (sockaddr *)&addr, sizeof(addr)) < 0) {
        std::cerr << "[CLIENT] Failed to connect.\n";
        close(fd);
        return 1;
    }
    std::cout << "[CLIENT] Connected to server.\n";

    uint8_t buffer[1024];

    ssize_t n = read(fd, buffer, sizeof(buffer));
    if (n <= 0) {
        std::cerr << "[CLIENT] No data from server\n";
        close(fd);
        return 1;
    }

    Packet serverHello = Packet::deserialize(buffer, n);
    std::string helloTxt(serverHello.payload.begin(), serverHello.payload.end());
    std::cout << "[CLIENT] Received: " << helloTxt << "\n";

    Packet clientHello(PacketType::CLIENT_HELLO, std::vector<uint8_t>{'t', 'o', 't', 'o'});
    auto bin = clientHello.serialize();
    write(fd, bin.data(), bin.size());
    std::cout << "[CLIENT] Sent CLIENT_HELLO\n";

    n = read(fd, buffer, sizeof(buffer));
    if (n <= 0) {
        std::cerr << "[CLIENT] No OK packet.\n";
        close(fd);
        return 1;
    }

    Packet ok = Packet::deserialize(buffer, n);
    if (ok.payload.size() < 2) {
        std::cerr << "[CLIENT] Invalid OK packet payload size.\n";
        close(fd);
        return 1;
    }

    int playerId = (ok.payload[0] << 8) | ok.payload[1];
    std::cout << "[CLIENT] Server assigned ID = " << playerId << "\n";

    raylib::Window window(800, 600, "Mystic-Type");
    ::SetTargetFPS(60);
    if (!::IsWindowReady()) {
        std::cerr << "Failed to initialize window." << std::endl;
        close(fd);
        return 1;
    }

    ECS ecs;
    Entity player = ecs.createEntity();
    ecs.addComponent(player, Position{100, 100});
    ecs.addComponent(player, Velocity{0, 0});

    auto playerSprite = std::make_shared<Rtype::Graphic::AnimatedSprite>(
        "./sprites/r-typesheet42.gif",
        raylib::Vector2{33, 17},
        raylib::Vector2{0, 0},
        5,
        0.15f,
        raylib::Vector2{100, 100}
    );
    ecs.addComponent(player, Sprite{playerSprite});

    InputSystem inputSystem;
    MovementSystem movementSystem;
    SpriteRenderSystem spriteRenderSystem;

    bool connected = true;

    while (connected && !window.ShouldClose()) {
        ssize_t r = recv(fd, buffer, sizeof(buffer), MSG_DONTWAIT);
        if (r > 0) {
            Packet p = Packet::deserialize(buffer, r);

            if (p.type == PacketType::PING) {
                std::cout << "[CLIENT] Received PING → sending PONG\n";
                Packet pong(PacketType::PONG, {});
                auto pongData = pong.serialize();
                write(fd, pongData.data(), pongData.size());
            } else {
                std::cout << "[CLIENT] Received packet type: " << static_cast<int>(p.type) << "\n";
            }
        } else if (r == 0) {
            std::cout << "[CLIENT] Server closed the connection.\n";
            connected = false;
        } else {
            if (errno != EAGAIN && errno != EWOULDBLOCK) {
                std::perror("[CLIENT] recv error");
                connected = false;
            }
        }
        if (!connected)
            break;
        inputSystem.update(ecs, player);
        movementSystem.update(ecs, player);
        window.BeginDrawing();
        window.ClearBackground(RAYWHITE);
        spriteRenderSystem.update(ecs, player);
        window.EndDrawing();
    }
    if (connected == false) {
        if (::IsWindowReady())
            ::CloseWindow();
    }
    close(fd);
    return 0;
}
