import paramiko, time
c=paramiko.SSHClient()
c.set_missing_host_key_policy(paramiko.AutoAddPolicy())
c.connect("192.168.5.5", username="root", password="1", timeout=20, allow_agent=False, look_for_keys=False)

def run(cmd):
    i,o,e=c.exec_command(cmd, timeout=60)
    out=o.read().decode("utf-8","replace")
    err=e.read().decode("utf-8","replace")
    print("===", cmd[:140])
    print(out)
    if err.strip():
        print("ERR:", err)

run("pidof PAL; ps w | grep PAL | grep -v grep")
run("ls -la /media/fat/games/PAL/Games/ 2>/dev/null | head -15")
run("ls -la /media/fat/games/PAL2/Games/ 2>/dev/null | head -15")
run("cat /media/fat/games/PAL/_handler.sh")
time.sleep(3)
run("tail -120 /media/fat/logs/PAL/PAL.log 2>/dev/null")
run("grep -iE 'RIX|AUDIO|keepalive|music|Init' /media/fat/logs/PAL/PAL.log 2>/dev/null | tail -40")
run("grep -i keepalive /media/fat/logs/PAL/PAL.log 2>/dev/null | wc -l")
c.close()
