# Mystic-Type

Multiplayer R-Type-like with an authoritative TCP/UDP server and a raylib graphical client.

## Prerequisites
- CMake ≥ 3.14
- C++17 compiler
- raylib (fetched automatically via FetchContent)
- POSIX sockets (Linux/macOS). Windows would need WSA adaptations (not provided).

## Build
```bash
./compile.sh            # build client + server (Release)
./compile.sh server     # build server only
./compile.sh client     # build client only
./compile.sh clean      # remove build/
```
Executables are in `build/src/server/` (`rtype-server`) and `build/src/graphical-client/` (`rtype-client`).

## Run
Server:
```bash
cd build/src/server
./rtype-server
```
Graphical client:
```bash
cd build/src/graphical-client
./rtype-client
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
- Authoritative server, fixed tick simulation, periodic snapshots.
- Rate-limited inputs/shoots server-side.
- TCP heartbeat and session cleanup (GameWorld purge).
- Simple graphical client (raylib) with a minimal ECS.

## Documentation
- Doxygen: `https://mysticorga.github.io/Mystic-Type/`
- Protocol RFCs: `docs/tcp_protocol_rfc.txt`, `docs/mtp_protocol_rfc.txt`

## Authors
- Quentin Hivanhoe
- Loic Rabearivelo
- Emma Hoarau
- Nicolas Samy
