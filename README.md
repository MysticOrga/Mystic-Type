# Mystic-Type

Multiplayer R-Type-like with an authoritative TCP/UDP server and a raylib graphical client.

## Prerequisites
- CMake ≥ 3.14
- C++17 compiler
- raylib (fetched automatically via FetchContent)
- Works on macOS, Linux, and Windows (WSA init handled in entry points).

## Build

### Linux
```bash
./build.sh              # build client + servers
```
Executables are in `build/linux/`:
- `rtype-tcp-server`
- `rtype-udp-server`
- `rtype-client`

### macOS
```bash
./build.sh
```
Executables are in `build/linux/`:
- `rtype-tcp-server`
- `rtype-udp-server`
- `rtype-client`

### Windows
```bash
cmake -S . -B build -G "Visual Studio 17 2022"
```
Executables are in `build/Release/`:
- `rtype-tcp-server.exe`
- `rtype-udp-server.exe`
- `rtype-client.exe`

## Run

### Linux
```bash
cd ./build/linux/
./rtype-tcp-server
./rtype-client
```

### macOS
```bash
cd ./build/linux/
./rtype-tcp-server
./rtype-client
```

### Windows
```bash
cd .\build/Release
.\rtype-tcp-server.exe
.\rtype-client.exe
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
- [Doxygen](https://mysticorga.github.io/Mystic-Type/doxygen/html/index.html)
- Protocol RFCs:

    - [mtp protocol](https://mysticorga.github.io/Mystic-Type/rfc/mtp_protocol_rfc.txt)

    - [tcp protocol](https://mysticorga.github.io/Mystic-Type/rfc/tcp_protocol_rfc.txt)

## Authors
- Quentin Hivanhoe: `quentin.hivanhoe@epitech.eu`
- Loic Rabearivelo: `loic.rabearivelo@epitech.eu`
- Emma Hoarau: `emma-frederique.hoarau@epitech.eu`
- Nicolas Samy: `nicolas.samy@epitech.eu`

## License

This project is licensed under the MIT License - see below for details.

```
MIT License

Copyright (c) 2026 Mystic-Type Contributors

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
```
