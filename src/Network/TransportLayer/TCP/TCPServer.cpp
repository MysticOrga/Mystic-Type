/*
** EPITECH PROJECT, 2025
** Mystic-Type
** File description:
** TCPServer
*/

#include "TCPServer.hpp"
#include <algorithm>
#include <chrono>
#include <cstdint>
#include <iostream>
#include <string>
#include <random>
#include <unistd.h>
#include <sys/select.h>
#include "../Protocol.hpp"

TCPServer::TCPServer(uint16_t port, SessionManager &sessions)
    : _sessions(sessions)
{
    for (auto &c : _clients) {
        c.fd = -1;
    }
    if (!_serverSocket.bindAndListen(port, INADDR_ANY, MAX_CLIENT)) {
        throw std::runtime_error("Failed to start TCP server");
    }

    std::cout << "[SERVER] TCPServer started on port " << port << std::endl;
}

TCPServer::~TCPServer()
{
    for (auto &c : _clients) {
        resetClient(c);
    }
    _serverSocket.closeSocket();
}

Packet TCPServer::makeStringPacket(PacketType type, const std::string &payload)
{
    return Packet{type, std::vector<uint8_t>(payload.begin(), payload.end())};
}

Packet TCPServer::makeIdPacket(PacketType type, int value)
{
    std::vector<uint8_t> payload{
        static_cast<uint8_t>((value >> 8) & 0xFF),
        static_cast<uint8_t>(value & 0xFF)
    };
    return Packet{type, payload};
}

Packet TCPServer::makeLobbyPacket(PacketType type, const std::string &payload)
{
    return Packet{type, std::vector<uint8_t>(payload.begin(), payload.end())};
}

bool TCPServer::sendPacket(int fd, const Packet &packet)
{
    if (fd == -1) {
        return false;
    }
    std::vector<uint8_t> framed;
    try {
        framed = Protocol::frameTcp(packet);
    } catch (const std::exception &) {
        return false;
    }
    return writeAll(fd, framed.data(), framed.size());
}

TCPServer::RecvResult TCPServer::receivePacket(int fd, Packet &packet, std::vector<uint8_t> &recvBuffer)
{
    if (fd == -1) {
        return RecvResult::Disconnected;
    }

    uint8_t tmp[BUFFER_SIZE]{};
    ssize_t n = readFd(fd, tmp, sizeof(tmp));
    if (n <= 0) {
        return RecvResult::Disconnected;
    }

    recvBuffer.insert(recvBuffer.end(), tmp, tmp + n);

    auto status = Protocol::consumeChunk(tmp, static_cast<std::size_t>(n), recvBuffer, packet);
    if (status == Protocol::StreamStatus::Ok)
        return RecvResult::Ok;
    if (status == Protocol::StreamStatus::Incomplete)
        return RecvResult::Incomplete;
    return RecvResult::Disconnected;
}

bool TCPServer::waitForReadable(int fd, int timeoutSec, int timeoutUsec)
{
    fd_set rfds;
    FD_ZERO(&rfds);
    FD_SET(fd, &rfds);

    struct timeval timeout{timeoutSec, timeoutUsec};

    int ret = selectFdSet(fd + 1, &rfds, nullptr, nullptr, &timeout);
    return ret > 0 && FD_ISSET(fd, &rfds);
}

void TCPServer::closeFd(int &fd)
{
    if (fd != -1) {
        closeFdRaw(fd);
        fd = -1;
    }
}

void TCPServer::resetClient(Client &client)
{
    removeFromLobby(client);
    _sessions.removeById(client.id);
    closeFd(client.fd);
    client.id = 0;
    client.addr = {};
    client.handshakeDone = false;
    client.lastPongTime = 0;
    client.handshakeStart = 0;
    client.posX = 0;
    client.posY = 0;
    client.lobbyCode.clear();
    client.recvBuffer.clear();
}

int TCPServer::pollSockets(fd_set &readfds, int maxFd, struct timeval &timeout)
{
    return selectFdSet(maxFd + 1, &readfds, nullptr, nullptr, &timeout);
}

