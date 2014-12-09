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
        int x = atoi(argv[1]);
        int y = atoi(argv[2]);
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
        int range = 205887;
        auto compare = [&xilinx,&nick](int x, int y) -> bool
        {
            int phase1;
            unsigned mag1;
            nick(x, y, phase1, mag1);
            int phase2;
            unsigned mag2;
            xilinx(x, y, phase2, mag2);
            if ( mag1 != mag2 || phase1 != phase2 ) return false;
            return true;
        };
        std::vector<int> radii({1, 23405, 840, 12, 123456});
        for(int r : radii)
        {
            for(int i=-range; i<=range; ++i)
            {
                int a = r*cos(i*M_PI/range);
                int b = r*sin(i*M_PI/range);
                if ( !compare(a, b) )
                {
                    count++;
                }
            }
        }
        std::cout << "Bad angle count: " << count << " of " << (2*range+1)*radii.size() << std::endl;
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
