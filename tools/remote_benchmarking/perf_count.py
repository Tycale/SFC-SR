import sys
from pexpect import pxssh

HOST4 = 'comp4'
HOST5 = 'comp5'
HOST6 = 'comp6'
USER = 'root'

args = sys.argv

memory = 128

th = (args[1])

try:
    comp4 = pxssh.pxssh()
    comp5 = pxssh.pxssh()
    comp6 = pxssh.pxssh()
    comp4.login(HOST4, USER)
    comp5.login(HOST5, USER)
    comp6.login(HOST6, USER)

    #comp5.sendline('cd /root/srmthesis/services/lib/ && git pull origin master && make')
    #comp5.prompt(timeout=30)
    #print(comp5.before)

    #comp5.sendline('cd /root/srmthesis/services/compression/ && git pull origin master && make')
    comp5.sendline('cd /root/srmthesis/services/count/')
    comp5.prompt(timeout=30)
    print(comp5.before)


    comp5.sendline('ip -6 sr action flush && bash -c \'taskset 0x01 ./count fc01::5\' >> /dev/null 2>&1')

    print('\n\nService launched\n\n')

    comp4.sendline('nohup iperf3 -s -B fc00::44 &')
    comp4.prompt(timeout=120)
    print(comp4.before)

    print('\n\nIperf3 server launched\n\n')

    comp6.sendline('iperf3 -B fc01::6 -c fc00::44 -t 60 -F /mnt/ramfs/coucou >> /mnt/ramfs/count')
    comp6.prompt(timeout=120)
    print(comp6.before)

    print('\n\nIperf3 client launched\n\n')

    comp4b = pxssh.pxssh()
    comp4b.login(HOST4, USER)
    comp4b.sendline('pkill iperf3')
    comp4b.prompt()
    print(comp4b.before)
    comp4b.logout()

    print('\n\nKilled iperf3\n\n')

    comp5.sendcontrol('C')
    comp5.prompt(timeout=120)
    print(comp5.before)

    comp4.logout()
    comp5.logout()
    comp6.logout()
except pxssh.ExceptionPxssh as e:
    print("pxssh failed on login.")
    print(e)
