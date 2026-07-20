#!/bin/bash
#
# PAL2 handler — always launch SDLPAL against Games/ (no testpattern).
# Logs: /media/fat/logs/PAL2/PAL.log  (grep [DIAG] for bounce/flicker)
#
# CRITICAL: only ONE ./PAL may write the DDR audio ring. Dual writers
# corrupt PCM (crackles/noise) even when wptr/rptr rates look ~48 kHz.
# The PAL binary itself takes /var/run/PAL.singleton.lock (flock).
#

GAMEDIR="/media/fat/games/PAL2"
LOGDIR="/media/fat/logs/PAL2"

cd "$GAMEDIR" || exit 1
mkdir -p "$LOGDIR" "$GAMEDIR/Games"

# Drop any leftover writers (manual nohup / raced deploy) before start.
killall -9 PAL 2>/dev/null
sleep 0.3

# Keep a short history so agent can compare runs
if [ -f "$LOGDIR/PAL.log" ]; then
  mv -f "$LOGDIR/PAL.prev.log" "$LOGDIR/PAL.prev2.log" 2>/dev/null
  mv -f "$LOGDIR/PAL.log" "$LOGDIR/PAL.prev.log" 2>/dev/null
fi

if [ -f /tmp/pal_reset_marker ]; then
    rm -f /tmp/pal_reset_marker 2>/dev/null
else
    rm -f /media/fat/config/PAL2.s0 2>/dev/null
fi

echo 3 > /proc/sys/vm/drop_caches 2>/dev/null
sleep 1

{
  echo "===== PAL2 handler $(date -Iseconds) pid=$$ ====="
  echo "DIAG: grep [DIAG] in this file for input/vid/life/shutdown"
  echo "map: Start=Search(bit5) Select=Menu(bit6) Q=Flee(bit9)"
  echo "note: PAL binary enforces singleton flock /var/run/PAL.singleton.lock"
} > "$LOGDIR/PAL.log"

exec taskset 0x03 ./PAL -nativevideo -game "$GAMEDIR/Games" \
  >> "$LOGDIR/PAL.log" 2>&1
