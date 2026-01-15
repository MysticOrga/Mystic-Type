#!/usr/bin/env bash
set -euo pipefail

HOST="${HOST:-127.0.0.1}"
PORT="${PORT:-4243}"
LOBBY="${LOBBY:-PUBLIC}"
LOBBIES="${LOBBIES:-}"
COUNT="${COUNT:-4}"
DURATION="${DURATION:-60}"
CHAT_RATE="${CHAT_RATE:-0.2}"
INPUT_RATE="${INPUT_RATE:-10}"
SHOOT_RATE="${SHOOT_RATE:-2}"

usage() {
  cat <<EOF
Usage: ./scripts/stress_run.sh [options]

Env vars:
  HOST        (default: 127.0.0.1)
  PORT        (default: 4243)
  LOBBY       (default: PUBLIC)
  LOBBIES     (optional: comma-separated list, overrides LOBBY)
  COUNT       (default: 4)
  DURATION    (default: 60)
  CHAT_RATE   (default: 0.2)
  INPUT_RATE  (default: 10)
  SHOOT_RATE  (default: 2)

Example:
  COUNT=4 DURATION=120 CHAT_RATE=1 INPUT_RATE=20 SHOOT_RATE=5 ./scripts/stress_run.sh
EOF
}

if [[ "${1:-}" == "-h" || "${1:-}" == "--help" ]]; then
  usage
  exit 0
fi

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PIDS=()
IFS=',' read -r -a LOBBY_LIST <<< "${LOBBIES}"
for i in $(seq 1 "$COUNT"); do
  NAME="BOT${i}"
  LOBBY_NAME="${LOBBY}"
  if [[ -n "${LOBBIES}" && ${#LOBBY_LIST[@]} -gt 0 ]]; then
    idx=$(( (i - 1) % ${#LOBBY_LIST[@]} ))
    LOBBY_NAME="${LOBBY_LIST[$idx]}"
  fi
  python3 "${SCRIPT_DIR}/stress_bot.py" \
    --host "$HOST" \
    --port "$PORT" \
    --lobby "$LOBBY_NAME" \
    --name "$NAME" \
    --duration "$DURATION" \
    --chat-rate "$CHAT_RATE" \
    --input-rate "$INPUT_RATE" \
    --shoot-rate "$SHOOT_RATE" &
  PIDS+=($!)
done

for pid in "${PIDS[@]}"; do
  wait "$pid"
done
