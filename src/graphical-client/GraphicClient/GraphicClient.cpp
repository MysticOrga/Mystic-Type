/*
** EPITECH PROJECT, 2025
** Mystic-Type
** File description:
** GraphicClient
*/

#include "graphical-client/GraphicClient/GraphicClient.hpp"
#include "graphical-client/ecs/Components.hpp"
#include <iostream>
#include <memory>
#include <unordered_set>
#include <cmath>
#include <algorithm>
#include <limits>
#include <cctype>

GraphicClient::GraphicClient(const std::string& ip, int port)
    : _window(1920, 1080, "Mystic-Type"), _net(ip, port)
{
    _window.setTargetFPS(60);
    _lastKeepAlive = std::chrono::steady_clock::now();
}

bool GraphicClient::init()
{
    if (!_net.connectToServer()) return false;
    if (!_net.performHandshake()) return false;
    std::cout << "[CLIENT] Assigned ID " << _net.getPlayerId() << "\n";
    if (!selectLobby()) {
        std::cerr << "[CLIENT] Failed to select lobby\n";
        return false;
    }
    _net.sendHelloUdp(0, 0);

    _net.pollPackets();
    for (const auto &p : _net.getLastPlayerList())
        _state.upsertPlayer(p.id, p.x, p.y);
    syncEntities(_state.listPlayers());
    _net.clearEvents();
    return true;
}

Entity GraphicClient::createPlayerEntity(float x, float y)
{
    Entity ent = _ecs.createEntity();
    _ecs.addComponent(ent, Position{x, y});
    _ecs.addComponent(ent, Velocity{0, 0});
    auto sprite = std::make_shared<Rtype::Graphic::AnimatedSprite>(
        "../../../sprites/r-typesheet42.gif", Vector2{33, 17}, Vector2{0, 0}, 4, 0.15f, Vector2{x, y}
    );
    _ecs.addComponent(ent, Sprite{sprite});
    return ent;
}

Entity GraphicClient::createBulletEntity(float x, float y, float vx, float vy)
{
    Entity ent = _ecs.createEntity();
    _ecs.addComponent(ent, Position{x, y});
    _ecs.addComponent(ent, Velocity{vx, vy});
    _ecs.addComponent(ent, RectangleComponent{6, 6, BLACK});
    return ent;
}

Entity GraphicClient::createMonsterEntity(float x, float y, uint8_t type)
{
    Entity ent = _ecs.createEntity();
    _ecs.addComponent(ent, Position{x, y});
    _ecs.addComponent(ent, Velocity{0, 0});
    Color c = (type == 1) ? BLUE : RED;
    _ecs.addComponent(ent, RectangleComponent{18, 18, c});
    return ent;
}

void GraphicClient::syncEntities(const std::vector<PlayerState> &players)
{
    int myId = _net.getPlayerId();

    std::unordered_set<int> liveIds;
    for (const auto &p : players) {
        liveIds.insert(p.id);
        if (_entities.size() >= 4 && _entities.find(p.id) == _entities.end())
            continue;

        float clientX = static_cast<float>(p.x);
        float clientY = static_cast<float>(p.y);

        auto it = _entities.find(p.id);
        if (it == _entities.end()) {
            Entity ent = createPlayerEntity(clientX, clientY);
            _entities[p.id] = ent;
        } else {
            auto &pos = _ecs.getComponent<Position>(it->second);
            auto &vel = _ecs.getComponent<Velocity>(it->second);

            // On suit les positions serveur mais en lissant pour éviter l'effet rollback
            float dx = clientX - pos.x;
            float dy = clientY - pos.y;
            float dist2 = dx * dx + dy * dy;

            if (p.id == myId) {
                // On suit directement le serveur pour soi-même pour éviter l'avance locale puis rollback
                pos.x = clientX;
                pos.y = clientY;
                vel.vx = 0.0f;
                vel.vy = 0.0f;
            } else {
                // Pour les autres joueurs, on lisse toujours (et on remet les vitesses à zéro)
                const float smooth = 0.25f;
                if (dist2 > 100.0f) { // snap si trop loin (~10 unités)
                    pos.x = clientX;
                    pos.y = clientY;
                } else {
                    pos.x += dx * smooth;
                    pos.y += dy * smooth;
                }
                vel.vx = 0.0f;
                vel.vy = 0.0f;
            }
        }
    }

    // Remove entities that are no longer in the snapshot/player list
    for (auto it = _entities.begin(); it != _entities.end(); ) {
        if (liveIds.find(it->first) == liveIds.end()) {
            it = _entities.erase(it);
        } else {
            ++it;
        }
    }
}

