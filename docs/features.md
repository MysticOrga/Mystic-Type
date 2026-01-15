# Mystic-Type - Feature List

## 1) Network / Protocol
- TCP protocol with framing (2-byte length + packet header + payload).
- UDP protocol for realtime game traffic (HELLO_UDP, INPUT, SHOOT, SNAPSHOT).
- Packet validation with magic header and payload size checks.

## 2) TCP Server
- Accepts client connections and performs handshake (SERVER_HELLO -> CLIENT_HELLO -> OK/REFUSED).
- Heartbeat with PING/PONG and timeout disconnect.
- Lobby management: create, join by code, auto public lobby.
- Player list broadcast and NEW_PLAYER notifications.
- Chat relay within lobby (SYS and CHAT messages).
- IPC bridge to UDP server (boss spawn/dead, player dead, no players).

## 3) UDP Game Server
- Authoritative simulation with fixed tick interval.
- Player registration via HELLO_UDP and per-lobby game worlds.
- Player movement update from inputs (server-authoritative).
- Bullet spawning and movement.
- Monster spawning (sine, zigzag) with variable spawn timing.
- Boss spawning based on score, boss movement and boss bullets.
- Collisions: player vs monster/boss, bullet vs monster/boss.
- Score tracking per lobby; boss defeat state.
- Periodic snapshot broadcast to all players in the lobby.

## 4) Client (Graphical)
- Raylib window, 60 FPS target.
- ECS with Position/Velocity/Sprite/Rectangle components.
- Rendering of players, bullets, and monsters.
- Client-side smoothing/interpolation for players and monsters.
- In-game chat UI (input + log).
- Lobby selection UI (auto / create / join by code).
- Pseudo selection UI.
- Return-to-lobby flow on death, boss victory, boss win, or ESC.

## 5) Sessions / State
- Shared SessionManager for TCP/UDP data (id, lobby code, pseudo, limits).
- Rate limit hooks for inputs and shooting (server-side support).
- Per-lobby score tracking and player state cleanup on disconnect.

## 6) Cross-platform / Build
- CMake build with raylib dependency.
- macOS/Linux supported; Windows compiles with IPC/fork disabled.
- build.sh adapted for macOS (no nproc).
