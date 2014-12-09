#include <memory>
#include <iostream>
#include <vector>
#include <cmath>

#include "../interface/Cordic.h"
#include "CordicXIP.h"
#include "CordicXilinx.h"

int main (int argc, char ** argv)
{
    // CordicXIP(int inputBits, int magBits, int phiBits);
    CordicXIP xilinx(24, 19, 19);
    // CordicXilinx(int inputBits, int outputBits, int phiScale);
    CordicXilinx nick(24, 19, 1<<19, argc > 1);
    // Cordic( const uint32_t& aPhiScale , const uint32_t& aMagnitudeBits , const uint32_t& aSteps );
    Cordic andy(14, 24, 26);

    int32_t phaseOut;
    uint32_t magOut;
    if (argc == 3) {
        int x = atoi(argv[1])*32;
        int y = atoi(argv[2])*32;
        nick(x, y, phaseOut, magOut);
        std::cout << "me  mag = " << magOut << ", phase = " << phaseOut << std::endl;
        xilinx(x, y, phaseOut, magOut);
        std::cout << "XIP mag = " << magOut << ", phase = " << phaseOut << std::endl;
        magOut = sqrt(x*1.*x+y*1.*y)/32.;
        phaseOut = pow(2.,16)*atan2(y*1.,x*1.);
        std::cout << "FPU mag = " << magOut << ", phase = " << phaseOut << std::endl;
    }
    else
    {
        int count = 0;
        for(int i=-4000; i<=4000; ++i)
        {
            int a = pow(2.43, 14)*cos(i*M_PI/4000);
            int b = pow(2.43, 14)*sin(i*M_PI/4000);
            nick(a, b, phaseOut, magOut);
            int phase1=phaseOut;
            int mag1=magOut;
            xilinx(a, b, phaseOut, magOut);
            if ( mag1 != magOut || phase1 != phaseOut )
            {
                std::cout << "in x: " << a << " y: " << b << std::endl;
                std::cout << "me  mag = " << mag1 << ", phase = " << phase1 << std::endl;
                std::cout << "XIP mag = " << magOut << ", phase = " << phaseOut << std::endl;
                count++;
            }
        }
        std::cout << "Bad angle count: " << count << std::endl;
    }

    for(int i=40; i<40; ++i)
    {
        int a = pow(2., 9)*cos(i*M_PI/20);
        int b = pow(2., 9)*sin(i*M_PI/20);
        std::cout << "a = " << a << " b = " << b << std::endl;
        xilinx(a, b, phaseOut, magOut);
        int andyPhase = round( 14*36.*phaseOut/pow(2.,16)/M_PI );
        if ( andyPhase < 0 ) andyPhase += 2*504;
        std::cout << "xilinx mag = " << magOut << " phase = " << andyPhase << " real phi = " << phaseOut/pow(2.,16) << std::endl;
        andy(a, b, phaseOut, magOut);
        double realPhase = phaseOut*M_PI/14/36;
        if ( realPhase > M_PI ) realPhase -= 2*M_PI;
        std::cout << "andy mag = " << (magOut>>5) << " phase = " << phaseOut << " real phi = " << realPhase << std::endl;
    }
}
