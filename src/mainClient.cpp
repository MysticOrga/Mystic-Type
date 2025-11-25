#include <iostream>
#include <vector>
#include <unistd.h>
#include <arpa/inet.h>
#include "Network/TransportLayer/Packet.hpp"

struct PlayerInfo {
    int id = 0;
    uint8_t x = 0;
    uint8_t y = 0;
};

static void logPlayerList(const std::vector<PlayerInfo> &players)
{
    std::cout << "[CLIENT] Player list (" << players.size() << "):\n";
    for (const auto &p : players) {
        std::cout << "  id=" << p.id << " pos=(" << static_cast<int>(p.x) << "," << static_cast<int>(p.y) << ")\n";
    }
}

static std::vector<PlayerInfo> parsePlayerList(const Packet &p)
{
    std::vector<PlayerInfo> players;
    if (p.payload.empty())
        return players;

    uint8_t count = p.payload[0];
    size_t expectedBytes = 1 + static_cast<size_t>(count) * 4;
    if (p.payload.size() < expectedBytes)
        throw std::runtime_error("PLAYER_LIST payload too small");

    players.reserve(count);
    for (size_t i = 0; i < count; ++i) {
        size_t offset = 1 + i * 4;
        int id = (p.payload[offset] << 8) | p.payload[offset + 1];
        uint8_t x = p.payload[offset + 2];
        uint8_t y = p.payload[offset + 3];
        players.push_back(PlayerInfo{id, x, y});
    }
    return players;
}

static PlayerInfo parseNewPlayer(const Packet &p)
{
    if (p.payload.size() < 4)
        throw std::runtime_error("NEW_PLAYER payload too small");
    int id = (p.payload[0] << 8) | p.payload[1];
    uint8_t x = p.payload[2];
    uint8_t y = p.payload[3];
    return PlayerInfo{id, x, y};
}

int main()
{
    int fd = socket(AF_INET, SOCK_STREAM, 0);

    sockaddr_in addr{};
    addr.sin_family = AF_INET;
    addr.sin_port = htons(4242);
    inet_pton(AF_INET, "127.0.0.1", &addr.sin_addr);

    if (connect(fd, (sockaddr*)&addr, sizeof(addr)) < 0) {
        std::cerr << "[CLIENT] Failed to connect.\n";
        return 1;
    }
    std::cout << "[CLIENT] Connected to server.\n";

    uint8_t buffer[1024];
    ssize_t n = read(fd, buffer, sizeof(buffer));
    if (n <= 0) {
        std::cerr << "[CLIENT] No data from server\n";
        return 1;
    }

    Packet serverHello = Packet::deserialize(buffer, n);
    std::string helloTxt(serverHello.payload.begin(), serverHello.payload.end());
    std::cout << "[CLIENT] Received: " << helloTxt << "\n";

    Packet clientHello(PacketType::CLIENT_HELLO, std::vector<uint8_t>({'t','o','t','o'}));
    auto bin = clientHello.serialize();
    write(fd, bin.data(), bin.size());

    std::cout << "[CLIENT] Sent CLIENT_HELLO\n";

    n = read(fd, buffer, sizeof(buffer));
    if (n <= 0) {
        std::cerr << "[CLIENT] No OK packet.\n";
        return 1;
    }

    Packet ok = Packet::deserialize(buffer, n);
    int playerId = (ok.payload[0] << 8) | ok.payload[1];

    std::cout << "[CLIENT] Server assigned ID = " << playerId << "\n";

    // Expect initial player list
    n = read(fd, buffer, sizeof(buffer));
    if (n > 0) {
        Packet list = Packet::deserialize(buffer, n);
        if (list.type == PacketType::PLAYER_LIST) {
            try {
                auto players = parsePlayerList(list);
                logPlayerList(players);
            } catch (const std::exception &e) {
                std::cerr << "[CLIENT] Failed to parse PLAYER_LIST: " << e.what() << "\n";
            }
        }
    }

    while (true)
    {
        ssize_t r = read(fd, buffer, sizeof(buffer));
        if (r <= 0) break;

        Packet p = Packet::deserialize(buffer, r);

        if (p.type == PacketType::PING) {
            std::cout << "[CLIENT] Received PING → sending PONG\n";

            Packet pong(PacketType::PONG, {});
            auto pongData = pong.serialize();
            write(fd, pongData.data(), pongData.size());
        } else if (p.type == PacketType::NEW_PLAYER) {
            try {
                auto info = parseNewPlayer(p);
                std::cout << "[CLIENT] New player joined: id=" << info.id
                          << " pos=(" << static_cast<int>(info.x) << "," << static_cast<int>(info.y) << ")\n";
            } catch (const std::exception &e) {
                std::cerr << "[CLIENT] Failed to parse NEW_PLAYER: " << e.what() << "\n";
            }
        } else if (p.type == PacketType::PLAYER_LIST) {
            try {
                auto players = parsePlayerList(p);
                logPlayerList(players);
            } catch (const std::exception &e) {
                std::cerr << "[CLIENT] Failed to parse PLAYER_LIST: " << e.what() << "\n";
            }
        }
    }

    close(fd);
    return 0;
}
