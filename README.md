# Mystic-Type

Multiplayer R-Type-like with an authoritative TCP/UDP server and a raylib graphical client.

## Prerequisites
- CMake ≥ 3.14
- C++17 compiler
- raylib (fetched automatically via FetchContent)
- Works on macOS, Linux, and Windows (WSA init handled in entry points).

## Build
```bash
./build.sh              # build client + servers
```
Executables are in `build/linux/bin/`:
- `rtype-tcp-server`
- `rtype-udp-server`
- `rtype-client`

## Run
Server:
```bash
./build/linux/bin/rtype-tcp-server
```
Graphical client:
```bash
./build/linux/bin/rtype-client
```

## Architecture
- `src/server/`: server entry point, TCP loop (handshake + heartbeat) and UDP loop (simulation).
- `src/graphical-client/`: raylib client + minimal ECS.
- `src/Network/TransportLayer/`:
  - `TCPServer`, `TCPSocket`: listening socket, handshake, PING/PONG.
  - `UDPGameServer`: handles HELLO_UDP/INPUT/SHOOT, simulation tick, SNAPSHOT broadcast.
  - `GameWorld`: authoritative simulation (players, bullets, monsters).
  - `Packet`, `Protocol`: packet format, TCP framing.
- `src/Network/Client/`: `NetworkClient` manages TCP (handshake/heartbeat) and UDP (inputs, snapshots).
- `src/Network/SessionManager`: tracks sessions, rate-limits INPUT/SHOOT, purges game state on disconnect.

## Network protocol (summary)
- TCP (reliable):
  - Handshake: SERVER_HELLO → CLIENT_HELLO("toto…") → OK(id) or REFUSED(reason).
  - Heartbeat: PING/PONG, timeout >10s → disconnect.
  - Lobby: PLAYER_LIST, NEW_PLAYER.
  - Framing: 2-byte length prefix + packet (magic 0x5254, type, size, payload).
- UDP (real-time):
  - HELLO_UDP(id,x,y) to register endpoint.
  - INPUT(id, pos, vel, dir) → server ignores client positions (authority).
  - SHOOT(id, pos, vel).
  - SNAPSHOT periodically: players, bullets, monsters.

## Controls by default (client)
- Move: W/A/S/D
- Shoot: Space

## Key features
- TCP protocol with framing and handshake (SERVER_HELLO/CLIENT_HELLO/OK/REFUSED).
- Heartbeat (PING/PONG) with timeout disconnect and session cleanup.
- Lobby system: auto public, create, join by code; lobby-scoped player list and chat.
- UDP authoritative simulation with fixed tick and periodic snapshots.
- Player input, shooting, bullets, monsters (sine/zigzag), boss logic, and collisions.
- Client UI: lobby/pseudo selection, in-game chat, and return-to-lobby flow.

## Documentation
- [Doxygen](https://mysticorga.github.io/Mystic-Type/)
- Protocol RFCs:

    - [mtp protocol](https://mysticorga.github.io/Mystic-Type/rfc/mtp_protocol_rfc.txt)

    - [tcp protocol](https://mysticorga.github.io/Mystic-Type/rfc/tcp_protocol_rfc.txt)

## Authors
- Quentin Hivanhoe: `quentin.hivanhoe@epitech.eu`
- Loic Rabearivelo: `loic.rabearivelo@epitech.eu`
- Emma Hoarau: `emma-frederique.hoarau@epitech.eu`
- Nicolas Samy: `nicolas.samy@epitech.eu`
