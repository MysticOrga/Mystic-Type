/*
** EPITECH PROJECT, 2025
** Mystic-Type
** File description:
** GraphicClient
*/

#include "graphical-client/GraphicClient/GraphicClient.hpp"
#include "graphical-client/ecs/Components.hpp"
#include <algorithm>
#include <cctype>
#include <cmath>
#include <iostream>
#include <limits>
#include <memory>
#include <unordered_set>

GraphicClient::GraphicClient(const std::string &ip, int port) : _window(1920, 1080, "Mystic-Type"), _net(ip, port)
{
    _window.setTargetFPS(60);
    _lastKeepAlive = std::chrono::steady_clock::now();
    _lastHello = _lastKeepAlive;
}

bool GraphicClient::init()
{
    _spriteRenderSystem.setGameAreaOffset(GAME_AREA_OFFSET_X, GAME_AREA_OFFSET_Y, GAME_AREA_SIZE);
    _rectangleRenderSystem.setGameAreaOffset(GAME_AREA_OFFSET_X, GAME_AREA_OFFSET_Y, GAME_AREA_SIZE);

    if (!_net.connectToServer())
        return false;
    if (!_net.performHandshake())
        return false;
    std::cout << "[CLIENT] Assigned ID " << _net.getPlayerId() << "\n";
    if (!selectLobby())
    {
        std::cerr << "[CLIENT] Failed to select lobby\n";
        return false;
    }
    _net.sendHelloUdp(0, 0);
    _lastHello = std::chrono::steady_clock::now();

    _net.pollPackets();
    for (const auto &p : _net.getLastPlayerList())
        _state.upsertPlayer(p.id, p.x, p.y, p.hp);
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
        "../../../sprites/r-typesheet42.gif", Vector2{33, 17}, Vector2{0, 0}, 4, 0.15f, Vector2{x, y});
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
    
    // Create animated sprite for monster
    // Spritesheet: 205x18, 17 sprites, so each sprite is ~12x18
    // Type 1 = Blue (row 0), Type 2 = Red (row 1)
    Vector2 spriteSize{17, 18};
    Vector2 posInSheet{0, static_cast<float>(type - 1)}; // Row based on type
    auto sprite = std::make_shared<Rtype::Graphic::AnimatedSprite>(
        "../../../sprites/r-typesheet3.gif", 
        spriteSize, 
        posInSheet, 
        12,  // 17 sprites in the sheet
        0.15f,  // Frame time
        Vector2{x, y}
    );
    _ecs.addComponent(ent, Sprite{sprite});
    
    return ent;
}

void GraphicClient::syncEntities(const std::vector<PlayerState> &players)
{
    int myId = _net.getPlayerId();

    std::unordered_set<int> liveIds;
    for (const auto &p : players)
    {
        liveIds.insert(p.id);
        if (_entities.size() >= 4 && _entities.find(p.id) == _entities.end())
            continue;

        float clientX = static_cast<float>(p.x);
        float clientY = static_cast<float>(p.y);

        auto it = _entities.find(p.id);
        if (it == _entities.end())
        {
            Entity ent = createPlayerEntity(clientX, clientY);
            _entities[p.id] = ent;
        }
        else
        {
            auto &pos = _ecs.getComponent<Position>(it->second);
            auto &vel = _ecs.getComponent<Velocity>(it->second);

            float dx = clientX - pos.x;
            float dy = clientY - pos.y;
            float dist2 = dx * dx + dy * dy;

            if (p.id == myId)
            {
                pos.x = clientX;
                pos.y = clientY;
                vel.vx = 0.0f;
                vel.vy = 0.0f;
            }
            else
            {
                const float smooth = 0.25f;
                if (dist2 > 100.0f)
                {
                    pos.x = clientX;
                    pos.y = clientY;
                }
                else
                {
                    pos.x += dx * smooth;
                    pos.y += dy * smooth;
                }
                vel.vx = 0.0f;
                vel.vy = 0.0f;
            }
        }
    }

    // Remove entities that are no longer in the snapshot/player list
    for (auto it = _entities.begin(); it != _entities.end();)
    {
        if (liveIds.find(it->first) == liveIds.end())
        {
            it = _entities.erase(it);
        }
        else
        {
            ++it;
        }
    }
}

