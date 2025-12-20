/*
** EPITECH PROJECT, 2025
** Mystic-Type
** File description:
** main
*/

#include "src/graphical-client/GraphicClient/GraphicClient.hpp"
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
    GraphicClient client("10.49.84.89", 4243);    return client.run();
}
