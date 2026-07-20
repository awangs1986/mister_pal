import paramiko, hashlib, os, time, struct, sys

HOST='192.168.5.5'
USER='root'
PASS='1'
LOCAL=r'D:\godot project\240pal\MiSTer_PAL\_Other\PAL.rbf'
REMOTE='/media/fat/_Other/PAL2.rbf'
BACKUP='/media/fat/_Other/PAL2_broken_20260720_1825.rbf'
EXPECTED='A06476A796020DA59C01CE1608D1A3F5'

h=hashlib.md5()
with open(LOCAL,'rb') as f:
    for chunk in iter(lambda: f.read(1<<20), b''):
        h.update(chunk)
local_md5=h.hexdigest().upper()
print('LOCAL_MD5', local_md5, 'SIZE', os.path.getsize(LOCAL), flush=True)
assert local_md5==EXPECTED

print('Connecting...', flush=True)
client=paramiko.SSHClient()
client.set_missing_host_key_policy(paramiko.AutoAddPolicy())
client.connect(HOST, username=USER, password=PASS, timeout=15, allow_agent=False, look_for_keys=False, banner_timeout=15)
print('SSH OK', flush=True)

def run(cmd, timeout=90):
    print(f'\n$ {cmd}', flush=True)
    stdin, stdout, stderr = client.exec_command(cmd, timeout=timeout)
    out=stdout.read().decode('utf-8','replace')
    err=stderr.read().decode('utf-8','replace')
    code=stdout.channel.recv_exit_status()
    if out:
        sys.stdout.write(out if out.endswith('\n') else out+'\n')
    if err.strip():
        print('STDERR:', err.strip(), flush=True)
    print(f'[exit {code}]', flush=True)
    return code, out, err

# 1 backup
run(f'cp {REMOTE} {BACKUP}')
run(f'md5sum {REMOTE} {BACKUP}')
run(f'ls -la {REMOTE} {BACKUP}')

# 2 upload
print('Uploading RBF...', flush=True)
sftp=client.open_sftp()
sftp.put(LOCAL, REMOTE)
st=sftp.stat(REMOTE)
print(f'Uploaded size={st.st_size}', flush=True)
sftp.close()

code, out, _ = run(f'md5sum {REMOTE}')
dev_md5=out.strip().split()[0].upper() if out.strip() else ''
print('DEVICE_MD5', dev_md5, 'MATCH', dev_md5==EXPECTED, flush=True)
run(f'ls -la {REMOTE}')

# 3 load core
run('cat /tmp/CORENAME 2>/dev/null || true')
run('echo load_core /media/fat/_Other/PAL2.rbf > /dev/MiSTer_cmd')
time.sleep(4)
code, corename, _ = run('cat /tmp/CORENAME 2>/dev/null || true')
print('CORENAME_AFTER_LOAD1:', repr(corename.strip()), flush=True)

if corename.strip() != 'PAL2':
    print('Trying quoted load_core...', flush=True)
    run('echo "load_core \\"/_Other/PAL2.rbf\\"" > /dev/MiSTer_cmd')
    time.sleep(4)
    code, corename, _ = run('cat /tmp/CORENAME 2>/dev/null || true')
    print('CORENAME_AFTER_LOAD2:', repr(corename.strip()), flush=True)

if corename.strip() != 'PAL2':
    print('Trying killall MiSTer then load...', flush=True)
    run('killall MiSTer; sleep 1; echo load_core /media/fat/_Other/PAL2.rbf > /dev/MiSTer_cmd')
    time.sleep(6)
    code, corename, _ = run('cat /tmp/CORENAME 2>/dev/null || true')
    print('CORENAME_AFTER_LOAD3:', repr(corename.strip()), flush=True)

run(f'md5sum {REMOTE}; ls -la {REMOTE}')

# 4 DDR peek + config
peek_script = r'''python3 - <<'PY'
import os, mmap, struct
BASE=0x3A000000
SIZE=0x100000
fd=os.open('/dev/mem', os.O_RDONLY | getattr(os,'O_SYNC',0))
mm=mmap.mmap(fd, SIZE, mmap.MAP_SHARED, mmap.PROT_READ, offset=BASE)
ctrl=struct.unpack_from('<I', mm, 0)[0]
fb=struct.unpack_from('<8I', mm, 0x100)
# count nonzero among first 64 words of fb
words=struct.unpack_from('<'+'I'*64, mm, 0x100)
nonzero=sum(1 for w in words if w!=0)
print('CTRL=0x%08X frame=%u active=%u' % (ctrl, ctrl>>2, ctrl&3))
print('FB0_WORDS=', ' '.join('0x%08X'%w for w in fb))
print('FB0_first64_nonzero_words=', nonzero)
# also feedback
fbk=struct.unpack_from('<I', mm, 0x18)[0]
print('FEEDBACK=0x%08X' % fbk)
mm.close(); os.close(fd)
PY'''

run(peek_script)
# fallback with dd/hexdump if python fails
run('ls -la /media/fat/config/*PAL* /media/fat/config/*pal* 2>/dev/null; ls -la /media/fat/config/ 2>/dev/null | head -40')
run('for f in /media/fat/config/*PAL* /media/fat/config/*pal* /media/fat/config/MiSTer.ini; do [ -f "$f" ] && echo "==== $f ====" && cat "$f"; done 2>/dev/null | head -200')
run('ps w 2>/dev/null | grep -iE "pal|MiSTer" | grep -v grep; pidof PAL MiSTer 2>/dev/null; true')
run('dmesg 2>/dev/null | tail -40')

client.close()
print('\nALL DONE', flush=True)
