#!/usr/bin/python2

import helpers

def default_matching(a, b):
    return helpers.Utils.inner_IPv6(a) == helpers.Utils.inner_IPv6(b)

class Matcher(object):

    def __init__(self, fct=default_matching):
        self._match_fct=fct

    def _match(self, rcv, exp):
        return self._match_fct(rcv, exp)

    def _matches(self, rcv_list, exp_list):
        if(len(rcv_list) != len(exp_list)):
            print("List of captured packets is not equal to the excepted list of packets :")
            print("Expected {} packet(s), got {} packet(s)".format(len(exp_list), len(rcv_list)))
            return False

        for rcv in rcv_list :
            n =  next((exp for exp in exp_list if self._match(rcv, exp)), None)

            if(n != None): exp_list.remove(n)
            else: return False

        return True

    def perform_match(self, rcv, exp):
        if(not set(rcv.keys()) == set(exp.keys())):
            print("rcv keys not equal to the exp keys :")
            print(rcv.keys())
            print(exp.keys())
            return False

        match = [self._matches(rcv.get(key), exp.get(key))
                 for key in rcv.keys()]
        print("Results of matching by flow :")
        print(match)

        return all(match)
