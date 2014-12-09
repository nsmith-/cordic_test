#include <memory>
#include <iostream>
#include <vector>
#include <cmath>
#include <random>
#include <atomic>
#include <thread>
#include <chrono>

#include "CordicXIP.h"
#include "CordicXilinx.h"

int main (int argc, char ** argv)
{
    // CordicXIP(int inputBits, int magBits, int phiBits);
    CordicXIP xilinx(24, 19, 19);
    // CordicXilinx(int inputBits, int outputBits, int phiScale);
    CordicXilinx nick(24, 19, 1<<19, argc > 2);
    
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


    if (argc == 3) {
        // Test x and y inputs
        int32_t phaseOut;
        uint32_t magOut;
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
    else if (argc == 2)
    {
        // Test phi LSB at a few radii
        int count = 0;
        int range = 205887;
        std::vector<int> radii({1, 23405, 840, 1200003, 123456, 8388608});
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
    else
    {
        // Test random input
        std::independent_bits_engine<std::mt19937, 24, uint32_t> rand(42);
        int runs(0), errors(0);
        std::atomic_bool run(true);
        std::thread keywatch([&run]()
        {
            std::cout << "Press q<Enter> to exit" << std::endl;
            char c;
            while(run)
            {
                std::cin >> c;
                if ( c == 'q' )
                {
                    run=false;
                    std::cout << "Stopping..." << std::endl;
                }
            }
        });
        while ( run )
        {
            runs++;
            // Uniform in [-2^23, 2^23)
            int x = rand()-(1<<23);
            int y = rand()-(1<<23);
            if ( !compare(x,y) )
            {
                errors++;
                std::cout << "err x,y: " << x << " " << y << std::endl;
            }
            
            if ( runs % 10000 == 0 ) printf("\rErrors: % 8d / % 8d  (% 2.2f\%)", errors, runs, errors*100. / runs);
            std::cout.flush();
        }
        keywatch.join();
    }
}
