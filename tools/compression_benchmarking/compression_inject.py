#!/usr/bin/env python2.7

from scapy.all import *
import time, pickle
from collections import defaultdict

conf.iface6 = "A-0"

# A -- B (compression) -- C

FILE = "input_lzo_smb.txt"
FILE_ASSOC = "assoc_lzo_smb.pickle"

INPUT = "./lo_smb_cap.pcap"
ADDRESS = "192.168.0.1"
ADDRESS6 = "::"

# new dump data
#INPUT = "./new.pcap"
#ADDRESS = "192.168.0.20"
#ADDRESS6 = "2a02:2788:7d4:1b81:5927:ea8b:4251:205"

# small dump data
#ADDRESS = "192.168.0.12"
#ADDRESS6 = "2a02:2788:7d4:1b81:c53f:64bd:1dc1:909e"

na = 'fc00:2:0:1::1' #node A
nb = 'fc00:2:0:3::1' #node B
nc = 'fc00:2:0:2::1' #node C
nd = 'fc00:2:0:5::1' #node D
ne = 'fc00:2:0:4::1' #node E
nf = 'fc00:2:0:7::1' #node F
ng = 'fc00:2:0:6::1' #node g

addrs = [nd, nf, ng]

port_assoc = defaultdict(set)
count_send = 0
err = []

start = time.time()

sr  = IPv6ExtHdrSegment(addresses=addrs[::-1], nseg=len(addrs)-1, lseg=len(addrs)-1)

s = conf.L3socket(iface=conf.iface6)
fo = open(FILE, "w+")

r_count = 0
count = 0
with PcapReader(INPUT) as pr:
    for p in pr:
        r_count += 1
        if hasattr(p[1], 'dst') and (hasattr(p[1], 'proto') or hasattr(p[1], 'nh')):
            
            port = None
            
            if hasattr(p[1], 'sport') and hasattr(p[1], 'dport'): # Protocol with ports identification (TCP, UDP)
                if p[1].dport >= p[1].sport:
                    port_assoc[p[1].sport].add(p[1].dport)
                else:
                    port_assoc[p[1].dport].add(p[1].sport)

                if p[1].dst == ADDRESS or p[1].dst == ADDRESS6 :
                    port = p[1].sport
                elif p[1].src == ADDRESS or p[1].src == ADDRESS6  :
                    port = p[1].dport

            if hasattr(p[1], 'nh'): #IPv6
                nh = p[1].nh
            elif hasattr(p[1], 'proto'): #IPv4
                nh = p[1].proto

            if port == None :
                port = 0
            
            if port:
                ip6 = IPv6(src=addrs[-1], dst=addrs[0], fl=count)
                hdr = (ip6/sr)

                data = hdr/p[1]
                size = len(data)
                p_size = len(p[1])

                fo.write('{} {} {} {} {} {}\n'.format(r_count, count, port, nh, p_size, size))
                count_send += len(data)
                count += 1
                try:
                    s.send(data)
                    #data.show()
                    #print((hdr/data).summary())
                    #if count == 2: break
                except p:
                    print(p)
                    err.append(data)
                if (count % 10000) == 0:
                    print count

fo.close()

print("{} packets sent ({} bytes), {} packets found in the pcap file".format(count, count_send, r_count))


end = time.time()
print("TIME ELAPSED")
print(end - start)

pf = open(FILE_ASSOC, "w+")
pic = pickle.Pickler(pf)
pic.dump(port_assoc)
pf.close()

print("\n=== error ===\n")
for e in err:
    print(e.summary())
    print(len(e))



