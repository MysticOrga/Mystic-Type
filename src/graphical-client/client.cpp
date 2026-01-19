/*
** EPITECH PROJECT, 2025
** Mystic-Type
** File description:
** main
*/

#include "GraphicClient/GraphicClient.hpp"
#include <iostream>

int main()
{
#ifdef _WIN32
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)
    {
        return 1;
    }
#endif
    GraphicClient client;
    client.run();

#ifdef _WIN32
    WSACleanup();
#endif
    return 0;
}
