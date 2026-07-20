import paramiko, time, sys, re
HOST="192.168.5.5"; USER="root"; PASS="1"
c=paramiko.SSHClient(); c.set_missing_host_key_policy(paramiko.AutoAddPolicy())
c.connect(HOST, username=USER, password=PASS, timeout=30, allow_agent=False, look_for_keys=False)

def run(cmd, timeout=60):
    _,o,e=c.exec_command(cmd, timeout=timeout)
    out=o.read().decode("utf-8","replace"); code=o.channel.recv_exit_status()
    sys.stdout.write(out if out.endswith("\n") else out+"\n")
    return out

print("=== LIVE PAL2 LOG ===")
run("wc -l /media/fat/logs/PAL2/PAL.log; echo '---'; grep -E 'presents|DROP|drops=|wait_TIMEOUT|DIAG|OGG|NativeVideo|volume' /media/fat/logs/PAL2/PAL.log | tail -60")
print("=== wait 8s for more diag ===")
time.sleep(8)
run("grep -E 'presents|DROP|drops=|wait_TIMEOUT' /media/fat/logs/PAL2/PAL.log | tail -40")
run("echo PRESENT=$(grep -c presents /media/fat/logs/PAL2/PAL.log); echo DROP=$(grep -cE 'wait_TIMEOUT DROP|drops=' /media/fat/logs/PAL2/PAL.log); echo LINES=$(wc -l </media/fat/logs/PAL2/PAL.log)")
# compute presents/s from consecutive present lines if any
out=run("grep -E 'presents/s|presents=' /media/fat/logs/PAL2/PAL.log | tail -20")
# also frame rate from ctrl
peek=r'''python3 - <<'PY'
import os,mmap,struct,time
BASE=0x3A000000; SIZE=0x1000
fd=os.open('/dev/mem', os.O_RDONLY|getattr(os,'O_SYNC',0))
mm=mmap.mmap(fd,SIZE,mmap.MAP_SHARED,mmap.PROT_READ,offset=BASE)
c1=struct.unpack_from('<I',mm,0)[0]>>2
time.sleep(2.0)
c2=struct.unpack_from('<I',mm,0)[0]>>2
print('frames_delta=%u over_2s => ~%.1f fps' % ((c2-c1)&0x3fffffff, ((c2-c1)&0x3fffffff)/2.0))
mm.close(); os.close(fd)
PY'''
print("=== frame rate ===")
run(peek)
print("=== final audio ===")
run(r'''python3 - <<'PY'
import os,mmap,struct
BASE=0x3A000000; SIZE=0x80000
fd=os.open('/dev/mem', os.O_RDONLY|getattr(os,'O_SYNC',0))
mm=mmap.mmap(fd,SIZE,mmap.MAP_SHARED,mmap.PROT_READ,offset=BASE)
mid_nz=sum(1 for w in struct.unpack_from('<8I', mm, 0x100+320*224) if w)
samples=struct.unpack_from('<'+'h'*(4096*2), mm, 0x48000)
absvals=[abs(s) for s in samples]
peak=max(absvals); mean=sum(absvals)/len(absvals); mad=sum(abs(a-mean) for a in absvals)/len(absvals)
nz=sum(1 for s in samples if s)
print('mid_nz=%u peak=%d mad=%.2f mad/peak=%.4f nonzero=%d' % (mid_nz, peak, mad, mad/peak if peak else 999, nz))
print('CORE md5 check remote:')
mm.close(); os.close(fd)
PY''')
run("md5sum /media/fat/_Other/PAL2.rbf; cat /tmp/CORENAME; pidof PAL; ls -la /media/fat/games/PAL/Games/ogg/05.ogg /media/fat/games/PAL2/Games/ogg/05.ogg 2>/dev/null; ls -la /media/fat/_Other/PAL.rbf 2>/dev/null | head -1; true")
c.close()
