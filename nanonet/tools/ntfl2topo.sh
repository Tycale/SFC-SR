#!/bin/bash

cat <<EOF
#!/usr/bin/env python

from node import *
from net import *

class $2(Topo):
	def build(self):
EOF

nodes=$(cat $1 | sed -E 's/\S+,\+//g' | cut -d " " -f 1,2 | sed -E 's/ /\n/' | sort -u)

for n in $nodes; do
	echo -e "\t\tself.add_node(\"$n\")"
done

for n in $(cat $1 | sed -E 's/\S+,\+//g' | sed -e 's/ /;/g'); do
	n1=$(echo $n | cut -d ";" -f 1)
	n2=$(echo $n | cut -d ";" -f 2)
	w=$(echo $n | cut -d ";" -f 3)
	d=$(echo $n | cut -d ";" -f 4)
	echo -e "\t\tself.add_link_name(\"$n1\", \"$n2\", cost=$w, delay=$d)"
done

echo "topos = { '$2': (lambda: $2()) }"
