#include <iostream>
#include <vector>
#include <unistd.h>
#include <arpa/inet.h>
#include "Network/TransportLayer/Packet.hpp"

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
        }
    }

    close(fd);
    return 0;
}
