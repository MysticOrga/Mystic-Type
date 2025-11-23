/*
** EPITECH PROJECT, 2025
** Mystic-Type
** File description:
** main
*/

#include "Network/TransportLayer/TCP/TCPServer.hpp"

int main()
{
    TCPServer server(4242);
    server.run();
    return 0;
}
