#!/usr/bin/env python3
import argparse
import random
import socket
import struct
import threading
import time

PACKET_MAGIC = 0x5254

PT_SERVER_HELLO = 1
PT_CLIENT_HELLO = 2
PT_OK = 3
PT_PING = 5
PT_PONG = 6
PT_MESSAGE = 7
PT_JOIN_LOBBY = 15

PT_HELLO_UDP = 10
PT_INPUT = 11
PT_SHOOT = 13


def build_packet(packet_type, payload):
    size = len(payload)
    header = struct.pack(">HBB", PACKET_MAGIC, packet_type, size)
    return header + payload


def frame_tcp(packet_bytes):
    length = len(packet_bytes)
    return struct.pack(">H", length) + packet_bytes


def sanitize_name(name):
    out = []
    for c in name:
        if c.isalnum() or c in "_-":
            out.append(c)
        if len(out) >= 12:
            break
    return "".join(out)


class BotClient:
    def __init__(self, host, port, lobby, name, duration, chat_rate, input_rate, shoot_rate):
        self.host = host
        self.port = port
        self.lobby = lobby
        self.name = sanitize_name(name)
        self.duration = duration
        self.chat_rate = chat_rate
        self.input_rate = input_rate
        self.shoot_rate = shoot_rate
        self.tcp = None
        self.udp = None
        self.player_id = None
        self._recv_buf = bytearray()
        self._stop = threading.Event()
        self._last_ping_payload = b""

    def connect(self):
        self.tcp = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        self.tcp.connect((self.host, self.port))
        self.tcp.setblocking(True)

        self.udp = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
        self.udp.connect((self.host, self.port))

        hello_payload = ("toto|" + self.name).encode("ascii", "ignore")
        pkt = build_packet(PT_CLIENT_HELLO, hello_payload)
        self.tcp.sendall(frame_tcp(pkt))

        # Read until OK to get player id
        deadline = time.time() + 5.0
        while self.player_id is None and time.time() < deadline:
            self._recv_once()
        if self.player_id is None:
            raise RuntimeError("Handshake failed (no OK)")

        join_payload = self.lobby.encode("ascii", "ignore")
        pkt = build_packet(PT_JOIN_LOBBY, join_payload)
        self.tcp.sendall(frame_tcp(pkt))

        self._send_udp_hello()

    def _send_udp_hello(self):
        payload = struct.pack(">HBB", self.player_id, 0, 0)
        pkt = build_packet(PT_HELLO_UDP, payload)
        self.udp.send(pkt)

    def _recv_once(self):
        try:
            data = self.tcp.recv(4096)
        except BlockingIOError:
            return
        if not data:
            self._stop.set()
            return
        self._recv_buf.extend(data)
        self._parse_frames()

    def _parse_frames(self):
        while len(self._recv_buf) >= 2:
            length = (self._recv_buf[0] << 8) | self._recv_buf[1]
            if len(self._recv_buf) < 2 + length:
                return
            frame = bytes(self._recv_buf[2:2 + length])
            del self._recv_buf[:2 + length]
            self._handle_packet(frame)

    def _handle_packet(self, frame):
        if len(frame) < 4:
            return
        header, ptype, size = struct.unpack(">HBB", frame[:4])
        if header != PACKET_MAGIC:
            return
        payload = frame[4:4 + size]
        if ptype == PT_OK and len(payload) >= 2:
            self.player_id = (payload[0] << 8) | payload[1]
        elif ptype == PT_PING:
            self._last_ping_payload = payload
            pong = build_packet(PT_PONG, payload)
            self.tcp.sendall(frame_tcp(pong))

    def start(self):
        self.tcp.setblocking(False)
        threads = [
            threading.Thread(target=self._recv_loop, daemon=True),
            threading.Thread(target=self._chat_loop, daemon=True),
            threading.Thread(target=self._input_loop, daemon=True),
            threading.Thread(target=self._shoot_loop, daemon=True),
        ]
        for t in threads:
            t.start()
        time.sleep(self.duration)
        self._stop.set()

    def _recv_loop(self):
        while not self._stop.is_set():
            self._recv_once()
            time.sleep(0.001)

    def _chat_loop(self):
        if self.chat_rate <= 0:
            return
        interval = 1.0 / self.chat_rate
        while not self._stop.is_set():
            msg = f"bot {self.name} ping"
            payload = msg.encode("ascii", "ignore")
            pkt = build_packet(PT_MESSAGE, payload)
            self.tcp.sendall(frame_tcp(pkt))
            time.sleep(interval)

    def _input_loop(self):
        if self.input_rate <= 0:
            return
        interval = 1.0 / self.input_rate
        while not self._stop.is_set():
            velx = random.randint(-5, 5)
            vely = random.randint(-5, 5)
            dirv = random.randint(0, 3)
            payload = struct.pack(">HBBBBB",
                                  self.player_id,
                                  0,
                                  0,
                                  velx & 0xFF,
                                  vely & 0xFF,
                                  dirv & 0xFF)
            pkt = build_packet(PT_INPUT, payload)
            self.udp.send(pkt)
            time.sleep(interval)

    def _shoot_loop(self):
        if self.shoot_rate <= 0:
            return
        interval = 1.0 / self.shoot_rate
        while not self._stop.is_set():
            velx = random.randint(2, 5)
            vely = random.randint(-1, 1)
            payload = struct.pack(">HBBBB",
                                  self.player_id,
                                  0,
                                  0,
                                  velx & 0xFF,
                                  vely & 0xFF)
            pkt = build_packet(PT_SHOOT, payload)
            self.udp.send(pkt)
            time.sleep(interval)


def main():
    parser = argparse.ArgumentParser(description="Mystic-Type stress bot")
    parser.add_argument("--host", default="127.0.0.1")
    parser.add_argument("--port", type=int, default=4243)
    parser.add_argument("--lobby", default="PUBLIC")
    parser.add_argument("--name", default="BOT")
    parser.add_argument("--duration", type=int, default=60)
    parser.add_argument("--chat-rate", type=float, default=0.2, help="messages per second")
    parser.add_argument("--input-rate", type=float, default=10.0, help="inputs per second")
    parser.add_argument("--shoot-rate", type=float, default=2.0, help="shots per second")
    args = parser.parse_args()

    bot = BotClient(
        host=args.host,
        port=args.port,
        lobby=args.lobby,
        name=args.name,
        duration=args.duration,
        chat_rate=args.chat_rate,
        input_rate=args.input_rate,
        shoot_rate=args.shoot_rate,
    )
    bot.connect()
    bot.start()


if __name__ == "__main__":
    main()
