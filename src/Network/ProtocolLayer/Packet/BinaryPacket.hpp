/*
** EPITECH PROJECT, 2025
** Mystic-Type
** File description:
** BinaryPacket
*/

#ifndef BINARYPACKET_HPP_
#define BINARYPACKET_HPP_
#include "APacket.hpp"

namespace Network::ProtocolLayer
{
template <typename P> class BinaryPacket : public APacket
{
    using PacketType = unsigned char;

  public:
    /**
     * @brief Construct a new BinaryPacket object
     *
     */
    BinaryPacket() : APacket(BINARYPACKET) {};

    /**
     * @brief Construct a new BinaryPacket object
     *
     * @param packet C packet
     */
    BinaryPacket(P packet) : APacket(sizeof(packet), BINARYPACKET)
    {
        memcpy(&_packet, packet, this->getSize());
    }

    /**
     * @brief Destroy the BinaryPacket object
     *
     */
    ~BinaryPacket() = default;

    /**
     * @brief Set internal packet structure
     *
     * @param packet structure to copy from
     */
    void setPacket(P packet)
    {
        this->setSize(sizeof(packet));
        memcpy(&_packet, &packet, this->getSize());
    };

    /**
     * @brief Set internal packet structure
     *
     * @param packet structure to copy from
     * @param size size of the structure
     */
    void setPacket(P packet, std::size_t size)
    {
        this->setSize(size);
        memcpy(&_packet, &packet, this->getSize());
    }

    /**
     * @brief Get internal packet structure
     *
     * @return P Packet strcture
     */
    inline P getPacket(void) const
    {
        return _packet;
    };

  protected:
  private:
    P _packet;
};

} // namespace Network::ProtocolLayer

#endif /* !BINARYPACKET_HPP_ */