void TCPServer::acceptNewClient()
{
    sockaddr_in addr{};
    int clientFd = _serverSocket.acceptClient(addr);

    if (clientFd == -1)
        return;

    int slot = -1;
    for (size_t i = 0; i < _clients.size(); i++) {
        if (_clients[i].fd == -1) {
            slot = i;
            break;
        }
    }

    if (slot == -1) {
        sendPacket(clientFd, makeStringPacket(PacketType::REFUSED, "FULL"));
        closeFd(clientFd);
        std::cout << "[SERVER] refused new client (FULL)\n";
        return;
    }

    _clients[slot].fd = clientFd;
    _clients[slot].addr = addr;
    _clients[slot].id = _nextId++;
    _clients[slot].posX = static_cast<uint8_t>(slot);
    _clients[slot].posY = 0;
    _sessions.addSession(_clients[slot].id, clientFd, addr, getCurrentTime());
    _clients[slot].handshakeStart = getCurrentTime();
    // Send SERVER_HELLO immediately; wait for CLIENT_HELLO asynchronously
    sendPacket(_clients[slot].fd, makeStringPacket(PacketType::SERVER_HELLO, "R-Type Server"));

    std::cout << "[SERVER] client " << _clients[slot].id << " connected (awaiting CLIENT_HELLO)\n";
}

void TCPServer::processClientData(Client &client)
{
    Packet packet;
    auto res = receivePacket(client.fd, packet, client.recvBuffer);
    if (res == RecvResult::Disconnected) {
        std::cout << "[SERVER] client " << client.id << " disconnected\n";
        resetClient(client);
        return;
    }
    if (res == RecvResult::Incomplete) {
        return;
    }

    if (!client.handshakeDone) {
        std::string payloadStr(packet.payload.begin(), packet.payload.end());
        if (packet.type != PacketType::CLIENT_HELLO || payloadStr.rfind("toto", 0) != 0) {
            sendPacket(client.fd, makeStringPacket(PacketType::REFUSED, "BAD_HANDSHAKE"));
            resetClient(client);
            return;
        }
        sendPacket(client.fd, makeIdPacket(PacketType::OK, client.id));
        client.handshakeDone = true;
        client.lastPongTime = getCurrentTime();
        std::cout << "[SERVER] client " << client.id << " handshake done (awaiting lobby selection)\n";
        return;
    }

    if (packet.type == PacketType::PONG) {
        client.lastPongTime = getCurrentTime();
        _sessions.updatePong(client.id, client.lastPongTime);
        std::cout << "[SERVER] Received PONG from client " << client.id << std::endl;
        return;
    }

    if (packet.type == PacketType::CREATE_LOBBY || packet.type == PacketType::JOIN_LOBBY) {
        handleLobbyPacket(client, packet);
        return;
    }

    std::string payloadStr(packet.payload.begin(), packet.payload.end());
    std::cout << "[SERVER] (" << client.id << ") packet type " << static_cast<int>(packet.type)
            << " payload: " << payloadStr << std::endl;
}


long TCPServer::getCurrentTime()
{
    using namespace std::chrono;
    return duration_cast<seconds>(steady_clock::now().time_since_epoch()).count();
}


void TCPServer::sendPingToAll()
{
    for (auto &c : _clients) {
        if (c.fd != -1 && c.handshakeDone) {
            std::cout << "[SERVER] Sending PING to client " << c.id << std::endl;
            sendPacket(c.fd, Packet(PacketType::PING, {}));
        }
    }
}

void TCPServer::checkHeartbeat()
{
    long now = getCurrentTime();

    for (auto &c : _clients) {
        if (c.fd == -1)
            continue;

        if (!c.handshakeDone) {
            if (now - c.handshakeStart > 3) {
                sendPacket(c.fd, makeStringPacket(PacketType::REFUSED, "TIMEOUT"));
                resetClient(c);
            }
            continue;
        }
        if (now - c.lastPongTime > 10) {
            std::cout << "[SERVER] Client " << c.id << " timed out (no PONG for " << (now - c.lastPongTime) << "s)" << std::endl;

            resetClient(c);
        }
    }
}

