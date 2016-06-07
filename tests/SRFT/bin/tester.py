#!/usr/bin/python2

from scapy.all import IPv6, conf, send
from threading import Thread
from functools import partial

import bin.sniffer as sn

import time
import helpers


class Test(object):
    fl = 1

    def __init__(self, crafter, sniffers, matcher):
        self.fl = Test.fl
        Test.fl += crafter.nfl_total()
        sniffers.append(partial(sn.filter_FL, range(self.fl, Test.fl)))

        self.snd = {}
        self.rcv = {}
        self.exp = {}

        self.crafter = crafter
        self.sniffer = sn.Sniffer(self, sniffers, crafter.nexp_total())
        self.matcher = matcher

    def add_to_snd(self, pkt) : self.add_to(self.snd, pkt)
    def add_to_rcv(self, pkt) : self.add_to(self.rcv, pkt)
    def add_to(self, dic, pkt): dic.setdefault(helpers.Utils.key(pkt), []).append(pkt)

    def crafts(self):
        res = self.crafter.craftes(self.fl)
        self.exp = res[1]
        return res[0]

    def sniff(self):
        return self.sniffer.sniff()

    def perform_match(self):
        return self.matcher.perform_match(self.rcv, self.exp)


class Tester():

    def __init__(self, inst):
        self.inst = inst
        self.thread = Thread(target=self.inst.sniff, args=[])

    def perform_test(self):
        self.thread.start()
        # FIXME
        time.sleep(1)
        self.send_pkts()
        self.thread.join()

        # Check result
        return self.inst.perform_match()

    def send_pkts(self):
        conf.iface6=helpers.INTERFACE


        for pkt in self.inst.crafts() :
            send(pkt, verbose=True)
            self.inst.add_to_snd(pkt)
