# A loop: fc00:2:0:1::1/64
# C loop: fc00:2:0:2::1/64
# B loop: fc00:2:0:3::1/64
# D loop: fc00:2:0:4::1/64

term 1
======

ip netns exec A bash
ping6 fc00:2:0:2::1 -I fc00:2:0:1::1

term 2
======

ip netns exec B bash
./seg6ctl -p fc00:2:0:2::1/64 -a fc00:2:0:4::1

term 3
======

ip netns exec D bash
./count fc00:2:0:4::1

