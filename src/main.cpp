#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <string>
#include <string.h>
#include <array>
#include <iostream>
#include <fstream>
#include "./Network/Socket/UDPSocket.hpp"

#define BUFFER_SIZE 2048
#define MAX_CLIENT 4
char buffer[BUFFER_SIZE] = {0};
std::string msg = "Hello World!\n";

struct PacketPosition {
    uint8_t header;
    int8_t x;
    int8_t y;
};

enum Move : uint8_t {
    NONE   = 0,
    UP     = 1,
    DOWN   = 2,
    LEFT   = 3,
    RIGHT  = 4
};

uint8_t encodeByte(uint8_t user, uint8_t move)
{
    return (user << 4) | (move & 0x0F);
}

struct Player
{
    int id;
    struct sockaddr_in addr;
    int x;
    int y;
};

void applyMove(Player &p, uint8_t move)
{
    switch (move) {
        case UP:    p.y -= 1; break;
        case DOWN:  p.y += 1; break;
        case LEFT:  p.x -= 1; break;
        case RIGHT: p.x += 1; break;
        default: break;
    }
}

uint8_t encode(uint8_t user, uint8_t move)
{
    return (user << 4) | (move & 0x0F);
}

int main()
{
    std::cout << "Server start!" << std::endl;

    int sockfd = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if (sockfd < 0) {
        perror("socket");
        return 84;
    }

    sockaddr_in servAddr{};
    servAddr.sin_family = AF_INET;
    servAddr.sin_port = htons(4242);
    servAddr.sin_addr.s_addr = INADDR_ANY;

    if (bind(sockfd, (sockaddr*)&servAddr, sizeof(servAddr)) < 0) {
        perror("bind");
        return 84;
    }

    std::array<Player, MAX_CLIENT> players{};
    char buffer[BUFFER_SIZE];

    while (1)
    {
        sockaddr_in clientAddr{};
        socklen_t clientLen = sizeof(clientAddr);
        memset(buffer, 0, BUFFER_SIZE);

        ssize_t r = recvfrom(sockfd, buffer, BUFFER_SIZE, 0, (sockaddr*)&clientAddr, &clientLen);

        if (r <= 0) {
            perror("recvfrom");
            continue;
        }

        uint8_t byte = buffer[0];
        uint8_t user = byte >> 4;
        uint8_t move = byte & 0x0F;

        std::cout << "\n--- PACKET ---\n";
        std::cout << "From: " << inet_ntoa(clientAddr.sin_addr) << ":" << ntohs(clientAddr.sin_port) << "\n";
        std::cout << "User = " << (int)user << " | Move = " << (int)move << "\n";

        if (user == 0) {
            std::cout << "Invalid user ID (0 ignored)" << std::endl;
            continue;
        }

        Player *p = nullptr;

        for (auto &pl : players) {
            if (pl.id == user) { p = &pl; break; }
        }

        if (!p) {
            for (auto &pl : players) {
                if (pl.id == 0) {
                    pl.id = user;
                    pl.addr = clientAddr;
                    pl.x = 0;
                    pl.y = 0;
                    p = &pl;
                    break;
                }
            }
        }

        if (!p) {
            std::cout << ">> Erreur: server is full (max 4 players).\n";
            continue;
        }
        applyMove(*p, move);

        std::cout << "New pos for player " << p->id << ": (" << p->x << ", " << p->y << ")\n";
        uint8_t out = encodeByte(p->id, move);
        PacketPosition posPacket;
        posPacket.header = out;
        posPacket.x = static_cast<int8_t>(p->x);
        posPacket.y = static_cast<int8_t>(p->y);

        for (auto &target : players) {
            if (target.id == 0)
                continue;
            sendto(sockfd, &posPacket, sizeof(posPacket), 0, (sockaddr*)&target.addr, sizeof(target.addr));
            sendto(sockfd, &out, 1, 0, (sockaddr*)&target.addr, sizeof(target.addr));
        }
    }
    close(sockfd);
    return 0;
}
