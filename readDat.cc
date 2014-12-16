#include <iostream>
#include <fstream>
#include <vector>

#include "CordicXIP.h"
#include "CordicXilinx.h"

int main (int argc, char ** argv)
{
    // Latency (clock cycles) of cordic alg.
    // can be tweaked by specifying an extra argument
    int latency = 27;

    // CordicXIP(int inputBits, int magBits, int phiBits);
    CordicXIP xilinx(24, 19, 19);
    // CordicXilinx(int inputBits, int outputBits, int phiScale, bool debug);
    CordicXilinx nick(24, 19, 1<<19, false);
 
    std::string fname;
    if ( argc >= 2 )
    {
        fname = std::string(argv[1]);
        if ( argc == 3 ) latency = std::stoi(argv[2]);
    }
    else
    {
        return 1;
    }

    std::ifstream dat(fname);

    std::vector<int> x_inputs, y_inputs, mag_outputs, phase_outputs;
    int clock_cycles(0);
    for ( std::string line; std::getline(dat, line); ++clock_cycles)
    {
        uint32_t xraw, yraw, magraw, phaseraw;
        sscanf(line.c_str(), "%6X%6X%5X%5X", &xraw, &yraw, &magraw, &phaseraw);
        // Fix the alignment
        int xin = xraw << 2 | yraw>>22;
        int yin = (yraw << 2 | magraw>>18) & 0xFFFFFF;
        uint32_t mag = (magraw<<1 | phaseraw>>19) & 0x7FFFF;
        int phase = phaseraw & 0x7FFFF;
        if ( xin > 1<<23 ) xin -= 1<<24;
        if ( yin > 1<<23 ) yin -= 1<<24;
        if ( phase > 1<<18 ) phase -= 1<<19;
        x_inputs.push_back(xin);
        y_inputs.push_back(yin);
        if ( clock_cycles >= latency )
        {
            mag_outputs.push_back(mag);
            phase_outputs.push_back(phase);
        }
    }

    int nerrors(0), xerrors(0);
    for ( size_t i=0; i<mag_outputs.size(); ++i)
    {
        int xin(x_inputs[i]);
        int yin(y_inputs[i]);
        int mag(mag_outputs[i]);
        int phase(phase_outputs[i]);

        uint32_t nmag, xmag;
        int nphase, xphase;
        xilinx(xin, yin, xphase, xmag);
        nick(xin, yin, nphase, nmag);
        std::cout <<
            "Input: " << xin << ", " << yin << "; Expected: " << mag << ", " << phase << std::endl;
        if ( xmag-mag != 0 || xphase - phase != 0 ) xerrors++;
        if ( nmag-mag != 0 || nphase - phase != 0 ) nerrors++;
    }
    std::cout << "XIP errors: " << xerrors << ", emulator errors: " << nerrors << std::endl;
}

