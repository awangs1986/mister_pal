import paramiko
import time
import sys
from pathlib import Path

HOST = "192.168.5.5"
USER = "root"
PASS = "1"
ROOT = Path(r"D:/godot project/240pal/MiSTer_PAL")

def run(client, cmd, timeout=120):
    print(f"\n$ {cmd[:300]}")
    stdin, stdout, stderr = client.exec_command(cmd, timeout=timeout)
    out = stdout.read().decode("utf-8", errors="replace")
    err = stderr.read().decode("utf-8", errors="replace")
    code = stdout.channel.recv_exit_status()
    if out:
        print(out, end="" if out.endswith("\n") else "\n")
    if err.strip():
        print("STDERR:", err, end="" if err.endswith("\n") else "\n")
    print(f"[exit {code}]")
    return out, err, code

def sftp_put(sftp, local, remote):
    print(f"PUT {local} -> {remote}")
    sftp.put(str(local), remote)
    st = sftp.stat(remote)
    print(f"  remote size={st.st_size}")

client = paramiko.SSHClient()
client.set_missing_host_key_policy(paramiko.AutoAddPolicy())
client.connect(HOST, username=USER, password=PASS, timeout=20, allow_agent=False, look_for_keys=False)
sftp = client.open_sftp()

run(client, "mkdir -p /media/fat/games/PAL /media/fat/logs/PAL /media/fat/_Other")
sftp_put(sftp, ROOT / "games/PAL/PAL", "/media/fat/games/PAL/PAL")
sftp_put(sftp, ROOT / "_Other/PAL2.rbf", "/media/fat/_Other/PAL2.rbf")
run(client, "chmod +x /media/fat/games/PAL/PAL")
run(client, "ls -la /media/fat/games/PAL/PAL /media/fat/_Other/PAL2.rbf")

run(client, 'echo load_core "/media/fat/_Other/PAL2.rbf" > /dev/MiSTer_cmd')
time.sleep(8)

run(client, "killall -9 PAL 2>/dev/null || true")
time.sleep(1)
run(client, "cd /media/fat/games/PAL && ./PAL -game >/media/fat/logs/PAL/game.log 2>&1 &")
time.sleep(4)

out, _, _ = run(client, "pidof PAL")
pids = out.strip().split()
print(f"PAL pid count: {len(pids)} pids={pids}")
run(client, "ps w | grep PAL | grep -v grep")

log_out, _, _ = run(client, "tail -80 /media/fat/logs/PAL/game.log 2>/dev/null; echo '---'; tail -80 /media/fat/logs/PAL/PAL.log 2>/dev/null; echo '---'; tail -80 PAL.log 2>/dev/null")
for line in log_out.splitlines():
    if any(k in line for k in ("RIX", "AUDIO", "keepalive", "RIX_Init", "music")):
        print("LOGHIT:", line)

run(client, "grep -iE 'RIX|AUDIO|keepalive' /media/fat/logs/PAL/game.log /media/fat/logs/PAL/PAL.log PAL.log 2>/dev/null | tail -30")

sftp.close()
client.close()
print("DEPLOY DONE")
