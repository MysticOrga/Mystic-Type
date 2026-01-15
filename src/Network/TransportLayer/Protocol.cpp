/*
** EPITECH PROJECT, 2025
** Mystic-Type
** File description:
** Protocol helpers shared by client and server
*/

#include "Protocol.hpp"
#include <stdexcept>

std::vector<uint8_t> Protocol::frameTcp(const Packet &packet)
{
    std::vector<uint8_t> payload = packet.serialize();
    if (payload.size() > UINT16_MAX) {
        throw std::runtime_error("Payload too large for framing");
    }
    uint16_t len = static_cast<uint16_t>(payload.size());
    std::vector<uint8_t> framed;
    framed.reserve(payload.size() + 2);
    framed.push_back(static_cast<uint8_t>(len >> 8));
    framed.push_back(static_cast<uint8_t>(len & 0xFF));
    framed.insert(framed.end(), payload.begin(), payload.end());
    return framed;
}

Protocol::StreamStatus Protocol::extractFromBuffer(std::vector<uint8_t> &recvBuffer, Packet &out)
{
    while (recvBuffer.size() >= 2) {
        uint16_t len = (static_cast<uint16_t>(recvBuffer[0]) << 8) | recvBuffer[1];
        if (recvBuffer.size() < 2 + len)
            return StreamStatus::Incomplete;

        std::vector<uint8_t> pktData(recvBuffer.begin() + 2, recvBuffer.begin() + 2 + len);
        recvBuffer.erase(recvBuffer.begin(), recvBuffer.begin() + 2 + len);
        try {
            out = Packet::deserialize(pktData.data(), pktData.size());
            return StreamStatus::Ok;
        } catch (const std::exception &) {
            // malformed packet, skip and continue parsing
            continue;
        }
    }
    return StreamStatus::Incomplete;
}

Protocol::StreamStatus Protocol::consumeChunk(const uint8_t *data, std::size_t len, std::vector<uint8_t> &recvBuffer, Packet &out)
{
    if (len == 0)
        return StreamStatus::Incomplete;
    recvBuffer.insert(recvBuffer.end(), data, data + len);
    return extractFromBuffer(recvBuffer, out);
}