void TCPServer::run()
{
    long lastPing = getCurrentTime();

    while (true) {
        long now = getCurrentTime();
        if (now - lastPing >= 5) {
            sendPingToAll();
            checkHeartbeat();
            lastPing = now;
        }

        fd_set readfds;
        FD_ZERO(&readfds);

        int maxFd = _serverSocket.getSocketFd();
        FD_SET(_serverSocket.getSocketFd(), &readfds);

        for (auto &c : _clients) {
            if (c.fd != -1) {
                FD_SET(c.fd, &readfds);
                maxFd = std::max(maxFd, c.fd);
            }
        }

        struct timeval tv;
        tv.tv_sec = 1;
        tv.tv_usec = 0;

        int activity = pollSockets(readfds, maxFd, tv);
        if (activity < 0) {
            continue;
        }

        if (activity == 0) {
            continue;
        }

        if (FD_ISSET(_serverSocket.getSocketFd(), &readfds)) {
            acceptNewClient();
        }

        for (auto &c : _clients) {
            if (c.fd != -1 && FD_ISSET(c.fd, &readfds)) {
                processClientData(c);
            }
        }
    }
}

ssize_t TCPServer::writeFd(int fd, const uint8_t *data, std::size_t size)
{
    return ::write(fd, data, size);
}

ssize_t TCPServer::readFd(int fd, uint8_t *data, std::size_t size)
{
    return ::read(fd, data, size);
}

int TCPServer::selectFdSet(int nfds, fd_set *readfds, fd_set *writefds, fd_set *exceptfds, struct timeval *timeout)
{
    return ::select(nfds, readfds, writefds, exceptfds, timeout);
}

int TCPServer::closeFdRaw(int fd)
{
    return ::close(fd);
}

bool TCPServer::writeAll(int fd, const uint8_t *data, std::size_t size)
{
    std::size_t total = 0;
    while (total < size) {
        ssize_t n = writeFd(fd, data + total, size - total);
        if (n <= 0) {
            return false;
        }
        total += static_cast<std::size_t>(n);
    }
    return true;
}
Packet TCPServer::buildPlayerListPacket(const std::string &lobbyCode) const
{
    std::vector<uint8_t> payload;
    payload.reserve(1 + _clients.size() * 6);

    payload.push_back(0); // placeholder for count
    uint8_t count = 0;

    for (const auto &c : _clients) {
        if (c.fd == -1 || !c.handshakeDone || c.lobbyCode != lobbyCode)
            continue;
        ++count;
        payload.push_back(static_cast<uint8_t>((c.id >> 8) & 0xFF));
        payload.push_back(static_cast<uint8_t>(c.id & 0xFF));
        payload.push_back(c.posX);
        payload.push_back(c.posY);
    }

    payload[0] = count;
    return Packet(PacketType::PLAYER_LIST, payload);
}

void TCPServer::sendPlayerListToClient(const Client &client)
{
    Packet list = buildPlayerListPacket(client.lobbyCode);
    sendPacket(client.fd, list);
}

void TCPServer::broadcastNewPlayer(const Client &newClient)
{
    std::vector<uint8_t> payload{
        static_cast<uint8_t>((newClient.id >> 8) & 0xFF),
        static_cast<uint8_t>(newClient.id & 0xFF),
        newClient.posX,
        newClient.posY
    };
    Packet pkt(PacketType::NEW_PLAYER, payload);

    for (auto &c : _clients) {
        if (c.fd == -1 || !c.handshakeDone || c.id == newClient.id || c.lobbyCode != newClient.lobbyCode)
            continue;
        sendPacket(c.fd, pkt);
    }
}

void TCPServer::refreshLobby(const std::string &code)
{
    for (auto &c : _clients) {
        if (c.fd == -1 || !c.handshakeDone || c.lobbyCode != code)
            continue;
        sendPlayerListToClient(c);
    }
}

