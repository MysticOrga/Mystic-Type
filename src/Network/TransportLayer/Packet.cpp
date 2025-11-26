/*
** EPITECH PROJECT, 2025
** Mystic-Type
** File description:
** Packet
*/

#include "Packet.hpp"

std::vector<uint8_t> Packet::serialize() const
{
    if (payload.size() > UINT8_MAX)
        throw std::runtime_error("Payload too large");

    std::vector<uint8_t> buffer;
    buffer.reserve(4 + payload.size());

    buffer.push_back(static_cast<uint8_t>(header >> 8));
    buffer.push_back(static_cast<uint8_t>(header & 0xFF));

    buffer.push_back(static_cast<uint8_t>(type));
    buffer.push_back(static_cast<uint8_t>(payload.size()));

    buffer.insert(buffer.end(), payload.begin(), payload.end());

    return buffer;
}

Packet Packet::deserialize(const uint8_t* data, size_t len)
{
    if (len < 4)
        throw std::runtime_error("Packet too small");

    uint16_t magic = (data[0] << 8) | data[1];
    if (magic != 0x5254)
        throw std::runtime_error("Bad header");

    Packet p;
    p.header = magic;
    p.type = static_cast<PacketType>(data[2]);
    p.size = data[3];

    if (len < (size_t)(4 + p.size))
        throw std::runtime_error("Incomplete payload");

    p.payload.assign(data + 4, data + 4 + p.size);

    return p;
}
