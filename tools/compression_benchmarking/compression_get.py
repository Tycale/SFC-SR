#!/usr/bin/env python2.7

from scapy.all import *
import time

FILE = "output_lzo_smb.txt"

conf.iface6 = 'D-1'

count = 0

fo = open(FILE, "w+")

def filter_SEG(pkt):
    if IPv6ExtHdrSegment in pkt:
        return 1

def trigger_count(pkt):
    global count
    count += len(pkt[1])
    fo.write("{} {}\n".format(pkt[1].fl, len(pkt[1])))

sniff(prn=trigger_count, lfilter=filter_SEG, store=0, iface=conf.iface6)

print(count)
