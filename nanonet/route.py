#!/usr/bin/env python

class Route(object):
	def __init__(self, dst, nh, cost):
		self.dst = dst
		self.nh = nh
		self.cost = cost

	def __hash__(self):
		return self.dst.__hash__()

	def __eq__(self, o):
		return self.dst == o.dst

	def __str__(self):
		return '%s via %s metric %d' % (self.dst, self.nh, self.cost)
