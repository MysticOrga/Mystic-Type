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

struct Player
{
    int id;
    struct sockaddr_in addr;
};

int main([[maybe_unused]] int ac, [[maybe_unused]] char **av)
{
    std::array<Player, MAX_CLIENT> player;
    RNetwork::UDPSocket server;

    server.createSock(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    server.setAddr(AF_INET, 4242, INADDR_ANY);
    memset(&player, 0, sizeof(Player) * MAX_CLIENT);

    if (!server.bindSock())
    {
        perror("[bind]");
        return EXIT_FAILURE;
    };

    while (1)
    {
        sockaddr_in clientAddr;
        socklen_t clientLen = sizeof(clientAddr);

        memset(&clientAddr, 0, sizeof(clientAddr));
        int sender = 0;
        if (recvfrom(server.getSocket(), buffer, BUFFER_SIZE, 0, (struct sockaddr*)&clientAddr, &clientLen) == -1)
        {
            perror("recv");
            return EXIT_FAILURE;
        }

        std::cout << "message receive from: " << inet_ntoa(clientAddr.sin_addr) << ":" << ntohs(clientAddr.sin_port) << std::endl;
        std::cout << "Add a possibly new user" << std::endl;

        for (std::size_t i = 0; i < MAX_CLIENT; i++)
        {
            std::cout << "Client[" << player[i].id << "]: " << inet_ntoa(player[i].addr.sin_addr) << ":" << ntohs(player[i].addr.sin_port) << std::endl;
            if (!strcmp(inet_ntoa(player[i].addr.sin_addr), inet_ntoa(clientAddr.sin_addr)) && ntohs(player[i].addr.sin_port) == ntohs(clientAddr.sin_port))
            {
                std::cout << "Client already existant\n" << std::endl;
                sender = player[i].id;
                break;
            }
            if (player[i].id == 0)
            {
                std::cout << "Add a new client\n" << std::endl;
                player[i].id = i + 1;
                sender = player[i].id;
                player[i].addr = clientAddr;
                break;
            }
        }

        for (std::size_t i = 0; i < MAX_CLIENT; i++)
        {
            if (player[i].id != 0 && player[i].id != sender)
            {
                std::cout << "Message send to: " << inet_ntoa(player[i].addr.sin_addr) << ":" << ntohs(player[i].addr.sin_port) << std::endl;
                sendto(server.getSocket(), buffer, BUFFER_SIZE, 0, (struct sockaddr*)&player[i].addr, sizeof(player[i].addr));
            }
        }
    }
    return EXIT_SUCCESS;
}