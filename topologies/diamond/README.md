
Schema
======

  ------ B(3) ----- 
 /        |        \
A(1) --- C(2) --- D(5)
 \        |        /
  ------ E(4) ----- 


Loopback addresses
==================

* A loop: fc00:2:0:1::1/64
* C loop: fc00:2:0:2::1/64
* B loop: fc00:2:0:3::1/64
* E loop: fc00:2:0:4::1/64
* D loop: fc00:2:0:5::1/64

Example of usage 
================

Term 1
------

$ ip netns exec A bash
$ ./seg6ctl -p fc00:2:0:5::1/64 -a fc00:2:0:3::1,fc00:2:0:2::1,fc00:2:0:4::1
$ ping6 fc00:2:0:5::1

Term 3
------

$ ip netns exec B bash
$ ./count fc00:2:0:3::1




