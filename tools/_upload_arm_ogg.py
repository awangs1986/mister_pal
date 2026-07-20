import paramiko, time
from pathlib import Path
HOST="192.168.5.5"; USER="root"; PASS="1"
ROOT=Path(r"D:/godot project/240pal/MiSTer_PAL")
c=paramiko.SSHClient(); c.set_missing_host_key_policy(paramiko.AutoAddPolicy())
c.connect(HOST, username=USER, password=PASS, timeout=20, allow_agent=False, look_for_keys=False)
sftp=c.open_sftp()
def run(cmd):
    print("$", cmd)
    _,o,e=c.exec_command(cmd, timeout=60)
    out=o.read().decode("utf-8","replace"); err=e.read().decode("utf-8","replace"); code=o.channel.recv_exit_status()
    if out: print(out, end="" if out.endswith("\n") else "\n")
    if err.strip(): print("STDERR:", err[:800])
    print("[exit", code, "]"); return out, code
run("killall -9 PAL 2>/dev/null; true")
time.sleep(1)
run("mkdir -p /media/fat/games/PAL/Games/ogg /media/fat/games/PAL2/games/ogg /media/fat/logs/PAL /media/fat/_Other")
run("df -h /media/fat; ls -la /media/fat/games/PAL/; mount | grep fat")
pairs=[
    (ROOT/"games/PAL/PAL", "/media/fat/games/PAL/PAL"),
    (ROOT/"games/PAL2/PAL", "/media/fat/games/PAL2/PAL"),
    (ROOT/"games/PAL/Games/ogg/05.ogg", "/media/fat/games/PAL/Games/ogg/05.ogg"),
    (ROOT/"games/PAL2/games/ogg/05.ogg", "/media/fat/games/PAL2/games/ogg/05.ogg"),
]
for local, remote in pairs:
    tmp = remote + ".new"
    print("PUT", local, "->", tmp)
    try:
        sftp.put(str(local), tmp)
        st=sftp.stat(tmp); print("  tmp size=", st.st_size)
        run("mv -f '%s' '%s'" % (tmp, remote))
    except Exception as ex:
        print("FAIL", ex)
        run("ls -la '%s' '%s' 2>/dev/null; true" % (remote, tmp))
        # fallback scp-style via cat?
        raise
run("chmod +x /media/fat/games/PAL/PAL /media/fat/games/PAL2/PAL")
run("ls -la /media/fat/games/PAL/PAL /media/fat/games/PAL2/PAL /media/fat/games/PAL/Games/ogg/05.ogg /media/fat/games/PAL2/games/ogg/05.ogg")
run("md5sum /media/fat/games/PAL/PAL /media/fat/games/PAL2/PAL")
sftp.close(); c.close(); print("ARM+OGG UPLOAD DONE")
