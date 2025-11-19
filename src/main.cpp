#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <string>
#include <string.h>
#include <array>
#include <iostream>
#include <fstream>
#include "Network/TransportLayer/UDP/UDPSocket.hpp"
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
    Network::TransportLayer::UDPSocket server;

    memset(&player, 0, sizeof(Player) * MAX_CLIENT);

    if (!server.bindSock(AF_INET, 4242, INADDR_ANY))
    {
        perror("[bind]");
        return EXIT_FAILURE;
    };

    while (1)
    {
        sockaddr_in clientAddr;

        memset(&clientAddr, 0, sizeof(clientAddr));
        int sender = 0;
        if (server.readByte(buffer, BUFFER_SIZE) != -1)
        {
            clientAddr = server.getSenderAddr();
            std::cout << "message receive from: " << inet_ntoa(clientAddr.sin_addr) << ":" << ntohs(clientAddr.sin_port) << std::endl;
            std::cout << "Add a possibly new user" << std::endl;

            for (std::size_t i = 0; i < MAX_CLIENT; i++)
            {
                std::cout << "Client[" << player[i].id << "]: " << inet_ntoa(player[i].addr.sin_addr) << ":" << ntohs(player[i].addr.sin_port) << std::endl;
                if (!strcmp(inet_ntoa(player[i].addr.sin_addr), inet_ntoa(clientAddr.sin_addr)) && ntohs(player[i].addr.sin_port) == ntohs(clientAddr.sin_port))
                {
                    std::cout << "Client already existant\n"
                              << std::endl;
                    sender = player[i].id;
                    break;
                }
                if (player[i].id == 0)
                {
                    std::cout << "Add a new client\n"
                              << std::endl;
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
                    if (server.writeByte(buffer, strlen(buffer), player[i].addr) == -1)
                        std::cout << "Error sending data" << std::endl;
                }
            }
            memset(buffer, 0, BUFFER_SIZE);
        }
    }
    return EXIT_SUCCESS;
}