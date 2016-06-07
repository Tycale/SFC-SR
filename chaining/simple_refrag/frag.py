#!/usr/bin/python

from scapy import *
from scapy.all import *

import time,random

conf.iface6='A-0'

dip = 'fc00:2:0:2::1'
sip = 'fc00:2:0:1::1'

packets = fragment6(IPv6(src=sip, dst=dip) / IPv6ExtHdrFragment() / ICMPv6EchoRequest(data='A'*800), 100)
send(packets)
