#!/usr/bin/env python

from route import *
import copy, random

def normalize(name):
	if len(name) > 12:
		return name[-12:]
	return name

class Node(object):
	def __init__(self, name):
		self.name = name
		self.cur_intf = 0
		self.intfs = []
		self.intfs_addr = {}
		self.addr = None

		self.routes = []

	def add_intf(self, intf):
		self.intfs.append(intf)

	def new_intf(self):
		i = self.cur_intf
		self.cur_intf += 1
		return i

	def __hash__(self):
		return self.name.__hash__()

	def __eq__(self, o):
		return self.name == o.name

class Edge(object):
	def __init__(self, node1, node2, port1, port2, cost, delay):
		self.node1 = node1
		self.node2 = node2
		self.port1 = port1
		self.port2 = port2
		self.cost = cost
		self.delay = delay

class Topo(object):
	def __init__(self):
		self.nodes = set()
		self.edges = list()
		self.dmin = 0
		self.dmax = 0

	def copy(self):
		t = Topo()
		t.nodes = copy.deepcopy(self.nodes)
		t.edges = copy.deepcopy(self.edges)

		for e in t.edges:
			e.node1 = t.get_node(e.node1.name)
			e.node2 = t.get_node(e.node2.name)

		return t

	def copy_unit(self):
		t = self.copy()

		for e in t.edges:
			e.cost = 1

		t.compute()
		return t

	def build(self):
		pass

	def add_node(self, name):
		n = Node(normalize(name))
		self.nodes.add(n)
		return n

	def get_node(self, name):
		for n in self.nodes:
			if n.name == normalize(name):
				return n

		return None

	def add_link(self, node1, node2, port1=None, port2=None, cost=1, delay=None):
		if port1 is None:
			port1 = node1.new_intf()
		if port2 is None:
			port2 = node2.new_intf()

		node1.add_intf(port1)
		node2.add_intf(port2)

		if delay is None:
			delay = random.uniform(self.dmin, self.dmax)

		e = Edge(node1, node2, port1, port2, int(cost), delay)
		self.edges.append(e)
		return e

	def add_link_name(self, name1, name2, *args, **kwargs):
		return self.add_link(self.get_node(name1), self.get_node(name2), *args, **kwargs)

	def get_edges(self, node1, node2):
		res = []

		for e in self.edges:
			if e.node1 == node1 and e.node2 == node2 or e.node1 == node2 and e.node2 == node1:
				res.append(e)

		return res

	def get_minimal_edge(self, node1, node2):
		edges = self.get_edges(node1, node2)
		cost = 2**32
		res = None

		for e in edges:
			if e.cost < cost:
				cost = e.cost
				res = e

		return res

	def get_neighbors(self, node1):
		res = set()

		for e in self.edges:
			if e.node1 == node1:
				res.add(e.node2)
			elif e.node2 == node1:
				res.add(e.node1)

		return res

	def set_default_delay(self, dmin, dmax):
		self.dmin = dmin
		self.dmax = dmax

#	def get_min_neighbors(self, n):
#		res = set()
#
#		mcost = 2**32
#		neighs = self.get_neighbors(n)
#
#		for neigh in neighs:
#			e = self.get_minimal_edge(n, neigh)
#			if e.cost < mcost:
#				mcost = e.cost
#
#		for neigh in neighs:
#			e = self.get_minimal_edge(n, neigh)
#			if e.cost == mcost:
#				

	def dijkstra(self, src):
		dist = {}
		prev = {}
		path = {}
		Q = set()

		dist[src] = 0
		prev[src] = None

		for v in self.nodes:
			if v != src:
				dist[v] = 2**32
				prev[v] = None
				path[v] = None
			Q.add(v)

		while len(Q) > 0:
			u = None
			tmpcost = 2**32
			for v in Q:
				if dist[v] < tmpcost:
					tmpcost = dist[v]
					u = v

			S = []
			w = u
			while prev[w] is not None:
				S.append(w)
				w = prev[w]
			path[u] = list(reversed(S))

			Q.remove(u)

			neighs = self.get_neighbors(u)
			for v in neighs:
				if v not in Q:
					continue
				alt = dist[u] + self.get_minimal_edge(u, v).cost
				if alt < dist[v]:
					dist[v] = alt
					prev[v] = u

		return dist, path

	def compute_node(self, n):
		n.routes = []
		dist, path = self.dijkstra(n)
		for t in dist.keys():
			if len(path[t]) == 0:
				continue
			e = self.get_minimal_edge(n, path[t][0])
			tmp = e.port1 if e.node1 == path[t][0] else e.port2
			r = Route(t.addr, path[t][0].intfs_addr[tmp].split("/")[0], dist[t])
			n.routes.append(r)

	def compute(self):
		cnt = 0
		for n in self.nodes:
			print '# Running dijkstra for node %s (%d/%d)' % (n.name, cnt+1, len(self.nodes))
			self.compute_node(n)
			cnt += 1
