#!/usr/bin/python2

import bin.tester as tt

class Crafter(object):
    def __init__(self, hdr, fct, nfl, nexp, nflux=1):
        self.hdr   = hdr
        self.fct   = fct
        self.nfl   = nfl
        self.nexp  = nexp
        self.nflux = nflux

    def nfl_total(self):
        return self.nfl * self.nflux

    def nexp_total(self):
        return self.nexp * self.nflux

    def craft(self, fl):
        return self.fct(self.hdr, fl)

    def craftes(self, fl):
        craftes = [ self.craft(fl+i*self.nfl) for i in range(self.nflux)]

        snd = []
        exp = {}

        for tsnd, texp in craftes:
            snd.extend(tsnd)
            exp.update(texp)

#        return reduce(lambda x, y: (x[0].extend(y[0]), x[1].update(y[1])),
#                      craftes)

        return (snd, exp)


class NoCrafter(Crafter):
    def __init__(self, hdr, to_snd, to_rcv, nbr_flux=1):
        super(NoCrafter, self).__init__(hdr, nbr_flux)
        self.snd = to_snd
        self.rcv = to_rcv

    def crafts(self, fl):
        tt.Utils.set_fl(self.snd)
        tt.Utils.set_fl(self.rcv)

        self.snd = self.snd.__class__(str(self.snd))
        self.rcv = self.rcv.__class__(str(self.rcv))

        return (self.snd, self.rcv)
