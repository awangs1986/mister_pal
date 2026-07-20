import paramiko, sys
c=paramiko.SSHClient(); c.set_missing_host_key_policy(paramiko.AutoAddPolicy())
c.connect("192.168.5.5", username="root", password="1", timeout=30, allow_agent=False, look_for_keys=False)

def run(cmd, timeout=60):
    print(f"\n$ {cmd}", flush=True)
    i,o,e=c.exec_command(cmd, timeout=timeout)
    out=o.read().decode("utf-8","replace"); err=e.read().decode("utf-8","replace")
    if out: sys.stdout.write(out if out.endswith("\n") else out+"\n")
    if err.strip(): print("STDERR:", err.strip()[:3000], flush=True)
    return out

run("PID=$(pidof PAL); echo PID=$PID; ls -l /proc/$PID/cwd /proc/$PID/exe; tr '\\0' ' ' < /proc/$PID/cmdline; echo; ls -l /proc/$PID/fd 2>/dev/null | head -40")
run("PID=$(pidof PAL); for fd in 1 2; do echo FD$fd; readlink /proc/$PID/fd/$fd; done")
run("find /media/fat /tmp /var/log -name '*PAL*' -o -name '*pal*' -o -name '*diag*' 2>/dev/null | head -50")
run("ls -la /media/fat/games/PAL2/; ls -la /media/fat/games/PAL2/*.log 2>/dev/null; ls -la /tmp/ | head -40")
run("dmesg | tail -80")
run("grep -rE 'DIAG|presents|wait_TIMEOUT|DROP' /tmp /media/fat/logs 2>/dev/null | head -40")
# Check if PAL writes to a tty or mister log
run("ls -la /media/fat/config/PAL2* /media/fat/config/*PAL* 2>/dev/null; cat /media/fat/config/PAL2*.ini 2>/dev/null | head -40")
c.close()