// ... (syncBullets inchangé) ...
void GraphicClient::syncBullets(const std::vector<BulletState> &bullets)
{
    std::unordered_set<int> liveIds;
    for (const auto &b : bullets)
    {
        liveIds.insert(b.id);
        float clientX = static_cast<float>(b.x);
        float clientY = static_cast<float>(b.y);
        float serverVx = static_cast<float>(b.vx);
        float serverVy = static_cast<float>(b.vy);
        auto it = _bulletEntities.find(b.id);
        if (it == _bulletEntities.end())
        {
            Entity ent = createBulletEntity(clientX, clientY, serverVx, serverVy);
            _bulletEntities[b.id] = ent;
        }
        else
        {
            auto &pos = _ecs.getComponent<Position>(it->second);
            auto &vel = _ecs.getComponent<Velocity>(it->second);

            vel.vx = serverVx;
            vel.vy = serverVy;
        }
    }
    for (auto &kv : _bulletEntities)
    {
        if (liveIds.find(kv.first) == liveIds.end())
        {
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
    for (const auto &m : monsters)
    {
        liveIds.insert(m.id);
        float clientX = static_cast<float>(m.x);
        float clientY = static_cast<float>(m.y);
        auto it = _monsterEntities.find(m.id);
        if (it == _monsterEntities.end())
        {
            Entity ent = createMonsterEntity(clientX, clientY, m.type);
            _monsterEntities[m.id] = ent;
        }
        else
        {
            auto &pos = _ecs.getComponent<Position>(it->second);
            const float smoothing = 0.25f;
            pos.x += (clientX - pos.x) * smoothing;
            pos.y += (clientY - pos.y) * smoothing;
        }
    }
    for (auto &kv : _monsterEntities)
    {
        if (liveIds.find(kv.first) == liveIds.end())
        {
            auto &pos = _ecs.getComponent<Position>(kv.second);
            pos.x = -1000.0f;
            pos.y = -1000.0f;
        }
    }
}

void GraphicClient::processNetworkEvents()
{
    _net.pollPackets();
    for (const auto &ev : _net.getEvents())
    {
        if (ev == "PLAYER_LIST" || ev == "NEW_PLAYER")
        {
            _state.clearPlayers();
            for (const auto &p : _net.getLastPlayerList())
                _state.upsertPlayer(p.id, p.x, p.y, p.hp);
        }
        else if (ev == "SNAPSHOT")
        {
            _udpReady = true;
            _state.clear();
            for (const auto &p : _net.getLastSnapshot())
                _state.upsertPlayer(p.id, p.x, p.y, p.hp);
            for (const auto &m : _net.getLastSnapshotMonsters())
                _state.upsertMonster(m.id, m.x, m.y, m.hp, m.type);
            int myId = _net.getPlayerId();
            bool foundMe = false;
            for (const auto &p : _state.listPlayers())
            {
                if (p.id == myId)
                {
                    foundMe = true;
                    if (p.hp == 0)
                    {
                        std::cerr << "[CLIENT] Vous etes mort\n";
                        _net.disconnect();
                        _window.close();
                        _forceExit = true;
                    }
                    break;
                }
            }
            if (!foundMe)
            {
                std::cerr << "[CLIENT] Vous etes mort\n";
                _net.disconnect();
                _window.close();
                _forceExit = true;
            }
        }
        else if (ev.rfind("MESSAGE:", 0) == 0)
        {
            std::string msg = ev.substr(std::string("MESSAGE:").size());
            if (msg == "DEAD")
            {
                std::cerr << "[CLIENT] Vous etes mort\n";
                _net.disconnect();
                _window.close();
                _forceExit = true;
            }
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

    if (_entities.find(myId) != _entities.end())
    {
        Entity myEntity = _entities[myId];
        const auto &myPos = _ecs.getComponent<Position>(myEntity);
        auto &myVel = _ecs.getComponent<Velocity>(myEntity);

        _inputSystem.update(_net, myPos, myVel);
    }

    for (const auto &kv : _entities)
    {
        if (kv.first == myId)
            continue;
        _movementSystem.update(_ecs, kv.second, dt);
    }
    for (const auto &kv : _bulletEntities)
    {
        _movementSystem.update(_ecs, kv.second, dt);
    }

    if (_entities.find(myId) != _entities.end())
    {
        Entity myEntity = _entities[myId];
        auto &pos = _ecs.getComponent<Position>(myEntity);

        if (pos.x < 0)
            pos.x = 0;
        if (pos.y < 0)
            pos.y = 0;
        if (pos.x > 255)
            pos.x = 255;
        if (pos.y > 255)
            pos.y = 255;
    }
}

bool GraphicClient::selectLobby()
{
    _uiEcs = ECS();
    std::vector<Entity> buttons;

    const float buttonWidth = 280.0f;
    const float buttonHeight = 70.0f;
    const float screenWidth = 1920.0f;
    const float screenHeight = 1080.0f;
    const float centerX = (screenWidth - buttonWidth) / 2.0f;
    const float startY = 450.0f;
    const float spacing = 120.0f;

    auto makeButton = [&](float x, float y, float w, float h, const std::string &label, const std::string &action) {
        Entity e = _uiEcs.createEntity();
        _uiEcs.addComponent(e, UIButton{Rectangle{x, y, w, h}, label, action});
        buttons.push_back(e);
    };

    makeButton(centerX, startY, buttonWidth, buttonHeight, "Auto - Public Lobby", "auto");
    makeButton(centerX, startY + spacing, buttonWidth, buttonHeight, "Create Private Lobby", "create");
    makeButton(centerX, startY + spacing * 2, buttonWidth, buttonHeight, "Join with Code", "join");

    std::string codeInput;
    bool enteringCode = false;
    bool submitted = false;
    std::string action = "auto";
    bool hasLobbyOk = false;
    std::string lobbyError;
    float hoverAnimTimer = 0.0f;

    auto start = std::chrono::steady_clock::now();

    auto processEvents = [&](bool checkImmediate) {
        bool ok = false;
        for (const auto &ev : _net.getEvents())
        {
            if (ev.rfind("LOBBY_OK:", 0) == 0)
            {
                hasLobbyOk = true;
                ok = true;
            }
            if (ev.rfind("LOBBY_ERROR:", 0) == 0)
            {
                lobbyError = ev.substr(std::string("LOBBY_ERROR:").size());
            }
            // Fallback: if we receive PLAYER_LIST before LOBBY_OK, assume lobby joined
            if (ev == "PLAYER_LIST" || ev == "NEW_PLAYER")
            {
                hasLobbyOk = true;
                ok = true;
            }
        }
        _net.clearEvents();
        if (checkImmediate && ok)
            return true;
        return false;
    };

    while (!submitted)
    {
        float dt = _window.getFrameTime();
        hoverAnimTimer += dt;

        _net.pollPackets();
        if (processEvents(true))
        {
            return true;
        }
        auto nowKeep = std::chrono::steady_clock::now();
        if (std::chrono::duration_cast<std::chrono::seconds>(nowKeep - _lastKeepAlive).count() >= 4)
        {
            _net.sendPong();
            _lastKeepAlive = nowKeep;
        }

        _window.beginDrawing();

        _window.clearBackground({15, 25, 50, 255});

        const char *title = "MYSTIC TYPE";
        int titleSize = 60;
        int titleWidth = MeasureText(title, titleSize);
        float titleX = (screenWidth - titleWidth) / 2.0f;

        for (int i = 3; i > 0; i--)
        {
            Color glowColor = {100, 150, 255, static_cast<unsigned char>(50 - i * 10)};
            Raylib::Draw::text(title, static_cast<int>(titleX - i), 100 - i, titleSize, glowColor);
            Raylib::Draw::text(title, static_cast<int>(titleX + i), 100 + i, titleSize, glowColor);
        }
        Raylib::Draw::text(title, static_cast<int>(titleX), 100, titleSize, {100, 200, 255, 255});

        const char *subtitle = "LOBBY SELECTION";
        int subtitleWidth = MeasureText(subtitle, 20);
        Raylib::Draw::text(subtitle, static_cast<int>((screenWidth - subtitleWidth) / 2.0f), 170, 20,
                           {150, 200, 255, 200});

        Vector2 mouse{(float)GetMouseX(), (float)GetMouseY()};
        bool click = IsMouseButtonPressed(MOUSE_BUTTON_LEFT);

        for (size_t idx = 0; idx < buttons.size(); idx++)
        {
            auto e = buttons[idx];
            auto &btn = _uiEcs.getComponent<UIButton>(e);
            btn.hovered = CheckCollisionPointRec(mouse, btn.bounds);

            Color borderColor = btn.hovered ? Color{100, 200, 255, 200} : Color{50, 150, 200, 150};
            Color bgColor = btn.hovered ? Color{30, 80, 150, 220} : Color{20, 50, 100, 180};

            Raylib::Draw::rectangleLines(static_cast<int>(btn.bounds.x) - 2, static_cast<int>(btn.bounds.y) - 2,
                                         static_cast<int>(btn.bounds.width) + 4,
                                         static_cast<int>(btn.bounds.height) + 4, borderColor);

            Raylib::Draw::rectangle(static_cast<int>(btn.bounds.x), static_cast<int>(btn.bounds.y),
                                    static_cast<int>(btn.bounds.width), static_cast<int>(btn.bounds.height), bgColor);

            if (btn.hovered)
            {
                Raylib::Draw::rectangleLines(static_cast<int>(btn.bounds.x) + 1, static_cast<int>(btn.bounds.y) + 1,
                                             static_cast<int>(btn.bounds.width) - 2,
                                             static_cast<int>(btn.bounds.height) - 2, Color{150, 220, 255, 100});

                float phase = std::sin(hoverAnimTimer * 3.0f) * 2.0f + 3.0f;
                Raylib::Draw::rectangleLines(
                    static_cast<int>(btn.bounds.x) - static_cast<int>(phase),
                    static_cast<int>(btn.bounds.y) - static_cast<int>(phase),
                    static_cast<int>(btn.bounds.width) + static_cast<int>(phase) * 2,
                    static_cast<int>(btn.bounds.height) + static_cast<int>(phase) * 2,
                    Color{100, 200, 255, static_cast<unsigned char>(100 + 55.0f * std::sin(hoverAnimTimer * 4.0f))});
            }

            int textW = MeasureText(btn.label.c_str(), 24);
            Color textColor = btn.hovered ? Color{255, 255, 255, 255} : Color{180, 220, 255, 220};
            Raylib::Draw::text(btn.label.c_str(), static_cast<int>(btn.bounds.x + (btn.bounds.width - textW) / 2.0f),
                               static_cast<int>(btn.bounds.y + (btn.bounds.height - 24) / 2.0f), 24, textColor);

            if (btn.hovered && click)
            {
                action = btn.action;
                if (action == "join")
                {
                    enteringCode = true;
                }
                else
                {
                    submitted = true;
                }
            }
        }

        if (enteringCode)
        {
            const float inputWidth = 300.0f;
            const float inputHeight = 70.0f;
            const float inputX = (screenWidth - inputWidth) / 2.0f;
            const float inputY = startY + spacing * 3 + 40.0f;
            Rectangle inputBox{inputX, inputY, inputWidth, inputHeight};

            Raylib::Draw::rectangleLines(static_cast<int>(inputBox.x) - 2, static_cast<int>(inputBox.y) - 2,
                                         static_cast<int>(inputBox.width) + 4, static_cast<int>(inputBox.height) + 4,
                                         Color{100, 200, 255, 150});

            Raylib::Draw::rectangle(static_cast<int>(inputBox.x), static_cast<int>(inputBox.y),
                                    static_cast<int>(inputBox.width), static_cast<int>(inputBox.height),
                                    Color{15, 40, 80, 200});

            Raylib::Draw::text("Enter code:", static_cast<int>(inputX + 10), static_cast<int>(inputY - 30), 18,
                               {150, 200, 255, 255});

            int key = GetCharPressed();
            while (key > 0)
            {
                if (std::isalnum(key) && codeInput.size() < 8)
                {
                    codeInput.push_back(static_cast<char>(std::toupper(key)));
                }
                key = GetCharPressed();
            }
            if (Raylib::Input::isKeyPressed(KEY_BACKSPACE) && !codeInput.empty())
                codeInput.pop_back();

            Raylib::Draw::text(codeInput.c_str(), static_cast<int>(inputX + 14), static_cast<int>(inputY + 18), 32,
                               {200, 255, 255, 255});
            if (!codeInput.empty())
            {
                Raylib::Draw::text("Press Enter to join", static_cast<int>(inputX), static_cast<int>(inputY + 80), 16,
                                   {150, 200, 255, 200});
            }
            if (Raylib::Input::isKeyPressed(KEY_ENTER) && !codeInput.empty())
            {
                submitted = true;
            }
        }

        _window.endDrawing();

        if (submitted)
        {
            if (action == "create")
            {
                if (!_net.sendCreateLobby())
                {
                    std::cerr << "[CLIENT] Failed to send CREATE_LOBBY\n";
                    return false;
                }
            }
            else if (action == "join")
            {
                if (codeInput.empty())
                {
                    submitted = false;
                    enteringCode = true;
                }
                else if (!_net.sendJoinLobby(codeInput))
                {
                    std::cerr << "[CLIENT] Failed to send JOIN_LOBBY\n";
                    return false;
                }
            }
            else
            {
                if (!_net.sendJoinLobby("PUBLIC"))
                {
                    std::cerr << "[CLIENT] Failed to send AUTO PUBLIC\n";
                    return false;
                }
            }
            break;
        }

        auto now = std::chrono::steady_clock::now();
        if (std::chrono::duration_cast<std::chrono::seconds>(now - start).count() > 30)
        {
            std::cerr << "[CLIENT] Lobby selection timeout\n";
            return false;
        }
    }

    // Wait for LOBBY_OK/ERROR
    auto waitStart = std::chrono::steady_clock::now();
    while (true)
    {
        _net.pollPackets();
        if (processEvents(true))
        {
            std::cout << "[CLIENT] Joined lobby " << _net.getLobbyCode() << "\n";
            return true;
        }
        if (!lobbyError.empty())
        {
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

void GraphicClient::drawGameBackground(float hoverAnimTimer)
{
    _window.clearBackground({15, 25, 50, 255});

    float x = GAME_AREA_OFFSET_X;
    float y = GAME_AREA_OFFSET_Y;
    float size = GAME_AREA_SIZE;

    Color glowColor = {100, 200, 255, 150};
    Raylib::Draw::rectangleLines(static_cast<int>(x) - 3, static_cast<int>(y) - 3, static_cast<int>(size) + 6,
                                 static_cast<int>(size) + 6, glowColor);

    Color borderColor = {100, 200, 255, 220};
    Raylib::Draw::rectangleLines(static_cast<int>(x), static_cast<int>(y), static_cast<int>(size),
                                 static_cast<int>(size), borderColor);

    Raylib::Draw::rectangleLines(static_cast<int>(x) + 2, static_cast<int>(y) + 2, static_cast<int>(size) - 4,
                                 static_cast<int>(size) - 4, Color{150, 220, 255, 100});
}

void GraphicClient::render(float dt)
{
    _gameAnimTimer += dt;

    _window.beginDrawing();

    drawGameBackground(_gameAnimTimer);

    uint8_t myHp = 0;
    bool hasHp = false;
    int myId = _net.getPlayerId();
    for (const auto &p : _state.listPlayers())
    {
        if (p.id == myId)
        {
            myHp = p.hp;
            hasHp = true;
            break;
        }
    }
    std::string hpText = hasHp ? ("HP: " + std::to_string(myHp)) : "HP: --";
    Raylib::Draw::text(hpText, static_cast<int>(GAME_AREA_OFFSET_X) + 12,
                       static_cast<int>(GAME_AREA_OFFSET_Y) + 12, 24, {255, 255, 255, 230});

    _spriteRenderSystem.setScale(1.0f, 1.0f);
    for (const auto &kv : _entities)
    {
        _spriteRenderSystem.update(_ecs, kv.second, dt);
    }

    for (const auto &kv : _bulletEntities)
    {
        _rectangleRenderSystem.update(_ecs, kv.second);
    }

    for (const auto &kv : _monsterEntities)
    {
        _spriteRenderSystem.update(_ecs, kv.second, dt);
    }

    _window.endDrawing();
}

void GraphicClient::gameLoop()
{
    while (!_window.shouldClose())
    {
        float dt = _window.getFrameTime();
        processNetworkEvents();
        if (_forceExit)
            return;
        // If PING send failed, drop to avoid server timeout
        for (const auto &ev : _net.getEvents())
        {
            if (ev == "PING_SEND_FAIL")
            {
                std::cerr << "[CLIENT] Failed to send PONG, disconnecting\n";
                _net.disconnect();
                return;
            }
        }
        _net.clearEvents();

        // Proactive keepalive to avoid timeout when idle in game
        auto nowKeep = std::chrono::steady_clock::now();
        if (std::chrono::duration_cast<std::chrono::seconds>(nowKeep - _lastKeepAlive).count() >= 4)
        {
            _net.sendPong();
            _lastKeepAlive = nowKeep;
        }

        if (!_udpReady)
        {
            auto nowHello = std::chrono::steady_clock::now();
            if (std::chrono::duration_cast<std::chrono::seconds>(nowHello - _lastHello).count() >= 1)
            {
                _net.sendHelloUdp(0, 0);
                _lastHello = nowHello;
            }
        }

        updateEntities(dt);
        render(dt);
    }
}

int GraphicClient::run()
{
    if (!init())
        return 1;
    gameLoop();
    return 0;
}
