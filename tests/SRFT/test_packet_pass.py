#!/usr/bin/python2

from scapy.all import conf, IPv6, IPv6ExtHdrSegmentRouting, ICMPv6EchoRequest

import bin.tester as tt
import bin.matcher as mt
import bin.crafter as ct
import bin.sniffer as sn

import helpers
import unittest


class PacketPassTestCase(unittest.TestCase):

    services = [helpers.BULK, helpers.TEST, helpers.DEBULK]

    def setUp(self) :
        self.conf = helpers.get_conf()
        self.sfc = helpers.addr_services(self.conf, PacketPassTestCase.services)

    def test_packet_pass(self) :
        conf.iface6="A-0"

        ip6 = IPv6(src=self.sfc[-1], dst=self.sfc[0])
        sr  = IPv6ExtHdrSegmentRouting(addresses=self.sfc[::-1])
        hdr = (ip6/sr)

        def craft(hdr, fl):
            helpers.Utils.set_fl(hdr, fl)
            pkt = hdr/IPv6()/ICMPv6EchoRequest()

            pkt = pkt.__class__(str(pkt))

            return ([pkt], {fl: [pkt]})

        sns = tt.Tester(tt.Test(
            ct.Crafter(hdr, craft, 1, 1, 5),
            [sn.filter_ICMP, sn.filter_SEG],
            mt.Matcher())
        )
        self.assertTrue(sns.perform_test())

def suite():
    return unittest.TestLoader().loadTestsFromName(__name__)

if __name__ == b'__main__':
    unittest.main(defaultTest='suite')
