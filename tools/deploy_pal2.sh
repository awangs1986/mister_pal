#!/usr/bin/env bash
set -euxo pipefail
HOST=192.168.5.5
USER=root
PASS=1
ROOT="/mnt/d/godot project/240pal/MiSTer_PAL"
export SSHPASS="$PASS"
SSH="sshpass -e ssh -o StrictHostKeyChecking=no -o UserKnownHostsFile=/dev/null"
SCP="sshpass -e scp -o StrictHostKeyChecking=no -o UserKnownHostsFile=/dev/null"

$SSH "$USER@$HOST" "mkdir -p /media/fat/games/PAL /media/fat/logs/PAL /media/fat/_Other"
$SCP "$ROOT/games/PAL/PAL" "$USER@$HOST:/media/fat/games/PAL/PAL"
$SCP "$ROOT/_Other/PAL2.rbf" "$USER@$HOST:/media/fat/_Other/PAL2.rbf"
$SSH "$USER@$HOST" "chmod +x /media/fat/games/PAL/PAL; ls -la /media/fat/games/PAL/PAL /media/fat/_Other/PAL2.rbf"

$SSH "$USER@$HOST" "echo load_core \"/media/fat/_Other/PAL2.rbf\" > /dev/MiSTer_cmd; sleep 8"

$SSH "$USER@$HOST" "killall -9 PAL 2>/dev/null || true; sleep 1; cd /media/fat/games/PAL && ./PAL -game >/media/fat/logs/PAL/game.log 2>&1 & sleep 3; pidof PAL; ps | grep -v grep | grep PAL || true"
