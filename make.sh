#!/bin/bash
g++ -std=c++0x -O2 -fprofile-arcs -ftest-coverage -pg test.cc CordicXIP.cc CordicXilinx.cc -L. -lgmp -lIp_cordic_v6_0_bitacc_cmodel -o test
