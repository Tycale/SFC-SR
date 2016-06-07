#!/usr/bin/python2

from scapy.all import conf, IPv6, IPv6ExtHdrSegmentRouting, ICMPv6EchoRequest

import bin.tester as tt
import bin.crafter as ct
import bin.sniffer as sn
import bin.matcher as mt

import helpers
import unittest


class CompressionTestCase(unittest.TestCase):

    services = [helpers.COMPRESSOR, helpers.DECOMPRESSOR, helpers.TEST]

    def setUp(self) :
        self.conf = helpers.get_conf()
        self.sfc = helpers.addr_services(self.conf, CompressionTestCase.services)

    def test_consistency(self) :
        conf.iface6="A-0"

        ip6 = IPv6(src=self.sfc[-1], dst=self.sfc[0])
        sr  = IPv6ExtHdrSegmentRouting(addresses=self.sfc[::-1],
                                nseg=len(self.sfc) - 1, fseg=len(self.sfc) - 1)
        hdr = (ip6/sr)

        matching = lambda a, b: tt.Utils.inner_IPv6(a) == tt.Utils.inner_IPv6(b)

        def craft(hdr, fl):
            tt.Utils.set_fl(hdr, fl)
            pkt = hdr/IPv6()/ICMPv6EchoRequest()

            pkt = pkt.__class__(str(pkt))

            return ([pkt], {fl: [pkt]})

        sns = tt.Tester(tt.Test(
            ct.Crafter(hdr, craft),
            [sn.filter_ICMP, sn.filter_SEG],
            mt.Matcher(matching))
        )

        self.assertTrue(sns.perform_test())

def suite():
    return unittest.TestLoader().loadTestsFromName(__name__)

if __name__ == b'__main__':
    unittest.main(defaultTest='suite')
