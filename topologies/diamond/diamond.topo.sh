# A loop: fc00:2:0:1::1/64
# C loop: fc00:2:0:2::1/64
# B loop: fc00:2:0:3::1/64
# E loop: fc00:2:0:4::1/64
# D loop: fc00:2:0:5::1/64

ip netns add A
ip netns add C
ip netns add B
ip netns add E
ip netns add D
ip link add name A-0 type veth peer name B-0
ip link set A-0 netns A
ip link set B-0 netns B
ip link add name A-1 type veth peer name C-0
ip link set A-1 netns A
ip link set C-0 netns C
ip link add name A-2 type veth peer name E-0
ip link set A-2 netns A
ip link set E-0 netns E
ip link add name A-3 type veth peer name D-0
ip link set A-3 netns A
ip link set D-0 netns D
ip link add name B-1 type veth peer name D-1
ip link set B-1 netns B
ip link set D-1 netns D
ip link add name B-2 type veth peer name C-1
ip link set B-2 netns B
ip link set C-1 netns C
ip link add name C-2 type veth peer name D-2
ip link set C-2 netns C
ip link set D-2 netns D
ip link add name C-3 type veth peer name E-1
ip link set C-3 netns C
ip link set E-1 netns E
ip link add name D-3 type veth peer name E-2
ip link set D-3 netns D
ip link set E-2 netns E
ip netns exec A bash -c 'ifconfig lo up; ip -6 ad ad fc00:2:0:1::1/64 dev lo; sysctl net.ipv6.conf.all.forwarding=1; ifconfig A-0 add fc00:42:0:1::1/64 up; tc qdisc add dev A-0 root handle 1: netem delay 1.00ms; ifconfig A-1 add fc00:42:0:2::1/64 up; tc qdisc add dev A-1 root handle 1: netem delay 1.00ms; ifconfig A-2 add fc00:42:0:3::1/64 up; tc qdisc add dev A-2 root handle 1: netem delay 1.00ms; ifconfig A-3 add fc00:42:0:4::1/64 up; tc qdisc add dev A-3 root handle 1: netem delay 1.00ms; ip -6 ro ad fc00:2:0:2::1/64 via fc00:42:0:2::2 metric 1; ip -6 ro ad fc00:2:0:3::1/64 via fc00:42:0:1::2 metric 1; ip -6 ro ad fc00:2:0:4::1/64 via fc00:42:0:3::2 metric 1; ip -6 ro ad fc00:2:0:5::1/64 via fc00:42:0:4::2 metric 1'
ip netns exec C bash -c 'ifconfig lo up; ip -6 ad ad fc00:2:0:2::1/64 dev lo; sysctl net.ipv6.conf.all.forwarding=1; ifconfig C-0 add fc00:42:0:2::2/64 up; tc qdisc add dev C-0 root handle 1: netem delay 1.00ms; ifconfig C-1 add fc00:42:0:6::2/64 up; tc qdisc add dev C-1 root handle 1: netem delay 1.00ms; ifconfig C-2 add fc00:42:0:7::1/64 up; tc qdisc add dev C-2 root handle 1: netem delay 1.00ms; ifconfig C-3 add fc00:42:0:8::1/64 up; tc qdisc add dev C-3 root handle 1: netem delay 1.00ms; ip -6 ro ad fc00:2:0:1::1/64 via fc00:42:0:2::1 metric 1; ip -6 ro ad fc00:2:0:3::1/64 via fc00:42:0:6::1 metric 1; ip -6 ro ad fc00:2:0:4::1/64 via fc00:42:0:8::2 metric 1; ip -6 ro ad fc00:2:0:5::1/64 via fc00:42:0:7::2 metric 1'
ip netns exec B bash -c 'ifconfig lo up; ip -6 ad ad fc00:2:0:3::1/64 dev lo; sysctl net.ipv6.conf.all.forwarding=1; ifconfig B-0 add fc00:42:0:1::2/64 up; tc qdisc add dev B-0 root handle 1: netem delay 1.00ms; ifconfig B-1 add fc00:42:0:5::1/64 up; tc qdisc add dev B-1 root handle 1: netem delay 1.00ms; ifconfig B-2 add fc00:42:0:6::1/64 up; tc qdisc add dev B-2 root handle 1: netem delay 1.00ms; ip -6 ro ad fc00:2:0:1::1/64 via fc00:42:0:1::1 metric 1; ip -6 ro ad fc00:2:0:2::1/64 via fc00:42:0:6::2 metric 1; ip -6 ro ad fc00:2:0:4::1/64 via fc00:42:0:1::1 metric 2; ip -6 ro ad fc00:2:0:5::1/64 via fc00:42:0:5::2 metric 1'
ip netns exec E bash -c 'ifconfig lo up; ip -6 ad ad fc00:2:0:4::1/64 dev lo; sysctl net.ipv6.conf.all.forwarding=1; ifconfig E-0 add fc00:42:0:3::2/64 up; tc qdisc add dev E-0 root handle 1: netem delay 1.00ms; ifconfig E-1 add fc00:42:0:8::2/64 up; tc qdisc add dev E-1 root handle 1: netem delay 1.00ms; ifconfig E-2 add fc00:42:0:9::2/64 up; tc qdisc add dev E-2 root handle 1: netem delay 1.00ms; ip -6 ro ad fc00:2:0:1::1/64 via fc00:42:0:3::1 metric 1; ip -6 ro ad fc00:2:0:2::1/64 via fc00:42:0:8::1 metric 1; ip -6 ro ad fc00:2:0:3::1/64 via fc00:42:0:3::1 metric 2; ip -6 ro ad fc00:2:0:5::1/64 via fc00:42:0:9::1 metric 1'
ip netns exec D bash -c 'ifconfig lo up; ip -6 ad ad fc00:2:0:5::1/64 dev lo; sysctl net.ipv6.conf.all.forwarding=1; ifconfig D-0 add fc00:42:0:4::2/64 up; tc qdisc add dev D-0 root handle 1: netem delay 1.00ms; ifconfig D-1 add fc00:42:0:5::2/64 up; tc qdisc add dev D-1 root handle 1: netem delay 1.00ms; ifconfig D-2 add fc00:42:0:7::2/64 up; tc qdisc add dev D-2 root handle 1: netem delay 1.00ms; ifconfig D-3 add fc00:42:0:9::1/64 up; tc qdisc add dev D-3 root handle 1: netem delay 1.00ms; ip -6 ro ad fc00:2:0:1::1/64 via fc00:42:0:4::1 metric 1; ip -6 ro ad fc00:2:0:2::1/64 via fc00:42:0:7::1 metric 1; ip -6 ro ad fc00:2:0:3::1/64 via fc00:42:0:5::1 metric 1; ip -6 ro ad fc00:2:0:4::1/64 via fc00:42:0:9::2 metric 1'
