#!/usr/bin/env python

from node import *
from net import *

class simple_lag(Topo):
	def build(self):
		self.add_node("A")
		self.add_node("B")
		self.add_node("C")
		self.add_node("D")
		self.add_node("E")
		self.add_node("F")
		self.add_node("G")
		self.add_node("H")
		self.add_link_name("A", "B", cost=1, delay=1)
		self.add_link_name("B", "C", cost=1, delay=1)
		self.add_link_name("C", "D", cost=1, delay=1)
		self.add_link_name("D", "E", cost=1, delay=1)
		self.add_link_name("E", "F", cost=1, delay=1)
		self.add_link_name("F", "G", cost=1, delay=1)
		self.add_link_name("G", "H", cost=1, delay=1)
topos = { 'simple_lag': (lambda: simple_lag()) }
