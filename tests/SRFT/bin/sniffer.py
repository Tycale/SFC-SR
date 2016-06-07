#!/usr/bin/python2

from scapy.all import IPv6, ICMPv6EchoRequest, IPv6ExtHdrSegmentRouting, sniff, TCP

INTERFACE="A-0"

class Sniffer(object):
    def __init__(self, test, filters, count=1, timeout=12):
        self.count   = count
        self.timeout = timeout

        self.prn = lambda pkt: test.add_to_rcv(pkt)
        self.lfilter = lambda pkt: all([f(pkt) for f in filters])

    def sniff(self):
        return sniff(iface=INTERFACE, timeout=self.timeout, count=self.count, \
                     lfilter=self.lfilter, prn=self.prn)

def filter_FL(fls, pkt):
    pkt_fl = pkt.getlayer(IPv6).fl
    return reduce(lambda x,y: x or y, [pkt_fl == fl for fl in fls])

def filter_TCP(pkt):
    return pkt.haslayer(TCP)

def filter_ICMP(pkt):
    return pkt.haslayer(ICMPv6EchoRequest)

def filter_SEG(pkt):
    if(not pkt.haslayer(IPv6ExtHdrSegmentRouting)):
        return 0
    else:
        return pkt.getlayer(IPv6ExtHdrSegmentRouting).nseg==0
