CC=g++
FLAGS=-std=c++11 -O2
LDFLAGS=-L. -lgmp -lIp_cordic_v6_0_bitacc_cmodel

all: met test readDat

met: met.cc CordicXilinx.cc
	$(CC) $(FLAGS) $^ -o $@

test: test.cc CordicXilinx.cc CordicXIP.cc
	$(CC) $(FLAGS) $^ $(LDFLAGS) -o $@

readDat: readDat.cc CordicXilinx.cc CordicXIP.cc
	$(CC) $(FLAGS) $^ $(LDFLAGS) -o $@