// ... (syncBullets inchangé) ...
void GraphicClient::syncBullets(const std::vector<BulletState> &bullets)
{
    std::unordered_set<int> liveIds;
    for (const auto &b : bullets) {
        liveIds.insert(b.id);
        float clientX = static_cast<float>(b.x);
        float clientY = static_cast<float>(b.y);
        float serverVx = static_cast<float>(b.vx);
        float serverVy = static_cast<float>(b.vy);
        auto it = _bulletEntities.find(b.id);
        if (it == _bulletEntities.end()) {
            Entity ent = createBulletEntity(clientX, clientY, serverVx, serverVy);
            _bulletEntities[b.id] = ent;
        } else {
            auto &pos = _ecs.getComponent<Position>(it->second);
            auto &vel = _ecs.getComponent<Velocity>(it->second);

            vel.vx = serverVx;
            vel.vy = serverVy;
        }
    }
    for (auto &kv : _bulletEntities) {
        if (liveIds.find(kv.first) == liveIds.end()) {
            auto &pos = _ecs.getComponent<Position>(kv.second);
            auto &vel = _ecs.getComponent<Velocity>(kv.second);
            pos.x = -1000.0f;
            pos.y = -1000.0f;
            vel.vx = 0.0f;
            vel.vy = 0.0f;
        }
    }
}

void GraphicClient::syncMonsters(const std::vector<MonsterState> &monsters)
{
    std::unordered_set<int> liveIds;
    for (const auto &m : monsters) {
        liveIds.insert(m.id);
        float clientX = static_cast<float>(m.x);
        float clientY = static_cast<float>(m.y);
        auto it = _monsterEntities.find(m.id);
        if (it == _monsterEntities.end()) {
            Entity ent = createMonsterEntity(clientX, clientY, m.type);
            _monsterEntities[m.id] = ent;
        } else {
            auto &pos = _ecs.getComponent<Position>(it->second);
            auto &rect = _ecs.getComponent<RectangleComponent>(it->second);
            const float smoothing = 0.25f;
            pos.x += (clientX - pos.x) * smoothing;
            pos.y += (clientY - pos.y) * smoothing;
            rect.color = (m.type == 1) ? BLUE : RED;
        }
    }
    for (auto &kv : _monsterEntities) {
        if (liveIds.find(kv.first) == liveIds.end()) {
            auto &pos = _ecs.getComponent<Position>(kv.second);
            pos.x = -1000.0f;
            pos.y = -1000.0f;
        }
    }
}

void GraphicClient::processNetworkEvents()
{
    _net.pollPackets();
    for (const auto &ev : _net.getEvents()) {
        if (ev == "PLAYER_LIST" || ev == "NEW_PLAYER") {
            _state.clearPlayers();
            for (const auto &p : _net.getLastPlayerList())
                _state.upsertPlayer(p.id, p.x, p.y);
        } else if (ev == "SNAPSHOT") {
            _state.clear();
            for (const auto &p : _net.getLastSnapshot())
                _state.upsertPlayer(p.id, p.x, p.y);
            for (const auto &m : _net.getLastSnapshotMonsters())
                _state.upsertMonster(m.id, m.x, m.y, m.hp, m.type);
        }
    }
    _net.clearEvents();
}
void GraphicClient::updateEntities(float dt)
{
    syncEntities(_state.listPlayers());
    syncBullets(_net.getLastSnapshotBullets());
    syncMonsters(_net.getLastSnapshotMonsters());

    int myId = _net.getPlayerId();

    if (_entities.find(myId) != _entities.end()) {
        Entity myEntity = _entities[myId];
        const auto &myPos = _ecs.getComponent<Position>(myEntity);
        auto &myVel = _ecs.getComponent<Velocity>(myEntity);

        _inputSystem.update(_net, myPos, myVel); 
    }

    for (const auto &kv : _entities) {
        if (kv.first == myId)
            continue; // On ne réintègre pas localement son propre joueur pour éviter les rollbacks visibles
        _movementSystem.update(_ecs, kv.second, dt);
    }
    for (const auto &kv : _bulletEntities) {
        _movementSystem.update(_ecs, kv.second, dt);
    }

    if (_entities.find(myId) != _entities.end()) {
        Entity myEntity = _entities[myId];
        auto &pos = _ecs.getComponent<Position>(myEntity);

        if (pos.x < 0) pos.x = 0;
        if (pos.y < 0) pos.y = 0;
        if (pos.x > 255) pos.x = 255;
        if (pos.y > 255) pos.y = 255;
    }

    // Plus de gestion de PV joueur ni fermeture auto ici
}

