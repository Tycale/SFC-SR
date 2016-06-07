#!/usr/bin/python2

from scapy.all import IPv6, IPv6ExtHdrSegmentRouting

import bin.tester as tt
import bin.crafter as ct
import bin.matcher as mt
import bin.sniffer as sn

import helpers
import unittest


BULK_SIZE = 300

sport=50000
dport=60000

def _craft_all_tcp_info(infos, hdr, fl):
    snd = []
    exp = {}

    for i, (snd_info, exp_info) in enumerate(infos):
        flux = fl + i
        helpers.Utils.set_fl(hdr, flux)

        snd.extend(helpers.craft_tcp_info(hdr, snd_info, sport+flux, dport+flux))
        exp[flux] = helpers.craft_tcp_info(hdr, exp_info, sport+flux, dport+flux)

    return (snd, exp)

def _craft_tcp_bulk(hdr, fl):
    snd = []
    rcv = {}
    helpers.Utils.set_fl(hdr, fl)
    (tsnd, trcv) =  helpers.craft_tcp( hdr, helpers.random_data(300),
            [20], sport=sport+fl, dport=dport+fl
        )

    snd.extend(tsnd)
    rcv[fl] = trcv

    return (snd, rcv)


class BulkBufTestCase(unittest.TestCase):

    services = [helpers.BULK, helpers.TEST, helpers.DEBULK]

    def setUp(self) :
        self.conf = helpers.get_conf()
        self.sfc = helpers.addr_services(self.conf, BulkBufTestCase.services)

        ip6 = IPv6(src=self.sfc[-1], dst=self.sfc[0])
        sr  = IPv6ExtHdrSegmentRouting(addresses=self.sfc[::-1],
                                nseg=len(self.sfc) - 1, fseg=len(self.sfc) - 1)
        self.hdr = (ip6/sr/IPv6())

        self.sport = 50000
        self.dport = 60000

        self.nflux = 1

    def _generic_test_tcp_info(self, infos, craft, nflux=1):
        nfl = len(infos)
        nexp = sum([len(x[1]) for x in infos ])

        sns = tt.Tester(tt.Test(
        ct.Crafter(self.hdr, craft, nfl, nexp, nflux),
            [sn.filter_TCP, sn.filter_SEG],
            mt.Matcher())
        )
        self.assertTrue(sns.perform_test())

    def test_consecutif_bulk(self) :
        data = helpers.random_data(300)

        infos = [ ( [
            {'flags':'', 'payload':data[0:20], 'seq':0},
            {'flags':'', 'payload':data[20:300], 'seq':20}
        ], [
            {'flags':'', 'payload':data[0:300], 'seq':0}
        ]) ]

        craft = lambda hdr, fl: _craft_all_tcp_info(infos, hdr, fl)
        self._generic_test_tcp_info(infos, craft, self.nflux)

    def test_flag_CE(self):
        data = helpers.random_data(300)

        infos = [ ( [
            {'flags':'E', 'payload':data[0:20], 'seq':0},
            {'flags':'', 'payload':data[20:300], 'seq':20}
        ], [
            {'flags':'E', 'payload':data[0:300], 'seq':0}
        ] ), ( [
            {'flags':'', 'payload':data[0:20], 'seq':0},
            {'flags':'E', 'payload':data[20:300], 'seq':20}
        ], [
            {'flags':'E', 'payload':data[0:300], 'seq':0}
        ] ), ( [
            {'flags':'E', 'payload':data[0:20], 'seq':0},
            {'flags':'E', 'payload':data[20:300], 'seq':20}
        ], [
            {'flags':'E', 'payload':data[0:300], 'seq':0}
        ] ),( [
            {'flags':'C', 'payload':data[0:20], 'seq':0},
            {'flags':'', 'payload':data[20:300], 'seq':20}
        ], [
            {'flags':'C', 'payload':data[0:300], 'seq':0}
        ] ), ( [
            {'flags':'', 'payload':data[0:20], 'seq':0},
            {'flags':'C', 'payload':data[20:300], 'seq':20}
        ], [
            {'flags':'C', 'payload':data[0:300], 'seq':0}
        ] ), ( [
            {'flags':'C', 'payload':data[0:20], 'seq':0},
            {'flags':'C', 'payload':data[20:300], 'seq':20}
        ], [
            {'flags':'C', 'payload':data[0:300], 'seq':0}
        ] ) ]

        craft = lambda hdr, fl: _craft_all_tcp_info(infos, hdr, fl)
        self._generic_test_tcp_info(infos, craft)

    def test_flag_URS(self):
        data = helpers.random_data(300)

        infos = [ ( [
            {'flags':'U', 'payload':data[0:20], 'seq':0},
            {'flags':'', 'payload':data[20:300], 'seq':20}
        ], [
            {'flags':'U', 'payload':data[0:20], 'seq':0},
            {'flags':'', 'payload':data[20:300], 'seq':20}
        ] ), ( [
            {'flags':'R', 'payload':data[0:20], 'seq':0},
            {'flags':'', 'payload':data[20:300], 'seq':20}
        ], [
            {'flags':'R', 'payload':data[0:20], 'seq':0},
            {'flags':'', 'payload':data[20:300], 'seq':20}
        ] ), ( [
            {'flags':'S', 'seq':0},
            {'flags':'', 'payload':data[20:300], 'seq':20}
        ], [
            {'flags':'S', 'seq':0},
            {'flags':'', 'payload':data[20:300], 'seq':20}
        ] ) ]

        craft = lambda hdr, fl: _craft_all_tcp_info(infos, hdr, fl)
        self._generic_test_tcp_info(infos, craft)

    def test_flag_A(self):
        data = helpers.random_data(300)

        infos = [ ( [
            {'flags':'A', 'payload':data[0:20], 'seq':0, 'ack':30},
            {'flags':'A', 'payload':data[20:300],'seq':20, 'ack':40}
        ], [
            {'flags':'A', 'payload':data[0:300], 'seq':0, 'ack':40},
        ] ), ( [
            {'flags':'A', 'payload':data[0:20], 'seq':0, 'ack':30},
            {'flags':'', 'payload':data[20:300],'seq':20, 'ack':40}
        ], [
            {'flags':'A', 'payload':data[0:300], 'seq':0, 'ack':30},
        ] ), ( [
            {'flags':'A', 'payload':data[0:20], 'seq':0, 'ack':40},
            {'flags':'A', 'payload':data[20:300],'seq':20, 'ack':30}
        ], [
            {'flags':'A', 'payload':data[0:300], 'seq':0, 'ack':40},
        ] ) ]

        craft = lambda hdr, fl: _craft_all_tcp_info(infos, hdr, fl)
        self._generic_test_tcp_info(infos, craft)

    def test_flag_PF(self):
        data = helpers.random_data(300)

        infos = [ ( [
            {'flags':'', 'payload':data[0:20], 'seq':0},
            {'flags':'P', 'payload':data[20:100], 'seq':20},
            {'flags':'', 'payload':data[100:300], 'seq':100}
        ], [
            {'flags':'P', 'payload':data[0:100], 'seq':0},
            {'flags':'', 'payload':data[100:300], 'seq':100}
        ] ), ( [
            {'flags':'', 'payload':data[0:20], 'seq':0},
            {'flags':'F', 'payload':data[20:100], 'seq':20},
            {'flags':'', 'payload':data[100:300], 'seq':100}
        ], [
            {'flags':'F', 'payload':data[0:100], 'seq':0},
            {'flags':'', 'payload':data[100:300], 'seq':100}
        ] ) ]

        craft = lambda hdr, fl: _craft_all_tcp_info(infos, hdr, fl)
        self._generic_test_tcp_info(infos, craft)


def suite():
    return unittest.TestLoader().loadTestsFromName(__name__)

if __name__ == b'__main__':
    unittest.main(defaultTest='suite')


