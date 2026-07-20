import paramiko, sys
c=paramiko.SSHClient(); c.set_missing_host_key_policy(paramiko.AutoAddPolicy())
c.connect("192.168.5.5", username="root", password="1", timeout=20, allow_agent=False, look_for_keys=False)
def run(cmd):
    print(f"\n$ {cmd}", flush=True)
    i,o,e=c.exec_command(cmd, timeout=60)
    out=o.read().decode("utf-8","replace")
    if out: sys.stdout.write(out if out.endswith("\n") else out+"\n")
    return out
run("head -40 /media/fat/logs/PAL2/PAL.log")
run("grep -nE 'GameMain|splash|OGG|shutdown|PAL_Shutdown|restart|wait_TIMEOUT DROP' /media/fat/logs/PAL2/PAL.log | head -40")
run("grep -c '\\[DIAG\\].*\\[shutdown\\]' /media/fat/logs/PAL2/PAL.log; grep '\\[DIAG\\].*\\[shutdown\\]' /media/fat/logs/PAL2/PAL.log | head -5")
run("grep -c 'wait_TIMEOUT DROP' /media/fat/logs/PAL2/PAL.log; ls /media/fat/_Other/PAL.rbf 2>&1; echo PID=$(pidof PAL) UPTIME_HINT=$(grep -oE '\\[+[0-9]+ms\\]' /media/fat/logs/PAL2/PAL.log | tail -1)")
c.close()