bool GraphicClient::selectLobby()
{
    // Build UI entities
    _uiEcs = ECS(); // reset
    std::vector<Entity> buttons;

    auto makeButton = [&](float x, float y, float w, float h, const std::string &label, const std::string &action) {
        Entity e = _uiEcs.createEntity();
        _uiEcs.addComponent(e, UIButton{Rectangle{x, y, w, h}, label, action});
        buttons.push_back(e);
    };
    makeButton(200, 200, 360, 60, "Auto - Public Lobby", "auto");
    makeButton(200, 280, 360, 60, "Create Private Lobby", "create");
    makeButton(200, 360, 360, 60, "Join with Code", "join");

    std::string codeInput;
    bool enteringCode = false;
    bool submitted = false;
    std::string action = "auto";
    bool hasLobbyOk = false;
    std::string lobbyError;

    auto start = std::chrono::steady_clock::now();

    auto processEvents = [&](bool checkImmediate) {
        bool ok = false;
        for (const auto &ev : _net.getEvents()) {
            if (ev.rfind("LOBBY_OK:", 0) == 0) {
                hasLobbyOk = true;
                ok = true;
            }
            if (ev.rfind("LOBBY_ERROR:", 0) == 0) {
                lobbyError = ev.substr(std::string("LOBBY_ERROR:").size());
            }
            // Fallback: if we receive PLAYER_LIST before LOBBY_OK, assume lobby joined
            if (ev == "PLAYER_LIST" || ev == "NEW_PLAYER") {
                hasLobbyOk = true;
                ok = true;
            }
        }
        _net.clearEvents();
        if (checkImmediate && ok)
            return true;
        return false;
    };

    while (!submitted) {
        // Poll to reply to PING during menu
        _net.pollPackets();
        // Process once to catch LOBBY_OK quickly
        if (processEvents(true)) {
            return true;
        }
        // Proactive keepalive to avoid server timeout when idling in the menu
        auto nowKeep = std::chrono::steady_clock::now();
        if (std::chrono::duration_cast<std::chrono::seconds>(nowKeep - _lastKeepAlive).count() >= 4) {
            _net.sendPong();
            _lastKeepAlive = nowKeep;
        }

        _window.beginDrawing();
        _window.clearBackground({20, 20, 40, 255});
        Raylib::Draw::text("Lobby Selection", 200, 140, 32, RAYWHITE);

        Vector2 mouse{(float)GetMouseX(), (float)GetMouseY()};
        bool click = IsMouseButtonPressed(MOUSE_BUTTON_LEFT);

        for (auto e : buttons) {
            auto &btn = _uiEcs.getComponent<UIButton>(e);
            btn.hovered = CheckCollisionPointRec(mouse, btn.bounds);
            Color bg = btn.hovered ? btn.hoverBg : btn.bg;
            Raylib::Draw::rectangle(static_cast<int>(btn.bounds.x), static_cast<int>(btn.bounds.y),
                                    static_cast<int>(btn.bounds.width), static_cast<int>(btn.bounds.height), bg);
            int textW = MeasureText(btn.label.c_str(), 22);
            Raylib::Draw::text(btn.label.c_str(), btn.bounds.x + (btn.bounds.width - textW) / 2, btn.bounds.y + 18, 22, btn.text);
            if (btn.hovered && click) {
                action = btn.action;
                if (action == "join") {
                    enteringCode = true;
                } else {
                    submitted = true;
                }
            }
        }

        if (enteringCode) {
            // text input area
            Rectangle inputBox{200, 440, 360, 60};
            Raylib::Draw::rectangle(static_cast<int>(inputBox.x), static_cast<int>(inputBox.y),
                                    static_cast<int>(inputBox.width), static_cast<int>(inputBox.height),
                                    {30, 30, 60, 255});
            Raylib::Draw::text("Enter code:", inputBox.x + 10, inputBox.y - 26, 20, RAYWHITE);

            int key = GetCharPressed();
            while (key > 0) {
                if (std::isalnum(key) && codeInput.size() < 8) {
                    codeInput.push_back(static_cast<char>(std::toupper(key)));
                }
                key = GetCharPressed();
            }
            if (Raylib::Input::isKeyPressed(KEY_BACKSPACE) && !codeInput.empty())
                codeInput.pop_back();

            Raylib::Draw::text(codeInput.c_str(), inputBox.x + 14, inputBox.y + 18, 26, RAYWHITE);
            if (!codeInput.empty()) {
                Raylib::Draw::text("Press Enter to join", inputBox.x, inputBox.y + 70, 18, LIGHTGRAY);
            }
            if (Raylib::Input::isKeyPressed(KEY_ENTER) && !codeInput.empty()) {
                submitted = true;
            }
        }

        _window.endDrawing();

        if (submitted) {
            if (action == "create") {
                if (!_net.sendCreateLobby()) {
                    std::cerr << "[CLIENT] Failed to send CREATE_LOBBY\n";
                    return false;
                }
            } else if (action == "join") {
                if (codeInput.empty()) {
                    submitted = false;
                    enteringCode = true;
                } else if (!_net.sendJoinLobby(codeInput)) {
                    std::cerr << "[CLIENT] Failed to send JOIN_LOBBY\n";
                    return false;
                }
            }
            // else {
            //     // auto public: demander explicitement le lobby public
            //     if (!_net.sendJoinLobby("PUBLIC")) {
            //         std::cerr << "[CLIENT] Failed to send AUTO PUBLIC\n";
            //         return false;
            //     }
            // }
            break;
        }

        auto now = std::chrono::steady_clock::now();
        if (std::chrono::duration_cast<std::chrono::seconds>(now - start).count() > 30) {
            std::cerr << "[CLIENT] Lobby selection timeout\n";
            return false;
        }
    }

    // Wait for LOBBY_OK/ERROR
    auto waitStart = std::chrono::steady_clock::now();
    while (true) {
        _net.pollPackets();
        if (processEvents(true)) {
            std::cout << "[CLIENT] Joined lobby " << _net.getLobbyCode() << "\n";
            return true;
        }
        if (!lobbyError.empty()) {
            std::cerr << "[CLIENT] Lobby error: " << lobbyError << "\n";
            return false;
        }
        // auto now = std::chrono::steady_clock::now();
        // if (std::chrono::duration_cast<std::chrono::seconds>(now - waitStart).count() > 10) {
        //     std::cerr << "[CLIENT] Lobby selection timeout (server), forcing PUBLIC\n";
        //     // Fallback: assume public and continue to avoid closing the window
        //     return true;
        // }
    }
}

