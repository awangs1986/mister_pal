import paramiko, time, sys, re
HOST="192.168.5.5"; USER="root"; PASS="1"
c=paramiko.SSHClient(); c.set_missing_host_key_policy(paramiko.AutoAddPolicy())
c.connect(HOST, username=USER, password=PASS, timeout=30, allow_agent=False, look_for_keys=False)

def run(cmd, timeout=120):
    print("$", cmd[:200], flush=True)
    _,o,e=c.exec_command(cmd, timeout=timeout)
    out=o.read().decode("utf-8","replace"); err=e.read().decode("utf-8","replace"); code=o.channel.recv_exit_status()
    if out: sys.stdout.write(out if out.endswith("\n") else out+"\n")
    if err.strip(): print("STDERR:", err[:1000], flush=True)
    print(f"[exit {code}]", flush=True)
    return out

print("=== find logs / presents ===", flush=True)
run("pidof PAL; ls -la /proc/$(pidof PAL)/cwd; ls -la /media/fat/logs/PAL/ 2>/dev/null; ls -la /tmp/*PAL* /tmp/*pal* 2>/dev/null; true")
run("find /media/fat/logs /tmp /media/fat/games/PAL2 -name '*.log' 2>/dev/null | head -40")
run("tr '\\0' ' ' < /proc/$(pidof PAL)/cmdline; echo; ls -l /proc/$(pidof PAL)/fd 2>/dev/null | head -40")

# sample FB + audio + presents over ~12s
peek = r'''python3 - <<'PY'
import os, mmap, struct
BASE=0x3A000000; SIZE=0x80000
fd=os.open('/dev/mem', os.O_RDONLY | getattr(os,'O_SYNC',0))
mm=mmap.mmap(fd, SIZE, mmap.MAP_SHARED, mmap.PROT_READ, offset=BASE)
ctrl=struct.unpack_from('<I', mm, 0)[0]
mid=struct.unpack_from('<8I', mm, 0x100+(320*224))
mid_nz=sum(1 for w in mid if w!=0)
samples=struct.unpack_from('<'+'h'*(4096*2), mm, 0x48000)
absvals=[abs(s) for s in samples]
peak=max(absvals); mean=sum(absvals)/len(absvals); mad=sum(abs(a-mean) for a in absvals)/len(absvals)
nz=sum(1 for s in samples if s!=0)
print('frame=%u mid_nz=%u peak=%d mad/peak=%.4f nonzero=%d' % (ctrl>>2, mid_nz, peak, (mad/peak if peak else 999), nz))
mm.close(); os.close(fd)
PY'''

presents_re = re.compile(r'presents/?s[=:\s]+([0-9.]+)|presents=(\d+).*drops=(\d+)|drops=(\d+)', re.I)
print("=== 12s sampling ===", flush=True)
for i in range(4):
    time.sleep(3)
    print(f"--- sample {i+1} ---", flush=True)
    run(peek)
    run("grep -hE 'presents|DROP|drops=' /media/fat/logs/PAL/*.log /tmp/*.log /media/fat/games/PAL2/*.log /media/fat/games/PAL2/Games/*.log 2>/dev/null | tail -20")

run("dmesg | tail -5; echo CORENAME=$(cat /tmp/CORENAME); pidof PAL; sleep 2; pidof PAL; echo STILL_ALIVE")
# try strace-less: read any stdout redirected
run("ls -la /media/fat/games/PAL2/; tail -50 /media/fat/games/PAL2/PAL.log 2>/dev/null; tail -50 /media/fat/games/PAL2/Games/PAL.log 2>/dev/null; true")
c.close()
print("DONE", flush=True)
