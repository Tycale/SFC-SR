
import os
import unittest
import ConfigParser as cp
import random
import string

from scapy.all import TCP, Raw, IPv6

CONF_FILE = "conf/local.conf"
INTERFACE = "A-0"

TEST         = 0
COMPRESSOR   = 1
DECOMPRESSOR = 2
DEFRAG       = 3
BULK         = 4
DEBULK       = 5

NAME = ['Test', 'Compressor', 'Decompressor', 'Defrag', 'Bulk', 'Debulk']
ADDR = 'Address'
UP = 'up'



def craft_tcp_info(hdr, info, sport, dport):
    crft = []

    for fields in info:
        tcp = TCP(sport=sport, dport=dport)

        for f, v in fields.iteritems():
            tcp.setfieldval(f, v)

        crft.append((hdr/tcp))

    return [i.__class__(str(i)) for i in crft]




def mult_flux(nbrFlux, fct, hdr, fl):
    snd = []
    rcv = {}

    for flux in range(fl, fl+nbrFlux):
        Utils.set_fl(hdr, flux)

        (tmp_snd, tmp_rcv) = fct(hdr, flux)

        snd.extend(tmp_snd)
        rcv[flux] = tmp_rcv

    return (snd, rcv)


def craft_tcp(hdr, data, middle, sport=50000, dport=60000, seq=0):
    middle = [x for x in sorted(middle) if x>0 and x<len(data)]
    middle.append(len(data))

    start = 0

    snd = []
    rcv = []

    for limit in middle:
        snd.append(hdr/TCP(sport=sport, dport=dport, flags='', \
                       seq=seq+start)/Raw(load=data[start:limit]))
        start = limit

    rcv.append(hdr/TCP(sport=sport, dport=dport, flags='', \
                   seq=seq)/Raw(load=data))


    return ([i.__class__(str(i)) for i in snd],
            [i.__class__(str(i)) for i in rcv])

def random_data(size=100):
    return ''.join(random.choice(string.digits + string.ascii_letters) for _ in range(size))

def addr_services(config, services):
    try:
        for i in services:
            if(not config.getboolean(NAME[i], UP)):
                raise unittest.SkipTest('Some service are not up')

        return [config.get(NAME[i], ADDR) for i in services]
    except:
        raise unittest.SkipTest('Some service are not in configuration \
                                file ({})'.format(os.getenv('SRFT_CONFIG', CONF_FILE)))

def get_conf():

    conf = cp.ConfigParser()
    conf.read(os.getenv('SRFT_CONFIG', CONF_FILE))

    return conf



class Utils(object):
    @staticmethod
    def key(pkt):
        return pkt.getlayer(IPv6).fl

    @staticmethod
    def inner_IPv6(pkt):
        if(pkt.getlayer(IPv6, 2) != None):
            return pkt.getlayer(IPv6, 2)
        else:
            return pkt.getlayer(IPv6)

    @staticmethod
    def set_fl(pkt, fl):
        pkt.getlayer(IPv6).fl = fl

