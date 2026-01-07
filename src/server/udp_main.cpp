/*
** EPITECH PROJECT, 2025
** Mystic-Type
** File description:
** UDP game server entry point (process enfant)
*/

#include "Network/TransportLayer/UDP/UDPGameServer.hpp"
#include "Network/SessionManager.hpp"

#include <iostream>

int main()
{
    try {
        SessionManager sessions;
        UDPGameServer udpServer(4243, sessions, 50); // même numéro que le TCP (protocoles différents)
        udpServer.run();
    }
    catch (const std::exception &e) {
        std::cerr << "[UDP SERVER ERROR] " << e.what() << std::endl;
        return 1;
    }
    return 0;
}
