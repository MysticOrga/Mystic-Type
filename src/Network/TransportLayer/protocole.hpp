#ifndef PROTOCOL_HPP_
#define PROTOCOL_HPP_

#include <cstdint>
#include <cstring>
#include <array>
#include <vector>
#include <stdexcept>
#include <arpa/inet.h>

namespace Network::Protocol
{
    // Magic number pour identifier le protocole
    constexpr uint16_t MAGIC_NUMBER = 0xCAFE;
    constexpr std::size_t HEADER_SIZE = 4;
    constexpr std::size_t MAX_PAYLOAD_SIZE = 255;

    /**
     * @enum PacketType
     * @brief Types de paquets supportés par le protocole
     */
    enum class PacketType : uint8_t
    {
        CONNECTION = 0x01,
        DISCONNECTION = 0x02,
        DATA = 0x03,
        HEARTBEAT = 0x04,
        ACK = 0x05,
        ERROR = 0xFF
    };

    /**
     * @struct PacketHeader
     * @brief Structure du header de paquet (4 octets)
     * Format: [MagicNumber:2][PayloadSize:1][Type:1]
     */
    struct PacketHeader
    {
        uint16_t magicNumber;  // 2 octets - Identification du protocole (0xCAFE)
        uint8_t payloadSize;   // 1 octet  - Taille du payload (0-255)
        uint8_t type;          // 1 octet  - Type de paquet

        /**
         * @brief Constructeur par défaut
         */
        PacketHeader() : magicNumber(MAGIC_NUMBER), payloadSize(0), type(0) {}

        /**
         * @brief Constructeur avec type et taille
         * @param packetType Type du paquet
         * @param size Taille du payload
         */
        PacketHeader(PacketType packetType, uint8_t size = 0)
            : magicNumber(MAGIC_NUMBER),
              payloadSize(size),
              type(static_cast<uint8_t>(packetType))
        {}

        /**
         * @brief Vérifie si le header est valide (magic number correct)
         * @return true si valide, false sinon
         */
        inline bool isValid() const
        {
            return magicNumber == MAGIC_NUMBER;
        }

        /**
         * @brief Récupère le type de paquet
         * @return PacketType Le type du paquet
         */
        inline PacketType getType() const
        {
            return static_cast<PacketType>(type);
        }

        /**
         * @brief Sérialise le header en tableau d'octets (network byte order)
         * @return std::array<uint8_t, HEADER_SIZE> Le header sérialisé
         */
        std::array<uint8_t, HEADER_SIZE> serialize() const
        {
            std::array<uint8_t, HEADER_SIZE> buffer;
            uint16_t netMagic = htons(magicNumber);
            
            std::memcpy(buffer.data(), &netMagic, sizeof(uint16_t));
            buffer[2] = payloadSize;
            buffer[3] = type;
            
            return buffer;
        }

        /**
         * @brief Désérialise un tableau d'octets en header (network byte order)
         * @param data Pointeur vers les données à désérialiser
         * @return PacketHeader Le header désérialisé
         */
        static PacketHeader deserialize(const uint8_t* data)
        {
            PacketHeader header;
            uint16_t netMagic;
            
            std::memcpy(&netMagic, data, sizeof(uint16_t));
            header.magicNumber = ntohs(netMagic);
            header.payloadSize = data[2];
            header.type = data[3];
            
            return header;
        }
    };

    /**
     * @class Packet
     * @brief Représente un paquet complet (header + payload)
     */
    class Packet
    {
    public:
        /**
         * @brief Constructeur par défaut
         */
        Packet() = default;

        /**
         * @brief Constructeur avec type et payload
         * @param type Type du paquet
         * @param payload Pointeur vers les données du payload (peut être nullptr)
         * @param size Taille du payload
         * @throws std::invalid_argument si la taille dépasse MAX_PAYLOAD_SIZE
         */
        Packet(PacketType type, const uint8_t* payload = nullptr, uint8_t size = 0)
            : _header(type, size)
        {
            if (payload && size > 0)
            {
                if (size > MAX_PAYLOAD_SIZE)
                    throw std::invalid_argument("Payload size exceeds maximum");
                
                _payload.resize(size);
                std::memcpy(_payload.data(), payload, size);
            }
        }

        /**
         * @brief Crée un paquet à partir de données brutes reçues
         * @param data Pointeur vers les données brutes
         * @param totalSize Taille totale des données
         * @return Packet Le paquet créé
         * @throws std::invalid_argument si les données sont invalides
         */
        static Packet fromRawData(const uint8_t* data, std::size_t totalSize)
        {
            if (totalSize < HEADER_SIZE)
                throw std::invalid_argument("Data too small for header");

            PacketHeader header = PacketHeader::deserialize(data);
            
            if (!header.isValid())
                throw std::invalid_argument("Invalid magic number");

            if (totalSize < HEADER_SIZE + header.payloadSize)
                throw std::invalid_argument("Incomplete packet data");

            Packet packet;
            packet._header = header;
            
            if (header.payloadSize > 0)
            {
                packet._payload.resize(header.payloadSize);
                std::memcpy(packet._payload.data(), data + HEADER_SIZE, header.payloadSize);
            }

            return packet;
        }

        /**
         * @brief Sérialise le paquet complet (header + payload)
         * @return std::vector<uint8_t> Le paquet sérialisé
         */
        std::vector<uint8_t> serialize() const
        {
            auto headerData = _header.serialize();
            std::vector<uint8_t> buffer(headerData.begin(), headerData.end());
            buffer.insert(buffer.end(), _payload.begin(), _payload.end());
            return buffer;
        }

        /**
         * @brief Récupère le header du paquet
         */
        inline const PacketHeader& getHeader() const { return _header; }
        
        /**
         * @brief Récupère le payload du paquet
         */
        inline const std::vector<uint8_t>& getPayload() const { return _payload; }
        
        /**
         * @brief Récupère le type du paquet
         */
        inline PacketType getType() const { return _header.getType(); }
        
        /**
         * @brief Récupère la taille totale du paquet (header + payload)
         */
        inline std::size_t getTotalSize() const { return HEADER_SIZE + _payload.size(); }

    private:
        PacketHeader _header;
        std::vector<uint8_t> _payload;
    };

} // namespace Network::Protocol

#endif // PROTOCOL_HPP_