/*
** EPITECH PROJECT, 2025
** Mystic-Type
** File description:
** APacket
*/

#ifndef APACKET_HPP_
#define APACKET_HPP_
#include "IPacket.hpp"

namespace Network::ProtocolLayer
{

class APacket : public IPacket
{
  public:
    /**
     * @brief Construct a new APacket object
     *
     */
    APacket() = default;

    /**
     * @brief Construct a new APacket object
     *
     * @param type type of the packet
     */
    APacket(PacketType type) : _size(0), _type(type) {};

    /**
     * @brief Construct a new APacket object
     *
     * @param size size of the packet un bytes
     */
    APacket(std::size_t size, PacketType type) : _size(size), _type(type) {};

    /**
     * @brief Destroy the APacket object
     *
     */
    ~APacket() = default;

    /**
     * @brief Set the size of the packet
     *
     * @param size packet's size
     */
    inline void setSize(std::size_t size) override
    {
        _size = size;
    };

    /**
     * @brief Set type of the packet
     *
     * @param type between binary or string
     */
    inline void setType(PacketType type) override
    {
        _type = type;
    };

    /**
     * @brief Get Packet size in bytes
     *
     * @return std::size_t size of the packets
     */
    inline std::size_t getSize(void) const override
    {
        return _size;
    };

    /**
     * @brief Get packet's type (binary or string)
     *
     * @return PacketType packet's type
     */
    inline PacketType getType(void) const override
    {
        return _type;
    };

  protected:
  private:
    std::size_t _size;
    PacketType _type;
};

} // namespace Network::ProtocolLayer

#endif /* !APACKET_HPP_ */
