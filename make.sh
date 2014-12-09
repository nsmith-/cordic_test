#!/bin/bash
g++ -std=c++0x -O2 -I. -I$CMSSW_BASE/src test.cc CordicXIP.cc CordicXilinx.cc ../src/firmware/Cordic.cc -L. -lgmp -lIp_cordic_v6_0_bitacc_cmodel -o test
