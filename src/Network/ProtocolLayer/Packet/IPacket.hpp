/*
** EPITECH PROJECT, 2025
** Mystic-Type
** File description:
** IPacket
*/

#ifndef IPACKET_HPP_
#define IPACKET_HPP_
#include <cstddef>
#include <string.h>

namespace Network::ProtocolLayer
{
/**
 * @brief Penum to define packet type
 *
 */
typedef enum PacketType_e : unsigned char
{
    BINARYPACKET,
    STRINGPACKET
} PacketType;

class IPacket
{
  public:
    /**
     * @brief Destroy the IPacket object
     *
     */
    virtual ~IPacket() = default;

    /**
     * @brief Set the size of the packet
     *
     * @param size packet's size
     */
    virtual inline void setSize(std::size_t size) = 0;

    /**
     * @brief Set type of the packet
     *
     * @param type between binary or string
     */
    virtual inline void setType(PacketType type) = 0;

    /**
     * @brief Get Packet size in bytes
     *
     * @return std::size_t sizeof the packets
     */
    virtual inline std::size_t getSize(void) const = 0;

    /**
     * @brief Get packet's type (binary or string)
     *
     * @return PacketType packet's type
     */
    virtual inline PacketType getType(void) const = 0;

  protected:
  private:
};
} // namespace Network::ProtocolLayer

#endif /* !IPACKET_HPP_ */
