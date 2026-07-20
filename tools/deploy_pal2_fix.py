import paramiko, time
from pathlib import Path
c=paramiko.SSHClient()
c.set_missing_host_key_policy(paramiko.AutoAddPolicy())
c.connect("192.168.5.5", username="root", password="1", timeout=20, allow_agent=False, look_for_keys=False)
sftp=c.open_sftp()

local = Path(r"D:/godot project/240pal/MiSTer_PAL/games/PAL/PAL")
for remote in ["/media/fat/games/PAL/PAL", "/media/fat/games/PAL2/PAL"]:
    print(f"PUT -> {remote}")
    sftp.put(str(local), remote)
    st=sftp.stat(remote)
    print(f"  size={st.st_size}")

def run(cmd):
    i,o,e=c.exec_command(cmd, timeout=120)
    out=o.read().decode("utf-8","replace")
    print("===", cmd)
    print(out)

run('echo load_core "/media/fat/_Other/PAL2.rbf" > /dev/MiSTer_cmd')
time.sleep(8)
run('killall -9 PAL 2>/dev/null || true')
time.sleep(1)
run('cd /media/fat/games/PAL2 && ./PAL -game /media/fat/games/PAL2/Games >/media/fat/logs/PAL2/manual_game.log 2>&1 &')
time.sleep(8)
run('pidof PAL; ps w | grep PAL | grep -v grep')
run('tail -100 /media/fat/logs/PAL2/PAL.log 2>/dev/null')
run('tail -100 /media/fat/logs/PAL2/manual_game.log 2>/dev/null')
run("grep -iE 'RIX|AUDIO|keepalive|music|Init' /media/fat/logs/PAL2/PAL.log /media/fat/logs/PAL2/manual_game.log 2>/dev/null | tail -50")
run("grep -i keepalive /media/fat/logs/PAL2/PAL.log /media/fat/logs/PAL2/manual_game.log 2>/dev/null | wc -l")
sftp.close(); c.close()
