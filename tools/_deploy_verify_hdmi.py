import paramiko, time, sys
from pathlib import Path

HOST="192.168.5.5"; USER="root"; PASS="1"
ROOT=Path(r"D:/godot project/240pal/MiSTer_PAL")
c=paramiko.SSHClient(); c.set_missing_host_key_policy(paramiko.AutoAddPolicy())
c.connect(HOST, username=USER, password=PASS, timeout=30, allow_agent=False, look_for_keys=False)
sftp=c.open_sftp()

def run(cmd, timeout=120):
    print("$", cmd, flush=True)
    _,o,e=c.exec_command(cmd, timeout=timeout)
    out=o.read().decode("utf-8","replace"); err=e.read().decode("utf-8","replace"); code=o.channel.recv_exit_status()
    if out: sys.stdout.write(out if out.endswith("\n") else out+"\n")
    if err.strip(): print("STDERR:", err[:1500], flush=True)
    print(f"[exit {code}]", flush=True)
    return out, code

# 1) Upload RBF via temp (exFAT overwrite safety). DO NOT touch PAL.rbf
run("killall -9 PAL 2>/dev/null; true")
time.sleep(1)
print("PUT PAL2.rbf", flush=True)
sftp.put(str(ROOT/"_Other/PAL2.rbf"), "/media/fat/_Other/PAL2.rbf.new")
run("mv -f /media/fat/_Other/PAL2.rbf.new /media/fat/_Other/PAL2.rbf")
run("ls -la /media/fat/_Other/PAL2.rbf /media/fat/_Other/PAL.rbf 2>/dev/null; md5sum /media/fat/_Other/PAL2.rbf; ls /media/fat/*.rbf /media/fat/_Other/*.rbf 2>/dev/null | head")

# 2) Hard reload: menu -> PAL2
run('echo load_core "/media/fat/menu.rbf" > /dev/MiSTer_cmd')
time.sleep(6)
run('echo load_core "/media/fat/_Other/PAL2.rbf" > /dev/MiSTer_cmd')
print("Waiting 15s after PAL2 load...", flush=True)
time.sleep(15)

# 3) Ensure ARM is running (handler may autostart; else start)
out,_=run("cat /tmp/CORENAME 2>/dev/null; pidof PAL; ps w | grep -E 'MiSTer|./PAL' | grep -v grep")
if "PAL" not in (out or "") or not any(x.strip().isdigit() for x in (out or "").splitlines() if x.strip().isdigit() or True):
    pass
# start game if needed
out2,_=run("pidof PAL || true")
if not out2.strip():
    run("mkdir -p /media/fat/logs/PAL; cd /media/fat/games/PAL2 && ./PAL -nativevideo -game /media/fat/games/PAL2/games >/media/fat/logs/PAL/game.log 2>&1 &")
    time.sleep(5)
else:
    print("PAL already running:", out2.strip(), flush=True)
    time.sleep(3)

run("ps w | grep -E 'MiSTer|./PAL' | grep -v grep; echo CORENAME=$(cat /tmp/CORENAME 2>/dev/null); md5sum /media/fat/_Other/PAL2.rbf /media/fat/games/PAL2/PAL /media/fat/games/PAL/PAL; ls -la /media/fat/games/PAL/Games/ogg/05.ogg")

# 4) Verify peek: FB + audio ring stats
peek = r'''python3 - <<'PY'
import os, mmap, struct, math
BASE=0x3A000000
SIZE=0x80000
fd=os.open('/dev/mem', os.O_RDONLY | getattr(os,'O_SYNC',0))
mm=mmap.mmap(fd, SIZE, mmap.MAP_SHARED, mmap.PROT_READ, offset=BASE)
ctrl=struct.unpack_from('<I', mm, 0)[0]
fbk=struct.unpack_from('<I', mm, 0x18)[0]
wptr=struct.unpack_from('<I', mm, 0x20)[0]
rptr=struct.unpack_from('<I', mm, 0x28)[0]
mid_off=0x100 + (320*224)
mid=struct.unpack_from('<8I', mm, mid_off)
words=struct.unpack_from('<'+'I'*64, mm, 0x100)
nonzero_fb=sum(1 for w in words if w!=0)
mid_nz=sum(1 for w in mid if w!=0)
# audio ring: 4096 stereo S16 at +0x48000
ring_off=0x48000
n=4096*2
samples=struct.unpack_from('<'+'h'*n, mm, ring_off)
absvals=[abs(s) for s in samples]
peak=max(absvals) if absvals else 0
mean=sum(absvals)/len(absvals) if absvals else 0
mad=sum(abs(a-mean) for a in absvals)/len(absvals) if absvals else 0
nz=sum(1 for s in samples if s!=0)
ratio=(mad/peak) if peak else 999.0
print('CTRL=0x%08X frame=%u active=%u' % (ctrl, ctrl>>2, ctrl&3))
print('FEEDBACK=0x%08X WPTR=%u RPTR=%u' % (fbk, wptr & 4095, rptr & 4095))
print('FB0_first64_nonzero=%u MID_8words_nz=%u MID=%s' % (nonzero_fb, mid_nz, ' '.join('0x%08X'%w for w in mid)))
print('AUD peak=%d mad=%.2f mad/peak=%.4f nonzero=%d' % (peak, mad, ratio, nz))
mm.close(); os.close(fd)
PY'''
print("=== MEM PEEK ===", flush=True)
run(peek)
print("=== LOG TAIL ===", flush=True)
run("grep -E 'presents|DROP|drops=|wait_TIMEOUT|DIAG|OGG|NativeVideo|volume|Audio' /media/fat/logs/PAL/game.log 2>/dev/null | tail -40")
run("echo PRESENT_LINES=$(grep -c presents /media/fat/logs/PAL/game.log 2>/dev/null || echo 0); echo DROP_COUNT=$(grep -cE 'DROP|drops=' /media/fat/logs/PAL/game.log 2>/dev/null || echo 0)")

sftp.close(); c.close()
print("DEPLOY+BOOT DONE", flush=True)
