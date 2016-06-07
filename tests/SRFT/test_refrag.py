#!/usr/bin/python2

from scapy.all import conf, IPv6, ICMPv6EchoRequest, fragment6, IPv6ExtHdrFragment, IPv6ExtHdrSegmentRouting

from bin.matcher import *
from bin.tester import Utils, Tester, Test
import bin.crafter as ct
import bin.sniffer as sn

import random

import helpers
import unittest


class DefragTestCase(unittest.TestCase):

    services = [helpers.DEFRAG, helpers.TEST, helpers.DECOMPRESSOR]

    def setUp(self):
        self.conf = helpers.get_conf()
        self.sfc = helpers.addr_services(self.conf, DefragTestCase.services)

    def test_refrag(self):
        nbrFlux = 4

        ip6 = IPv6(src=self.sfc[-1], dst=self.sfc[0])
        sr  = IPv6ExtHdrSegmentRouting(addresses=self.sfc[::-1],
                                nseg=len(self.sfc) - 1, fseg=len(self.sfc) - 1)
        hdr = (ip6/sr)

        def matching(a,b):
            return Utils.inner_IPv6(a) == Utils.inner_IPv6(b)

        def craft(hdr, fl):
            snd = []
            rcv = {}

            for fl in range(fl, fl+nbrFlux):
                Utils.set_fl(hdr, fl)

                sip = 'fc00:2:0:1::1' # Adresses don't rlly matter
                dip = 'fc00:2:0:2::1'

                packets_snd = fragment6(IPv6(src=sip, dst=dip) / IPv6ExtHdrFragment() / ICMPv6EchoRequest(data='A'*300), 100)
                packets_snd = [pkt.__class__(str(hdr/pkt)) for pkt in packets_snd]
                packet_excepted = pkt.__class__(str(hdr/IPv6(src=sip, dst=dip) / ICMPv6EchoRequest(data='A'*300)))

                snd.extend(packets_snd)
                rcv[fl] = [packet_excepted]

            return (snd, rcv)

        sns = Tester(Test(
            ct.Crafter(hdr, craft, nbr_flux=nbrFlux,
                       nbr_expected=nbrFlux),
            [sn.filter_ICMP, sn.filter_SEG],
            Matcher(matching)
        ))
        self.assertTrue(sns.perform_test())

    def test_uncomplete_refrag(self):
        nbrFlux = 4

        ip6 = IPv6(src=self.sfc[-1], dst=self.sfc[0])
        sr  = IPv6ExtHdrSegmentRouting(addresses=self.sfc[::-1],
                                nseg=len(self.sfc) - 1, fseg=len(self.sfc) - 1)
        hdr = (ip6/sr)

        def matching(a,b):
            return Utils.inner_IPv6(a) == Utils.inner_IPv6(b)

        def craft(hdr, fl):
            snd = []
            rcv = {}

            for fl in range(fl, fl+nbrFlux):
                Utils.set_fl(hdr, fl)

                sip = 'fc00:2:0:1::1' # Adresses don't rlly matter
                dip = 'fc00:2:0:2::1'

                packets_snd = fragment6(IPv6(src=sip, dst=dip) / IPv6ExtHdrFragment() / ICMPv6EchoRequest(data='A'*300), 100)
                packets_snd = [pkt.__class__(str(hdr/pkt)) for pkt in packets_snd[:-1]] # remove last frag

                snd.extend(packets_snd)
                rcv = {}

            return (snd, rcv)

        sns = Tester(Test(
            ct.Crafter(hdr, craft, nbr_flux=nbrFlux,
                       nbr_expected=nbrFlux),
            [sn.filter_ICMP, sn.filter_SEG],
            Matcher(matching)
        ))
        self.assertTrue(sns.perform_test())

    def test_disordered_refrag(self):
        nbrFlux = 4

        ip6 = IPv6(src=self.sfc[-1], dst=self.sfc[0])
        sr  = IPv6ExtHdrSegmentRouting(addresses=self.sfc[::-1],
                                nseg=len(self.sfc) - 1, fseg=len(self.sfc) - 1)
        hdr = (ip6/sr)

        def matching(a,b):
            return Utils.inner_IPv6(a) == Utils.inner_IPv6(b)

        def craft(hdr, fl):
            snd = []
            rcv = {}

            for fl in range(fl, fl+nbrFlux):
                Utils.set_fl(hdr, fl)

                sip = 'fc00:2:0:1::1' # Adresses don't rlly matter
                dip = 'fc00:2:0:2::1'

                packets_snd = fragment6(IPv6(src=sip, dst=dip) / IPv6ExtHdrFragment() / ICMPv6EchoRequest(data='A'*300), 100)
                packets_snd = [pkt.__class__(str(hdr/pkt)) for pkt in packets_snd]
                packet_excepted = pkt.__class__(str(hdr/IPv6(src=sip, dst=dip) / ICMPv6EchoRequest(data='A'*300)))

                snd.extend(packets_snd)
                random.shuffle(snd) # shuffle
                rcv[fl] = [packet_excepted]

            return (snd, rcv)

        sns = Tester(Test(
            ct.Crafter(hdr, craft, nbr_flux=nbrFlux,
                       nbr_expected=nbrFlux),
            [sn.filter_ICMP, sn.filter_SEG],
            Matcher(matching)
        ))
        self.assertTrue(sns.perform_test())

#    def test_inline_disordered_refrag(self):
#        nbrFlux = 1
#
#        sip = self.sfc[1] # Adresses matters here
#        dip = self.sfc[2]
#
#        ip = IPv6(src=sip, dst=dip)
#        sr  = IPv6ExtHdrSegmentRouting(addresses=self.sfc[::-1],
#                                nseg=len(self.sfc) - 1, fseg=len(self.sfc) - 1)
#        hdr = (ip/sr)
#
#
#        def matching(a,b):
#            return Utils.inner_IPv6(a) == Utils.inner_IPv6(b)
#
#        def craft(hdr, fl):
#            snd = []
#            rcv = {}
#
#            for fl in range(fl, fl+nbrFlux):
#                Utils.set_fl(hdr, fl)
#
#                packets_snd = fragment6( hdr / IPv6ExtHdrFragment() / ICMPv6EchoRequest(data='A'*200), 150)
#                packets_snd = [pkt.__class__(str(pkt)) for pkt in packets_snd]
#                packet_excepted = pkt.__class__(str( hdr / ICMPv6EchoRequest(data='A'*200)))
#
#                for i in packets_snd: i.show()
#                print(len(packets_snd))
#
#                snd.extend(packets_snd)
#                rcv[fl] = [packet_excepted]
#
#            return (snd, rcv)
#
#        sns = Tester(Test(
#            ct.Crafter(hdr, craft, nbr_flux=nbrFlux,
#                       nbr_expected=nbrFlux),
#            [sn.filter_ICMP, sn.filter_SEG],
#            Matcher(matching)
#        ))
#        self.assertTrue(sns.perform_test())


def suite():
    return unittest.TestLoader().loadTestsFromName(__name__)

if __name__ == b'__main__':
    unittest.main(defaultTest='suite')
    #unittest.main(defaultTest='DefragTestCase.test_inline_disordered_refrag')

