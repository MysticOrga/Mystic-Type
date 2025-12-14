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

/**
 * @brief Supported packet types exchanged between server and clients.
 */
enum class PacketType : uint8_t {
    //TCP
    SERVER_HELLO = 1,   ///< Server greeting with assigned id.
    CLIENT_HELLO = 2,   ///< Client greeting when connecting.
    OK = 3,             ///< Generic acknowledgement.
    REFUSED = 4,        ///< Connection refused or invalid request.
    PING = 5,           ///< Heartbeat ping.
    PONG = 6,           ///< Heartbeat response.
    MESSAGE = 7,        ///< Text or generic payload message.
    PLAYER_LIST = 8,    ///< Broadcast of currently connected players.
    NEW_PLAYER  = 9,    ///< Notification of a newly connected player.

    //UDP
    HELLO_UDP   = 10,   ///< UDP hello used for NAT hole punching.
    INPUT       = 11,   ///< Player input payload.
    SNAPSHOT    = 12,   ///< World state snapshot.
    SHOOT       = 13    ///< Player shoot command.
};

/**
 * @brief Light container for game protocol packets with (de)serialization helpers.
 */
class Packet {
    public:
        uint16_t header = PACKET_MAGIC; ///< Magic number to validate packets.
        PacketType type = PacketType::MESSAGE; ///< Packet category.
        uint8_t size = 0; ///< Payload size in bytes.
        std::vector<uint8_t> payload; ///< Raw payload bytes.

        Packet() = default;

        /**
         * @brief Construct a packet with a given type and payload.
         *
         * @param t Packet type.
         * @param data Payload bytes.
         */
        Packet(PacketType t, const std::vector<uint8_t>& data)
        : type(t), size(static_cast<uint8_t>(data.size())), payload(data) {}

        /**
         * @brief Serialize the packet to a byte buffer ready to send.
         *
         * @return Byte vector containing header, type, size, and payload.
         */
        std::vector<uint8_t> serialize() const;

        /**
         * @brief Deserialize a packet from a raw byte buffer.
         *
         * @param data Pointer to the raw buffer.
         * @param len Length of the buffer.
         * @return Parsed Packet instance.
         * @throws std::runtime_error if the buffer is invalid.
         */
        static Packet deserialize(const uint8_t* data, size_t len);
};