std::string TCPServer::generateLobbyCode()
{
    static const char chars[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789";
    static std::mt19937 rng(static_cast<unsigned long>(std::chrono::steady_clock::now().time_since_epoch().count()));
    std::uniform_int_distribution<int> dist(0, static_cast<int>(sizeof(chars) - 2));

    std::string code(6, 'A');
    do {
        for (auto &ch : code) {
            ch = chars[dist(rng)];
        }
    } while (_lobbies.find(code) != _lobbies.end());
    return code;
}

void TCPServer::removeFromLobby(const Client &client)
{
    std::string code = client.lobbyCode;
    auto mapIt = _clientLobby.find(client.id);
    if (code.empty() && mapIt != _clientLobby.end())
        code = mapIt->second;
    if (code.empty())
        return;

    auto it = _lobbies.find(code);
    if (it != _lobbies.end()) {
        auto &vec = it->second.players;
        vec.erase(std::remove(vec.begin(), vec.end(), client.id), vec.end());
    }
    _clientLobby.erase(client.id);
    _sessions.setLobbyCode(client.id, "");
    refreshLobby(code);
}

bool TCPServer::assignLobby(Client &client, const std::string &code, bool createIfMissing, bool isPublic, bool allowFull)
{
    removeFromLobby(client);

    auto it = _lobbies.find(code);
    if (it == _lobbies.end()) {
        if (!createIfMissing)
            return false;
        _lobbies[code] = LobbyInfo{isPublic, {}};
        it = _lobbies.find(code);
    }

    if (!allowFull && it->second.players.size() >= MAX_CLIENT)
        return false;

    it->second.players.push_back(client.id);
    client.lobbyCode = code;
    _clientLobby[client.id] = code;
    _sessions.setLobbyCode(client.id, code);
    return true;
}

std::string TCPServer::autoAssignPublic(Client &client)
{
    const std::string code = "PUBLIC";
    assignLobby(client, code, true, true);
    return code;
}

void TCPServer::handleLobbyPacket(Client &client, const Packet &packet)
{
    if (!client.handshakeDone)
        return;

    if (packet.type == PacketType::CREATE_LOBBY) {
        std::cout << "[SERVER] client " << client.id << " requested CREATE_LOBBY\n";
        std::string code = generateLobbyCode();
        if (!assignLobby(client, code, true, false, true)) {
            sendPacket(client.fd, makeLobbyPacket(PacketType::LOBBY_ERROR, "INVALID_STATE"));
            return;
        }
        std::cout << "[SERVER] client " << client.id << " joined lobby " << code << "\n";
        sendPacket(client.fd, makeLobbyPacket(PacketType::LOBBY_OK, code));
        refreshLobby(code);
        sendPlayerListToClient(client);
        broadcastNewPlayer(client);
        return;
    }

    if (packet.type == PacketType::JOIN_LOBBY) {
        std::string code(packet.payload.begin(), packet.payload.end());
        bool isAutoPublic = (code == "PUBLIC");
        if (code.empty())
            code = "PUBLIC";

        std::cout << "[SERVER] client " << client.id << " requested JOIN_LOBBY " << code << "\n";
        if (!isAutoPublic) {
            auto it = _lobbies.find(code);
            if (it == _lobbies.end()) {
                sendPacket(client.fd, makeLobbyPacket(PacketType::LOBBY_ERROR, "UNKNOWN_CODE"));
                return;
            }
            if (it->second.players.size() >= MAX_CLIENT) {
                sendPacket(client.fd, makeLobbyPacket(PacketType::LOBBY_ERROR, "FULL"));
                return;
            }
            if (!assignLobby(client, code, false, it->second.isPublic)) {
                sendPacket(client.fd, makeLobbyPacket(PacketType::LOBBY_ERROR, "INVALID_STATE"));
                return;
            }
        } else {
            if (!assignLobby(client, code, true, true, false)) {
                sendPacket(client.fd, makeLobbyPacket(PacketType::LOBBY_ERROR, "INVALID_STATE"));
                return;
            }
        }
        std::cout << "[SERVER] client " << client.id << " joined lobby " << code << "\n";
        sendPacket(client.fd, makeLobbyPacket(PacketType::LOBBY_OK, code));
        refreshLobby(code);
        sendPlayerListToClient(client);
        broadcastNewPlayer(client);
    }
}
