import paramiko, time
c=paramiko.SSHClient()
c.set_missing_host_key_policy(paramiko.AutoAddPolicy())
c.connect("192.168.5.5", username="root", password="1", timeout=20, allow_agent=False, look_for_keys=False)

def run(cmd, timeout=60):
    chan=c.get_transport().open_session()
    chan.settimeout(timeout)
    chan.exec_command(cmd)
    out=b""
    err=b""
    while True:
        if chan.recv_ready():
            out+=chan.recv(65535)
        if chan.recv_stderr_ready():
            err+=chan.recv_stderr(65535)
        if chan.exit_status_ready():
            while chan.recv_ready(): out+=chan.recv(65535)
            while chan.recv_stderr_ready(): err+=chan.recv_stderr(65535)
            break
        time.sleep(0.05)
    code=chan.recv_exit_status()
    print("===", cmd)
    print(out.decode("utf-8","replace"))
    if err: print("ERR:", err.decode("utf-8","replace"))
    print("[exit", code, "]")
    return out.decode("utf-8","replace")

run('killall -9 PAL 2>/dev/null || true; sleep 1; cd /media/fat/games/PAL2 && nohup ./PAL -game /media/fat/games/PAL2/Games > /media/fat/logs/PAL2/manual_game.log 2>&1 & echo started')
time.sleep(10)
run('pidof PAL; ps w | grep PAL | grep -v grep')
run('ls -la /media/fat/games/PAL2/PAL /media/fat/_Other/PAL2.rbf')
run('tail -80 /media/fat/logs/PAL2/manual_game.log')
run("grep -iE 'RIX|AUDIO|keepalive|music|Init' /media/fat/logs/PAL2/manual_game.log /media/fat/logs/PAL2/PAL.log 2>/dev/null | tail -50")
run("grep -i keepalive /media/fat/logs/PAL2/manual_game.log /media/fat/logs/PAL2/PAL.log 2>/dev/null || true")
c.close()
