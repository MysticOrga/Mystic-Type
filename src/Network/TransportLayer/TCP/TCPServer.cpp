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
#include <thread>
#include <unistd.h>
#include <netinet/tcp.h>
#include <sys/select.h>
#include "../Protocol.hpp"

namespace {
    constexpr uint8_t kDefaultPlayerHp = 5;
    std::string sanitizePseudo(const std::string &raw)
    {
        std::string out;
        out.reserve(raw.size());
        for (char c : raw) {
            if ((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z')
                || (c >= '0' && c <= '9') || c == '_' || c == '-') {
                out.push_back(c);
            }
            if (out.size() >= 12)
                break;
        }
        return out;
    }

    std::vector<uint8_t> encodeU64(uint64_t value)
    {
        std::vector<uint8_t> out(8);
        for (int i = 7; i >= 0; --i) {
            out[7 - i] = static_cast<uint8_t>((value >> (i * 8)) & 0xFF);
        }
        return out;
    }

    bool decodeU64(const std::vector<uint8_t> &data, uint64_t &out)
    {
        if (data.size() < 8)
            return false;
        uint64_t value = 0;
        for (size_t i = 0; i < 8; ++i) {
            value = (value << 8) | static_cast<uint64_t>(data[i]);
        }
        out = value;
        return true;
    }
}

TCPServer::TCPServer(uint16_t port, SessionManager &sessions, ChildProcessManager *childMgr)
    : _sessions(sessions), _childMgr(childMgr)
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

void TCPServer::broadcastToLobby(const std::string &lobbyCode, const Packet &packet)
{
    for (auto &c : _clients) {
        if (c.fd == -1 || !c.handshakeDone || c.lobbyCode != lobbyCode)
            continue;
        sendPacket(c.fd, packet);
    }
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
    client.lastPingSentMs = 0;
    client.pingMs = 0;
    client.posX = 0;
    client.posY = 0;
    client.hp = 0;
    client.lobbyCode.clear();
    client.pseudo.clear();
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
    int flag = 1;
    setsockopt(clientFd, IPPROTO_TCP, TCP_NODELAY, &flag, sizeof(flag));

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
    _clients[slot].hp = kDefaultPlayerHp;
    _sessions.addSession(_clients[slot].id, clientFd, addr, getCurrentTime());
    _clients[slot].handshakeStart = getCurrentTime();
    sendPacket(_clients[slot].fd, makeStringPacket(PacketType::SERVER_HELLO, "R-Type Server"));

    std::cout << "[SERVER] client " << _clients[slot].id << " connected (awaiting CLIENT_HELLO)\n";
}

void TCPServer::processClientData(Client &client)
{
    Packet packet;
    auto res = receivePacket(client.fd, packet, client.recvBuffer);
    if (res == RecvResult::Disconnected) {
        std::cout << "[SERVER] client " << client.id << " disconnected\n";
        if (client.handshakeDone && !client.lobbyCode.empty()) {
            std::string sys = "SYS:" + client.pseudo + " disconnected";
            broadcastToLobby(client.lobbyCode, makeStringPacket(PacketType::MESSAGE, sys));
        }
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
        std::string pseudo;
        auto sep = payloadStr.find('|');
        if (sep != std::string::npos && sep + 1 < payloadStr.size()) {
            pseudo = payloadStr.substr(sep + 1);
        }
        pseudo = sanitizePseudo(pseudo);
        if (pseudo.empty()) {
            pseudo = "Player" + std::to_string(client.id);
        }
        sendPacket(client.fd, makeIdPacket(PacketType::OK, client.id));
        client.handshakeDone = true;
        client.pseudo = pseudo;
        client.lastPongTime = getCurrentTime();
        _sessions.setPseudo(client.id, pseudo);
        std::cout << "[SERVER] client " << client.id << " handshake done (awaiting lobby selection)\n";
        return;
    }

    if (packet.type == PacketType::PONG) {
        client.lastPongTime = getCurrentTime();
        _sessions.updatePong(client.id, client.lastPongTime);
        long long nowMs = getCurrentTimeMs();
        uint64_t sentMs = 0;
        int rtt = 0;
        if (decodeU64(packet.payload, sentMs)) {
            rtt = static_cast<int>(std::max<long long>(0, nowMs - static_cast<long long>(sentMs)));
        } else if (client.lastPingSentMs > 0) {
            rtt = static_cast<int>(std::max<long long>(0, nowMs - client.lastPingSentMs));
        }
        client.pingMs = rtt;
        if (!client.lobbyCode.empty()) {
            std::string pingMsg = "PING:" + std::to_string(client.id) + ":" + std::to_string(rtt);
            broadcastToLobby(client.lobbyCode, makeStringPacket(PacketType::MESSAGE, pingMsg));
        }
        std::cout << "[SERVER] Received PONG from client " << client.id << std::endl;
        return;
    }

    if (packet.type == PacketType::CREATE_LOBBY || packet.type == PacketType::JOIN_LOBBY) {
        handleLobbyPacket(client, packet);
        return;
    }

    if (packet.type == PacketType::MESSAGE) {
        std::cerr << "message\n";
        std::string lobbyCode = client.lobbyCode;
        if (lobbyCode.empty()) {
            auto lobbyOpt = _sessions.getLobbyCode(client.id);
            if (lobbyOpt.has_value())
                lobbyCode = *lobbyOpt;
        }
        std::cout << "[SERVER] CHAT from id=" << client.id
                << " lobby=" << (lobbyCode.empty() ? "?" : lobbyCode) << "\n";
        if (!lobbyCode.empty()) {
            std::string text(packet.payload.begin(), packet.payload.end());
            std::string clean;
            clean.reserve(text.size());
            for (char c : text) {
                if (c >= 32 && c <= 126) {
                    clean.push_back(c);
                }
                if (clean.size() >= 120)
                    break;
            }
            if (!clean.empty()) {
                std::string name = client.pseudo;
                if (name.empty()) {
                    auto pseudoOpt = _sessions.getPseudo(client.id);
                    if (pseudoOpt.has_value())
                        name = *pseudoOpt;
                }
                if (name.empty()) {
                    name = "Player" + std::to_string(client.id);
                }
                std::string msg = "CHAT:" + name + ": " + clean;
                int recipients = 0;
                for (auto &c : _clients) {
                    if (c.fd != -1 && c.handshakeDone && c.lobbyCode == lobbyCode)
                        recipients++;
                }
                std::cout << "[SERVER] Broadcast lobby=" << lobbyCode << " recipients=" << recipients
                          << " msg=\"" << msg << "\"\n";
                broadcastToLobby(lobbyCode, makeStringPacket(PacketType::MESSAGE, msg));
            }
        } else {
            std::string text(packet.payload.begin(), packet.payload.end());
            std::string clean;
            clean.reserve(text.size());
            for (char c : text) {
                if (c >= 32 && c <= 126) {
                    clean.push_back(c);
                }
                if (clean.size() >= 120)
                    break;
            }
            if (!clean.empty()) {
                std::string name = client.pseudo.empty() ? ("Player" + std::to_string(client.id)) : client.pseudo;
                std::string msg = "CHAT:" + name + ": " + clean;
                int recipients = 0;
                for (auto &c : _clients) {
                    if (c.fd != -1 && c.handshakeDone)
                        recipients++;
                }
                std::cout << "[SERVER] Broadcast ALL recipients=" << recipients << " msg=\"" << msg << "\"\n";
                for (auto &c : _clients) {
                    if (c.fd != -1 && c.handshakeDone)
                        sendPacket(c.fd, makeStringPacket(PacketType::MESSAGE, msg));
                }
            }
        }
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

long long TCPServer::getCurrentTimeMs()
{
    using namespace std::chrono;
    return duration_cast<milliseconds>(steady_clock::now().time_since_epoch()).count();
}


void TCPServer::sendPingToAll()
{
    long long nowMs = getCurrentTimeMs();
    std::vector<uint8_t> payload = encodeU64(static_cast<uint64_t>(nowMs));
    for (auto &c : _clients) {
        if (c.fd != -1 && c.handshakeDone) {
            std::cout << "[SERVER] Sending PING to client " << c.id << std::endl;
            c.lastPingSentMs = nowMs;
            sendPacket(c.fd, Packet(PacketType::PING, payload));
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
            if (c.handshakeDone && !c.lobbyCode.empty()) {
                std::string sys = "SYS:" + c.pseudo + " disconnected";
                broadcastToLobby(c.lobbyCode, makeStringPacket(PacketType::MESSAGE, sys));
            }
            resetClient(c);
        }
    }
}

void TCPServer::processIpcMessages()
{
    for (auto &kv : _lobbies) {
        auto &ipc = kv.second.ipc;
        if (!ipc)
            continue;
        while (true) {
            auto msgOpt = ipc->recv(0);
            if (!msgOpt.has_value())
                break;
            const std::string &msg = *msgOpt;
            if (msg.rfind("BOSS:", 0) == 0) {
                std::string lobbyCode = msg.substr(5);
                if (!lobbyCode.empty()) {
                    broadcastToLobby(lobbyCode, makeStringPacket(PacketType::MESSAGE, "SYS:Boss spawned"));
                }
                continue;
            }
            if (msg.rfind("DEAD:", 0) == 0) {
                int id = 0;
                try {
                    id = std::stoi(msg.substr(5));
                } catch (const std::exception &) {
                    continue;
                }
                if (id <= 0)
                    continue;
                for (auto &c : _clients) {
                    if (c.fd != -1 && c.id == id) {
                        if (!c.lobbyCode.empty()) {
                            std::string sys = "SYS:" + c.pseudo + " died";
                            broadcastToLobby(c.lobbyCode, makeStringPacket(PacketType::MESSAGE, sys));
                        }
                        sendPacket(c.fd, makeStringPacket(PacketType::MESSAGE, "DEAD"));
                        resetClient(c);
                        break;
                    }
                }
            }
        }
    }
}

void TCPServer::run()
{
    long lastPing = getCurrentTime();

    while (true) {
        processIpcMessages();
        long now = getCurrentTime();
        if (now - lastPing >= 1) {
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
        for (const auto &kv : _lobbies) {
            if (!kv.second.ipc)
                continue;
            int ipcFd = kv.second.ipc->fd();
            if (ipcFd != -1) {
                FD_SET(ipcFd, &readfds);
                maxFd = std::max(maxFd, ipcFd);
            }
        }

        struct timeval tv;
        tv.tv_sec = 0;
        tv.tv_usec = 1000;

        int activity = pollSockets(readfds, maxFd, tv);
        if (activity < 0) {
            continue;
        }

        if (activity == 0) {
            std::this_thread::sleep_for(std::chrono::milliseconds(1));
            continue;
        }

        for (const auto &kv : _lobbies) {
            if (!kv.second.ipc)
                continue;
            int ipcFd = kv.second.ipc->fd();
            if (ipcFd != -1 && FD_ISSET(ipcFd, &readfds)) {
                processIpcMessages();
                break;
            }
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
    payload.reserve(1 + _clients.size() * 5);

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
        payload.push_back(c.hp);
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
        newClient.posY,
        newClient.hp
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

uint16_t TCPServer::allocatePort()
{
    static uint16_t nextPort = 50000;
    if (nextPort == 0 || nextPort >= 65000)
        nextPort = 50000;
    return nextPort++;
}

void TCPServer::ensureLobbyProcess(const std::string &code, bool isPublic)
{
    auto it = _lobbies.find(code);
    if (it == _lobbies.end())
        return;
    if (it->second.udpPort == 0) {
        it->second.udpPort = allocatePort();
    }
    if (_childMgr) {
        if (it->second.ipcPath.empty()) {
            it->second.ipcPath = "/tmp/rtype_" + code + ".sock";
        }
        if (!it->second.ipc) {
            it->second.ipc = std::make_unique<IpcChannel>();
            if (!it->second.ipc->bindServer(it->second.ipcPath)) {
                std::cerr << "[PARENT] Failed to bind IPC at " << it->second.ipcPath << "\n";
            }
        }
        _childMgr->spawn(code, it->second.udpPort, it->second.ipcPath);
        std::cout << "[PARENT] UDP servers active " << _childMgr->activeCount()
                  << "/" << _childMgr->maxCount() << "\n";
    }
    (void)isPublic;
}

bool TCPServer::assignLobby(Client &client, const std::string &code, bool createIfMissing, bool isPublic, bool allowFull)
{
    removeFromLobby(client);

    auto it = _lobbies.find(code);
    if (it == _lobbies.end()) {
        if (!createIfMissing)
            return false;
        _lobbies[code] = LobbyInfo{isPublic, {}, 0};
        it = _lobbies.find(code);
        ensureLobbyProcess(code, isPublic);
    }

    if (!allowFull && it->second.players.size() >= MAX_CLIENT)
        return false;

    it->second.players.push_back(client.id);
    client.lobbyCode = code;
    _clientLobby[client.id] = code;
    _sessions.setLobbyCode(client.id, code);
    if (it->second.udpPort == 0) {
        ensureLobbyProcess(code, isPublic);
    }
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
        // For now, send code|port in the payload so the client can aim UDP correctly.
        uint16_t port = _lobbies[code].udpPort ? _lobbies[code].udpPort : 4243;
        std::string payload = code + "|" + std::to_string(port);
        sendPacket(client.fd, makeLobbyPacket(PacketType::LOBBY_OK, payload));
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
        uint16_t port = _lobbies[code].udpPort ? _lobbies[code].udpPort : 4243;
        std::string payload = code + "|" + std::to_string(port);
        sendPacket(client.fd, makeLobbyPacket(PacketType::LOBBY_OK, payload));
        refreshLobby(code);
        sendPlayerListToClient(client);
        broadcastNewPlayer(client);
    }
}
