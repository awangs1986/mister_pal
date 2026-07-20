import paramiko, time, sys

HOST = "192.168.5.5"
c = paramiko.SSHClient()
c.set_missing_host_key_policy(paramiko.AutoAddPolicy())
c.connect(HOST, username="root", password="1", timeout=30, allow_agent=False, look_for_keys=False)

def run(cmd, timeout=120):
    print(f"\n$ {cmd}", flush=True)
    stdin, stdout, stderr = c.exec_command(cmd, timeout=timeout)
    out = stdout.read().decode("utf-8", "replace")
    err = stderr.read().decode("utf-8", "replace")
    code = stdout.channel.recv_exit_status()
    if out:
        sys.stdout.write(out if out.endswith("\n") else out + "\n")
    if err.strip():
        print("STDERR:", err.strip()[:2000], flush=True)
    print(f"[exit {code}]", flush=True)
    return out

peek = r"""python3 - <<'PY'
import os, mmap, struct
BASE=0x3A000000
SIZE=0x200000
fd=os.open('/dev/mem', os.O_RDONLY | getattr(os,'O_SYNC',0))
mm=mmap.mmap(fd, SIZE, mmap.MAP_SHARED, mmap.PROT_READ, offset=BASE)
ctrl=struct.unpack_from('<I', mm, 0)[0]
fbk=struct.unpack_from('<I', mm, 0x18)[0]
mid_off=0x100 + (320*224)
mid=struct.unpack_from('<8I', mm, mid_off)
words=struct.unpack_from('<'+'I'*64, mm, 0x100)
nonzero=sum(1 for w in words if w!=0)
mid_nz=sum(1 for w in mid if w!=0)
print('CTRL=0x%08X frame=%u active=%u' % (ctrl, ctrl>>2, ctrl&3))
print('FEEDBACK=0x%08X' % fbk)
print('FB0_first64_nonzero=%u MID_8words_nz=%u MID=%s' % (nonzero, mid_nz, ' '.join('0x%08X'%w for w in mid)))
mm.close(); os.close(fd)
PY"""

print("=== BASELINE PEEK ===")
run(peek)
print("=== LOGS ===")
run("ls -la /media/fat/logs/PAL/ 2>/dev/null; true")
run("for f in /media/fat/logs/PAL/game.log /media/fat/logs/PAL/PAL.log /tmp/PAL.log; do if [ -f \"$f\" ]; then echo ==== $f ====; wc -l \"$f\"; tail -80 \"$f\"; fi; done")
run("grep -hE 'DIAG|OGG|splash|GameMain|DROP|wait_TIMEOUT|presents|NativeVideo' /media/fat/logs/PAL/*.log 2>/dev/null | tail -100")

print("\n=== 20s sampling ===")
t0 = time.time()
for i in range(5):
    time.sleep(4)
    print(f"\n--- sample {i+1} t={time.time()-t0:.1f}s ---", flush=True)
    run(peek)
    run("grep -E 'presents|wait_TIMEOUT|DROP|drops=' /media/fat/logs/PAL/game.log 2>/dev/null | tail -15")

print("\n=== FINAL ===")
run("ps w | grep -E 'MiSTer|./PAL' | grep -v grep; echo CORENAME=$(cat /tmp/CORENAME); md5sum /media/fat/_Other/PAL2.rbf /media/fat/games/PAL2/PAL; ls -la /media/fat/_Other/PAL2.rbf /media/fat/games/PAL2/PAL")
run("pidof PAL; sleep 2; pidof PAL; echo STILL_ALIVE")
run("echo TIMEOUT_COUNT=$(grep -c wait_TIMEOUT /media/fat/logs/PAL/game.log 2>/dev/null || echo 0)")
run("echo DROP_COUNT=$(grep -cE 'DROP|drops=' /media/fat/logs/PAL/game.log 2>/dev/null || echo 0)")
run("echo PRESENT_LINES=$(grep -c presents /media/fat/logs/PAL/game.log 2>/dev/null || echo 0)")
run("grep -E 'DIAG|OGG ready|splash|GameMain|presents/s|wait_TIMEOUT|DROP' /media/fat/logs/PAL/game.log 2>/dev/null | head -60")
run("echo ...")
run("grep -E 'presents/s|wait_TIMEOUT|DROP' /media/fat/logs/PAL/game.log 2>/dev/null | tail -40")
c.close()
print("\nVERIFY DONE", flush=True)
