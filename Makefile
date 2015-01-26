CC=g++
FLAGS=-std=c++11 -O2
LDFLAGS=-L. -lgmp -lIp_cordic_v6_0_bitacc_cmodel

.PHONY: clean

all: met test readDat

met: met.cc CordicXilinx.cc fuzzer.h
	$(CC) $(FLAGS) $(filter %.cc, $^) -o $@

test: test.cc CordicXilinx.cc CordicXIP.cc fuzzer.h
	$(CC) $(FLAGS) $(filter %.cc, $^) $(LDFLAGS) -o $@

readDat: readDat.cc CordicXilinx.cc CordicXIP.cc
	$(CC) $(FLAGS) $^ $(LDFLAGS) -o $@

clean:
	rm -f met test readDat
