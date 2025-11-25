/*
** EPITECH PROJECT, 2025
** `
** File description:
** Protocol
*/

#pragma once
#include <cstdint>
#include <stdexcept>
#include <vector>

constexpr uint16_t PACKET_MAGIC = 0x5254; // "RT"

enum class PacketType : uint8_t {
    SERVER_HELLO = 1,
    CLIENT_HELLO = 2,
    OK = 3,
    REFUSED = 4,
    PING = 5,
    PONG = 6,
    MESSAGE = 7,
    PLAYER_LIST = 8,
    NEW_PLAYER  = 9
};

class Packet {
    public:
        uint16_t header = PACKET_MAGIC;
        PacketType type = PacketType::MESSAGE;
        uint8_t size = 0;
        std::vector<uint8_t> payload;

        Packet() = default;
        Packet(PacketType t, const std::vector<uint8_t>& data)
        : type(t), size(static_cast<uint8_t>(data.size())), payload(data) {}
        std::vector<uint8_t> serialize() const;
        static Packet deserialize(const uint8_t* data, size_t len);
};
