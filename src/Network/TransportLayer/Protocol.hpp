/*
** EPITECH PROJECT, 2025
** Mystic-Type
** File description:
** Protocol helpers shared by client and server
*/

#pragma once

#include "Packet.hpp"
#include <cstdint>
#include <vector>

namespace Protocol
{
/**
 * @brief Status for framed TCP stream parsing.
 */
enum class StreamStatus
{
    Disconnected,
    Incomplete,
    Ok
};

/**
 * @brief Build a TCP framed buffer (2 bytes length + serialized packet).
 *
 * @throw std::runtime_error if payload is too large.
 */
std::vector<uint8_t> frameTcp(const Packet &packet);

/**
 * @brief Try to extract a complete packet from an accumulated stream buffer.
 *
 * @param recvBuffer Mutable buffer that stores previous bytes.
 * @param out Parsed packet on success.
 * @return StreamStatus
 */
StreamStatus extractFromBuffer(std::vector<uint8_t> &recvBuffer, Packet &out);

/**
 * @brief Append newly read bytes then attempt extraction.
 */
StreamStatus consumeChunk(const uint8_t *data, std::size_t len, std::vector<uint8_t> &recvBuffer, Packet &out);
} // namespace Protocol
