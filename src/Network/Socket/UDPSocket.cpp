/*
** EPITECH PROJECT, 2025
** R-type
** File description:
** UDPSocket
*/

#include "UDPSocket.hpp"

void RNetwork::UDPSocket::setAddr(int domain, uint16_t port, in_addr_t addr)
{
    _addr.sin_addr.s_addr = addr;
    _addr.sin_port = htons(port);
    _addr.sin_family = domain;
}