void GraphicClient::render(float dt)
{
    _window.beginDrawing();
    _window.clearBackground(RAYWHITE);

    Raylib::Draw::rectangleLines(0, 0, 255 * 5, 255 * 5, RED);

    _spriteRenderSystem.setScale(2.0f, 2.0f);

    for (const auto &kv : _entities) {
        _spriteRenderSystem.update(_ecs, kv.second, dt);
    }
    for (const auto &kv : _bulletEntities) {
        _rectangleRenderSystem.update(_ecs, kv.second);
    }
    for (const auto &kv : _monsterEntities) {
        _rectangleRenderSystem.update(_ecs, kv.second);
    }

    _window.endDrawing();
}

void GraphicClient::gameLoop()
{
    while (!_window.shouldClose()) {
        float dt = _window.getFrameTime();
        processNetworkEvents();
        // If PING send failed, drop to avoid server timeout
        for (const auto &ev : _net.getEvents()) {
            if (ev == "PING_SEND_FAIL") {
                std::cerr << "[CLIENT] Failed to send PONG, disconnecting\n";
                _net.disconnect();
                return;
            }
        }
        _net.clearEvents();

        // Proactive keepalive to avoid timeout when idle in game
        auto nowKeep = std::chrono::steady_clock::now();
        if (std::chrono::duration_cast<std::chrono::seconds>(nowKeep - _lastKeepAlive).count() >= 4) {
            _net.sendPong();
            _lastKeepAlive = nowKeep;
        }

        updateEntities(dt);
        render(dt);
    }
}

int GraphicClient::run()
{
    if (!init()) return 1;
    gameLoop();
    return 0;
}
