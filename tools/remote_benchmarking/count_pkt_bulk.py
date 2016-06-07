import sys
from pexpect import pxssh

HOST4 = 'comp4'
HOST5 = 'comp5'
HOST6 = 'comp6'
USER = 'root'

args = sys.argv

dic = {0: '0',
	1: '0,1',
	2: '0,1,4',
	3: '0,1,4,5',
	4: '0,1,4,5',
	5: '0,1,4,5',
	6: '0,1,4,5',
	7: '0,1,4,5',
	8: '0,1,4,5',
	9: '0,1,4,5',
	10: '0,1,4,5',
	11: '0,1,4,5',
	12: '0,1,4,5',
	13: '0,1,4,5',
	16: '0,1,4,5',
	32: '0,1,4,5',
}

entries = {
	'threads' : int(args[1]),
	'type' : args[2], # tcp, pkt
	'delta' : args[3],
	'buffer' : args[4],
	'extra' : args[5],
	'memory' : 128,
	'taskset' : dic[int(args[1])],
	'bulk' : args[6],
	'parallel' : args[7],
}


try:
    comp4 = pxssh.pxssh()
    comp5 = pxssh.pxssh()
    comp6 = pxssh.pxssh()
    comp4.login(HOST4, USER)
    comp5.login(HOST5, USER)
    comp6.login(HOST6, USER)

    comp5.sendline('cd /root/srmthesis/services/bulk/')
    comp5.prompt(timeout=30)
    print(comp5.before)

    #comp5.sendline('pkill tcpdump; nohup tcpdump -i enp1s0f1 -s 200 -w /mnt/ramfs/bench_bulk_count.pcap &')
    comp5.sendline('for i in /sys/class/net/enp1s0f0/statistics/* ; do echo -n "${i}: " && cat $i; done > /mnt/ramfs/in_before')
    comp5.prompt(timeout=120)
    comp5.sendline('for i in /sys/class/net/enp1s0f1/statistics/* ; do echo -n "${i}: " && cat $i; done > /mnt/ramfs/out_before')
    comp5.prompt(timeout=120)
    print(comp5.before)

    comp5.sendline('pkill bulk; ip -6 sr action flush && bash -c \'taskset -c {taskset} ./bulk -{bulk} --memory {memory} --threads {threads} --type {type} --delta {delta} --buffer {buffer} fc01::5\' >> /root/srmthesis/res/bulk/bench_bulk_{bulk}_{extra}_th{threads}_{type}_buf{buffer}_delta{delta}_P{parallel} 2>&1'.format(**entries))

    print('\n\nService launched\n\n')

    comp4.sendline('pkill iperf3; nohup iperf3 -s -B fc00::44 &')
    comp4.prompt(timeout=120)
    print(comp4.before)


    print('\n\nIperf3 server launched\n\n')

    comp6.sendline('pkill iperf3; iperf3 -B fc01::6 -c fc00::44 -t 60 -P {parallel} -F /mnt/ramfs/coucou >> /mnt/ramfs/iperf_bench_bulk_{bulk}_{extra}_th{threads}_{type}_buf{buffer}_delta{delta}_P{parallel}'.format(**entries))
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

    comp5.sendline('for i in /sys/class/net/enp1s0f0/statistics/* ; do echo -n "${i}: " && cat $i; done > /mnt/ramfs/in_after')
    comp5.prompt(timeout=120)
    comp5.sendline('for i in /sys/class/net/enp1s0f1/statistics/* ; do echo -n "${i}: " && cat $i; done > /mnt/ramfs/out_after')
    comp5.prompt(timeout=120)
    comp5.sendline('cd /mnt/ramfs/ && a=`grep rx_bytes in_before | cut -d\' \' -f2`;b=`grep rx_bytes in_after | cut -d\' \' -f2`; echo "${{b}} - ${{a}}" | bc >> res_bytes_in_bench_bulk_{bulk}_{extra}_th{threads}_{type}_buf{buffer}_delta{delta}_P{parallel}'.format(**entries))
    comp5.prompt(timeout=120)
    comp5.sendline('cd /mnt/ramfs/ && a=`grep tx_bytes out_before | cut -d\' \' -f2`;b=`grep tx_bytes out_after | cut -d\' \' -f2`; echo "${{b}} - ${{a}}" | bc >> res_bytes_out_bench_bulk_{bulk}_{extra}_th{threads}_{type}_buf{buffer}_delta{delta}_P{parallel}'.format(**entries))
    comp5.prompt(timeout=120)
    comp5.sendline('cd /mnt/ramfs/ && a=`grep rx_packets in_before | cut -d\' \' -f2`;b=`grep rx_packets in_after | cut -d\' \' -f2`; echo "${{b}} - ${{a}}" | bc >> res_pkt_in_bench_bulk_{bulk}_{extra}_th{threads}_{type}_buf{buffer}_delta{delta}_P{parallel}'.format(**entries))
    comp5.prompt(timeout=120)
    comp5.sendline('cd /mnt/ramfs/ && a=`grep tx_packets out_before | cut -d\' \' -f2`;b=`grep tx_packets out_after | cut -d\' \' -f2`; echo "${{b}} - ${{a}}" | bc >> res_pkt_out_bench_bulk_{bulk}_{extra}_th{threads}_{type}_buf{buffer}_delta{delta}_P{parallel}'.format(**entries))
    comp5.prompt(timeout=120)
    print(comp5.before)

    #comp5.sendline('pkill tcpdump')
    #comp5.prompt()
    #print(comp5.before)
    #comp5.sendline('cd /mnt/ramfs/ && tshark -r ooo_memlink_mem{}_threads{}.pcap -Y tcp.analysis.out_of_order -T fields -e frame.number >> count_ooo_memlink_mem{}_threads{}'.format(memory, nb_threads, memory, nb_threads))
    #comp5.prompt(timeout=4000)
    #comp5.sendline('cd /mnt/ramfs/ && /bin/rm ooo_memlink_mem*.pcap')
    #comp5.prompt(timeout=4000)
    #print(comp5.before)

    comp4.logout()
    comp5.logout()
    comp6.logout()
except pxssh.ExceptionPxssh as e:
    print("pxssh failed on login.")
    print(e)
