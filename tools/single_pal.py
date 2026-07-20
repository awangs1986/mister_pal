import paramiko, time
c=paramiko.SSHClient()
c.set_missing_host_key_policy(paramiko.AutoAddPolicy())
c.connect("192.168.5.5", username="root", password="1", timeout=20, allow_agent=False, look_for_keys=False)

def run(cmd):
    i,o,e=c.exec_command(cmd, timeout=90)
    out=o.read().decode("utf-8","replace")
    print("===", cmd)
    print(out)

run('killall -9 PAL 2>/dev/null || true')
time.sleep(2)
run('cd /media/fat/games/PAL2 && nohup ./PAL -game /media/fat/games/PAL2/Games > /media/fat/logs/PAL2/manual_game.log 2>&1 & echo $!')
time.sleep(5)
run('pidof PAL; ps w | grep " PAL" | grep -v grep')
run('tail -40 /media/fat/logs/PAL2/manual_game.log')
run("grep -i keepalive /media/fat/logs/PAL2/manual_game.log || echo 'no keepalive'")
run("grep -iE 'RIX|AUDIO' /media/fat/logs/PAL2/manual_game.log | tail -5")
c.close()
