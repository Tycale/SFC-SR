# A loop: fc00:2:0:1::1/64
# C loop: fc00:2:0:2::1/64
# B loop: fc00:2:0:3::1/64
# E loop: fc00:2:0:4::1/64
# D loop: fc00:2:0:5::1/64
# G loop: fc00:2:0:6::1/64
# F loop: fc00:2:0:7::1/64
# H loop: fc00:2:0:8::1/64

ip netns add A
ip netns add C
ip netns add B
ip netns add E
ip netns add D
ip netns add G
ip netns add F
ip netns add H
ip link add name A-0 type veth peer name B-0
ip link set A-0 netns A
ip link set B-0 netns B
ip link add name B-1 type veth peer name C-0
ip link set B-1 netns B
ip link set C-0 netns C
ip link add name C-1 type veth peer name D-0
ip link set C-1 netns C
ip link set D-0 netns D
ip link add name D-1 type veth peer name E-0
ip link set D-1 netns D
ip link set E-0 netns E
ip link add name E-1 type veth peer name F-0
ip link set E-1 netns E
ip link set F-0 netns F
ip link add name F-1 type veth peer name G-0
ip link set F-1 netns F
ip link set G-0 netns G
ip link add name G-1 type veth peer name H-0
ip link set G-1 netns G
ip link set H-0 netns H
ip netns exec A bash -c 'ifconfig lo up; ip -6 ad ad fc00:2:0:1::1/64 dev lo; sysctl net.ipv6.conf.all.forwarding=1; ifconfig A-0 add fc00:42:0:1::1/64 up; tc qdisc add dev A-0 root handle 1: netem delay 1.00ms; ip -6 ro ad fc00:2:0:2::1/64 via fc00:42:0:1::2 metric 2; ip -6 ro ad fc00:2:0:3::1/64 via fc00:42:0:1::2 metric 1; ip -6 ro ad fc00:2:0:4::1/64 via fc00:42:0:1::2 metric 4; ip -6 ro ad fc00:2:0:5::1/64 via fc00:42:0:1::2 metric 3; ip -6 ro ad fc00:2:0:6::1/64 via fc00:42:0:1::2 metric 6; ip -6 ro ad fc00:2:0:7::1/64 via fc00:42:0:1::2 metric 5; ip -6 ro ad fc00:2:0:8::1/64 via fc00:42:0:1::2 metric 7'
ip netns exec C bash -c 'ifconfig lo up; ip -6 ad ad fc00:2:0:2::1/64 dev lo; sysctl net.ipv6.conf.all.forwarding=1; ifconfig C-0 add fc00:42:0:2::2/64 up; tc qdisc add dev C-0 root handle 1: netem delay 1.00ms; ifconfig C-1 add fc00:42:0:3::1/64 up; tc qdisc add dev C-1 root handle 1: netem delay 1.00ms; ip -6 ro ad fc00:2:0:1::1/64 via fc00:42:0:2::1 metric 2; ip -6 ro ad fc00:2:0:3::1/64 via fc00:42:0:2::1 metric 1; ip -6 ro ad fc00:2:0:4::1/64 via fc00:42:0:3::2 metric 2; ip -6 ro ad fc00:2:0:5::1/64 via fc00:42:0:3::2 metric 1; ip -6 ro ad fc00:2:0:6::1/64 via fc00:42:0:3::2 metric 4; ip -6 ro ad fc00:2:0:7::1/64 via fc00:42:0:3::2 metric 3; ip -6 ro ad fc00:2:0:8::1/64 via fc00:42:0:3::2 metric 5'
ip netns exec B bash -c 'ifconfig lo up; ip -6 ad ad fc00:2:0:3::1/64 dev lo; sysctl net.ipv6.conf.all.forwarding=1; ifconfig B-0 add fc00:42:0:1::2/64 up; tc qdisc add dev B-0 root handle 1: netem delay 1.00ms; ifconfig B-1 add fc00:42:0:2::1/64 up; tc qdisc add dev B-1 root handle 1: netem delay 1.00ms; ip -6 ro ad fc00:2:0:1::1/64 via fc00:42:0:1::1 metric 1; ip -6 ro ad fc00:2:0:2::1/64 via fc00:42:0:2::2 metric 1; ip -6 ro ad fc00:2:0:4::1/64 via fc00:42:0:2::2 metric 3; ip -6 ro ad fc00:2:0:5::1/64 via fc00:42:0:2::2 metric 2; ip -6 ro ad fc00:2:0:6::1/64 via fc00:42:0:2::2 metric 5; ip -6 ro ad fc00:2:0:7::1/64 via fc00:42:0:2::2 metric 4; ip -6 ro ad fc00:2:0:8::1/64 via fc00:42:0:2::2 metric 6'
ip netns exec E bash -c 'ifconfig lo up; ip -6 ad ad fc00:2:0:4::1/64 dev lo; sysctl net.ipv6.conf.all.forwarding=1; ifconfig E-0 add fc00:42:0:4::2/64 up; tc qdisc add dev E-0 root handle 1: netem delay 1.00ms; ifconfig E-1 add fc00:42:0:5::1/64 up; tc qdisc add dev E-1 root handle 1: netem delay 1.00ms; ip -6 ro ad fc00:2:0:1::1/64 via fc00:42:0:4::1 metric 4; ip -6 ro ad fc00:2:0:2::1/64 via fc00:42:0:4::1 metric 2; ip -6 ro ad fc00:2:0:3::1/64 via fc00:42:0:4::1 metric 3; ip -6 ro ad fc00:2:0:5::1/64 via fc00:42:0:4::1 metric 1; ip -6 ro ad fc00:2:0:6::1/64 via fc00:42:0:5::2 metric 2; ip -6 ro ad fc00:2:0:7::1/64 via fc00:42:0:5::2 metric 1; ip -6 ro ad fc00:2:0:8::1/64 via fc00:42:0:5::2 metric 3'
ip netns exec D bash -c 'ifconfig lo up; ip -6 ad ad fc00:2:0:5::1/64 dev lo; sysctl net.ipv6.conf.all.forwarding=1; ifconfig D-0 add fc00:42:0:3::2/64 up; tc qdisc add dev D-0 root handle 1: netem delay 1.00ms; ifconfig D-1 add fc00:42:0:4::1/64 up; tc qdisc add dev D-1 root handle 1: netem delay 1.00ms; ip -6 ro ad fc00:2:0:1::1/64 via fc00:42:0:3::1 metric 3; ip -6 ro ad fc00:2:0:2::1/64 via fc00:42:0:3::1 metric 1; ip -6 ro ad fc00:2:0:3::1/64 via fc00:42:0:3::1 metric 2; ip -6 ro ad fc00:2:0:4::1/64 via fc00:42:0:4::2 metric 1; ip -6 ro ad fc00:2:0:6::1/64 via fc00:42:0:4::2 metric 3; ip -6 ro ad fc00:2:0:7::1/64 via fc00:42:0:4::2 metric 2; ip -6 ro ad fc00:2:0:8::1/64 via fc00:42:0:4::2 metric 4'
ip netns exec G bash -c 'ifconfig lo up; ip -6 ad ad fc00:2:0:6::1/64 dev lo; sysctl net.ipv6.conf.all.forwarding=1; ifconfig G-0 add fc00:42:0:6::2/64 up; tc qdisc add dev G-0 root handle 1: netem delay 1.00ms; ifconfig G-1 add fc00:42:0:7::1/64 up; tc qdisc add dev G-1 root handle 1: netem delay 1.00ms; ip -6 ro ad fc00:2:0:1::1/64 via fc00:42:0:6::1 metric 6; ip -6 ro ad fc00:2:0:2::1/64 via fc00:42:0:6::1 metric 4; ip -6 ro ad fc00:2:0:3::1/64 via fc00:42:0:6::1 metric 5; ip -6 ro ad fc00:2:0:4::1/64 via fc00:42:0:6::1 metric 2; ip -6 ro ad fc00:2:0:5::1/64 via fc00:42:0:6::1 metric 3; ip -6 ro ad fc00:2:0:7::1/64 via fc00:42:0:6::1 metric 1; ip -6 ro ad fc00:2:0:8::1/64 via fc00:42:0:7::2 metric 1'
ip netns exec F bash -c 'ifconfig lo up; ip -6 ad ad fc00:2:0:7::1/64 dev lo; sysctl net.ipv6.conf.all.forwarding=1; ifconfig F-0 add fc00:42:0:5::2/64 up; tc qdisc add dev F-0 root handle 1: netem delay 1.00ms; ifconfig F-1 add fc00:42:0:6::1/64 up; tc qdisc add dev F-1 root handle 1: netem delay 1.00ms; ip -6 ro ad fc00:2:0:1::1/64 via fc00:42:0:5::1 metric 5; ip -6 ro ad fc00:2:0:2::1/64 via fc00:42:0:5::1 metric 3; ip -6 ro ad fc00:2:0:3::1/64 via fc00:42:0:5::1 metric 4; ip -6 ro ad fc00:2:0:4::1/64 via fc00:42:0:5::1 metric 1; ip -6 ro ad fc00:2:0:5::1/64 via fc00:42:0:5::1 metric 2; ip -6 ro ad fc00:2:0:6::1/64 via fc00:42:0:6::2 metric 1; ip -6 ro ad fc00:2:0:8::1/64 via fc00:42:0:6::2 metric 2'
ip netns exec H bash -c 'ifconfig lo up; ip -6 ad ad fc00:2:0:8::1/64 dev lo; sysctl net.ipv6.conf.all.forwarding=1; ifconfig H-0 add fc00:42:0:7::2/64 up; tc qdisc add dev H-0 root handle 1: netem delay 1.00ms; ip -6 ro ad fc00:2:0:1::1/64 via fc00:42:0:7::1 metric 7; ip -6 ro ad fc00:2:0:2::1/64 via fc00:42:0:7::1 metric 5; ip -6 ro ad fc00:2:0:3::1/64 via fc00:42:0:7::1 metric 6; ip -6 ro ad fc00:2:0:4::1/64 via fc00:42:0:7::1 metric 3; ip -6 ro ad fc00:2:0:5::1/64 via fc00:42:0:7::1 metric 4; ip -6 ro ad fc00:2:0:6::1/64 via fc00:42:0:7::1 metric 1; ip -6 ro ad fc00:2:0:7::1/64 via fc00:42:0:7::1 metric 2'
