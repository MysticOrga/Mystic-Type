/*
** EPITECH PROJECT, 2025
** Mystic-Type
** File description:
** StringPacket
*/

#ifndef STRINGPACKET_HPP_
#define STRINGPACKET_HPP_
#include "APacket.hpp"
#include <string>

namespace Network::ProtocolLayer
{
class StringPacket : public APacket
{
  public:
    /**
     * @brief Construct a new String Packet object
     *
     */
    StringPacket() : APacket(STRINGPACKET) {};

    /**
     * @brief Construct a new String Packet object
     *
     * @param packet
     */
    StringPacket(std::string packet) : APacket(packet.length(), STRINGPACKET) {};

    /**
     * @brief Destroy the String Packet object
     *
     */
    ~StringPacket() = default;

    /**
     * @brief Set string packet
     *
     * @param packet
     */
    void setPacket(std::string packet)
    {
        this->setSize(packet.length());
        _packet = packet;
    };

    /**
     * @brief Get internal string packet
     *
     * @return string packet
     */
    inline std::string getPacket(void) const { return _packet; };

  protected:
  private:
    std::string _packet;
};

} // namespace Network::ProtocolLayer

#endif /* !STRINGPACKET_HPP_ */
