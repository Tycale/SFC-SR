#!/usr/bin/python2

from scapy.all import IPv6, IPv6ExtHdrSegmentRouting
from random import shuffle

import helpers
import unittest

from test_bulk_buf import BulkBufTestCase


BULK_SIZE = 300

sport=50000
dport=60000


class BulkListTestCase(BulkBufTestCase):

    services = [helpers.BULK, helpers.TEST, helpers.DECOMPRESSOR]

    def setUp(self) :
        self.conf = helpers.get_conf()
        self.sfc = helpers.addr_services(self.conf, BulkListTestCase.services)

        ip6 = IPv6(src=self.sfc[-1], dst=self.sfc[0])
        sr  = IPv6ExtHdrSegmentRouting(addresses=self.sfc[::-1],
                                nseg=len(self.sfc) - 1, fseg=len(self.sfc) - 1)
        self.hdr = (ip6/sr/IPv6())

        self.sport = 50000
        self.dport = 60000

    def test_reordering_bulk(self) :
        data = helpers.random_data(300)

        infos = [ ( [
            {'flags':'', 'payload':data[0:20], 'seq':0},
            {'flags':'', 'payload':data[20:300], 'seq':20}
        ], [
            {'flags':'', 'payload':data[0:300], 'seq':0}
        ]) ]

        def craft(hdr, fl):
            (snd, rcv) = BulkBufTestCase._craft_all_tcp_info(infos, hdr, fl)
            shuffle(snd)
            return (snd, rcv)

        self._generic_test_tcp_info(infos, craft)


def suite():
    a= unittest.TestLoader().loadTestsFromName('{}.BulkListTestCase'.format(__name__))
    print a
    return a

if __name__ == b'__main__':
    unittest.main(defaultTest='suite')


