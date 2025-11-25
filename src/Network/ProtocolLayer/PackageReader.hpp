/*
** EPITECH PROJECT, 2025
** Mystic-Type
** File description:
** implementing framing rule
*/

#ifndef PACKAGEREADER_HPP_
#define PACKAGEREADER_HPP_
#include <cstddef>
#include <vector>
#include <stdexcept>
#include "../TransportLayer/ISocket.hpp"

namespace Network::ProtocolLayer
{
    /**
     * @struct HeaderStrategy_s
     * @brief Structure defining the header strategy for framing.
     */
    typedef struct HeaderStrategy_s
    {
        std::size_t headerSize;   // Total size of the header in bytes
        std::size_t lengthOffset; // Offset of the length field within the header in bytes
        std::size_t lengthSize;   // Size of the length field in bytes
        std::size_t typeOffset;   // Offset of the type field within the header
        std::size_t typeSize;     // Size of the type field in bytes
        std::size_t magicNumber;  // Magic number for validation
        std::size_t maxLength;    // Maximum allowed length of the payload in bytes
    } HeaderStrategy_t;

    /**
     * @enum FramingType
     * @brief Enumeration of different framing strategies.
     */
    enum FramingType
    {
        LENGTH_PREFIXED, // frames are prefixed with their length
        DELIMITER_BASED, // frames are separated by a specific delimiter
        FIXED_LENGTH,    // frames have a fixed length
        HEADER_BASED     // frames contain a header with metadata
    };

    /**
     * @union FramingStrategyDetails_u
     * @brief Union defining details for different framing strategies.
     */
    typedef union FramingStrategyDetail_u
    {
        HeaderStrategy_t header;    // Strategy details for header-based framing
        std::size_t prefixedLength; // Strategy details for length-prefixed framing
        unsigned char delimiter;    // Delimiter character for delimiter-based framing
        std::size_t fixedLength;    // Fixed length for fixed-length framing
    } FramingStrategyDetail_t;

    /**
     * @struct FramingStrategy_s
     * @brief Structure defining a framing strategy.
     */
    typedef struct FramingStrategy_s
    {
        FramingType type;                // Type of framing strategy
        FramingStrategyDetail_t details; // Details specific to the framing strategy
    } FramingStrategy_t;

    typedef unsigned char byte;

    /**
     * @class PackageReader
     * @brief Interface for framing strategy implementations.
     */
    template <std::size_t N>
    class PackageReader
    {
    public:
        /**
         * @brief Construct a new PackageReader object
         * @param strategy The framing strategy to be used
         */
        PackageReader(const FramingStrategy_t &strategy) : _strategy(strategy) {};

        /**
         * @brief Construct a new PackageReader object with no strategy need to implement later
         */
        PackageReader() = default;

        /**
         * @brief Destroy the PackageReader object
         */
        ~PackageReader() = default;

        /**
         * @brief Set the framing strategy
         *
         * @param strategy The framing strategy to be set
         */
        inline void setStrategy(const FramingStrategy_t &strategy) { _strategy = strategy; };

        ssize_t readFromSock(TransportLayer::ISocket &sock)
        {
            memset(_buffer, 0, _bufferSize);
            ssize_t nread = sock.readByte(_buffer, _bufferSize);
            switch (nread)
            {
            case 0:
                return 0;
            case -1:
                std::runtime_error("Error while reading from socket");
                return -1;
            default:
                this->fillHeader();
                this->fillBuffer();
                return nread;
            }
        }

        inline std::vector<byte> getPayload(void) const { return _payload; }

        inline std::vector<byte> getHeader(void) const { return _header; }

    protected:
    private:
        byte _buffer[N];
        std::size_t _bufferSize = N;
        std::vector<byte> _header;   // package header
        std::vector<byte> _payload;  // payload package
        FramingStrategy_t _strategy; // Framing strategy

        std::size_t byteToSize(const std::vector<byte> buf, const std::size_t off, const std::size_t len) const
        {
            std::size_t size = 0;

            if ((off + len) > buf.size())
                return -1;
            std::cout << "i = " << off + len << std::endl;
            for (ssize_t i = (off + len) - 1; i >= (ssize_t)off; i--)
            {
                std::cout << "byte[" << i << "]: " << buf[i] << std::endl;
                size = (size << 8) | buf[i];
            }
            return size;
        }

        std::size_t lenFromHeader(void) const
        {
            std::size_t headerOff = 0;
            std::size_t len = 0;

            switch (_strategy.type)
            {
            case FramingType::HEADER_BASED:
                headerOff = _strategy.details.header.lengthOffset;
                len = _strategy.details.header.lengthSize;
                return this->byteToSize(_header, headerOff, len);
            case FramingType::LENGTH_PREFIXED:
                len = _strategy.details.prefixedLength;
                return this->byteToSize(_header, 0, len);
            default:
                return 0;
            }
            return 0;
        }

        void fillHeader(void)
        {
            _header.clear();
            switch (_strategy.type)
            {
            case FramingType::HEADER_BASED:
                for (std::size_t i = 0; i < _strategy.details.header.headerSize; i++)
                {
                    _header.push_back(_buffer[i]);
                }
                break;
            case FramingType::LENGTH_PREFIXED:
                for (std::size_t i = 0; i < _strategy.details.prefixedLength; i++)
                    _header.push_back(_buffer[i]);
                break;
            default:
                break;
            }
        }

        void fillBuffer(void)
        {
            std::size_t payloadLen = this->lenFromHeader();
            std::size_t headerSize = 0;
            std::size_t fixedLen = _strategy.details.fixedLength;
            byte delim = _strategy.details.delimiter;

            _payload.clear();
            switch (_strategy.type)
            {
            case FramingType::HEADER_BASED:
                std::cout << "payload len: " << payloadLen << std::endl;
                headerSize = _strategy.details.header.headerSize;
                for (std::size_t i = headerSize; i < (headerSize + payloadLen); i++)
                {
                    _payload.push_back(_buffer[i]);
                }
                break;
            case FramingType::LENGTH_PREFIXED:
                headerSize = _strategy.details.prefixedLength;
                for (std::size_t i = headerSize; i < payloadLen; i++)
                    _payload.push_back(_buffer[i]);
                break;
            case FramingType::DELIMITER_BASED:
                for (std::size_t i = 0; _buffer[i] != delim; i++)
                {
                    if (i > _bufferSize)
                        break;
                    _payload.push_back(_buffer[i]);
                }
                break;
            case FramingType::FIXED_LENGTH:
                for (std::size_t i = 0; i < fixedLen; i++)
                {
                    if (i > _bufferSize)
                        break;
                    _payload.push_back(_buffer[i]);
                }
                break;
            default:
                break;
            }
        }
    };
}
#endif /* !PACKAGEREADER_HPP_ */
