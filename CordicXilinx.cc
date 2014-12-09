//#include "L1Trigger/L1TCalorimeter/interface/CordicXilinx.h"
#include "CordicXilinx.h"

#include <vector>
#include <iostream>
#include <iomanip> 
#include <cassert>
#include <math.h>

CordicXilinx::CordicXilinx(int inputBits, int outputBits, int phiScale, bool debug) :
    inputBits_(inputBits),
    outputBits_(outputBits),
    phiScale_(phiScale),
    debug_(debug)
{
    // Coarse rotation lowers necessary iterations by 2
    iterations_ = outputBits-2;
    // Internal precision is by default this value (when set to 0 in xilinx config)
    internalBits_ = outputBits+ceil(log((float) iterations_)/log(2.));

    double scaleFactor = 1.;
    for(int i=1; i<=iterations_; ++i)
    {
        int rotation = encodeAngle(atan(pow(2.,-i)));
        rotations_.push_back(rotation);
        scaleFactor *= pow(1+pow(2.,-2*i), -0.5);
    }
    scaleFactor_ = scaleFactor*pow(2., internalBits_);

    printf("Cordic setup: %d iterations, %d internal bits, scale factor = %d\n", iterations_, internalBits_, scaleFactor_);
}

CordicXilinx::~CordicXilinx()
{
}

int CordicXilinx::encodeAngle(const double angleFloat)
{
    assert(abs(angleFloat)<=M_PI);
    // Xilinx seems to store rounded rotation table
    return angleFloat*pow(2., internalBits_-3)+0.5;
}

void CordicXilinx::operator() ( int32_t aX , int32_t aY , int32_t& aPhi , uint32_t& aMagnitude )
{
    // Rotation to get from current vector to origin
    // must invert to get aPhi
    int rotation(0);

    // Convert to internal precision
    int x,y;
    if ( internalBits_ > inputBits_ )
    {
        x = (abs(aX) << (internalBits_-inputBits_)) * ((aX>0) ? 1:-1);
        y = (abs(aY) << (internalBits_-inputBits_)) * ((aY>0) ? 1:-1);
    }
    else
    {
        x = (abs(aX) >> (inputBits_-internalBits_)) * ((aX>0) ? 1:-1);
        y = (abs(aY) >> (inputBits_-internalBits_)) * ((aY>0) ? 1:-1);
    }
    auto printVals = [&x,&y,&rotation,this]{printf("x: % 8d y: % 8d phi: % 8d outphi: % 8d float phi = % f\n", x, y, rotation, rotation/pow(2., internalBits_-3), (abs(rotation)>>(internalBits_-outputBits_)) * ((rotation>0) ? -1:1) );};
    if ( debug_ ) printVals();

    if ( x == 0 && y == 0 )
    {
        // We're done
        aPhi = 0;
        aMagnitude = 0;
        return;
    }

    // Coarse rotate to [-pi/4,pi/4)
    if ( x-y >= 0 )
    {
        if ( x+y >= 0 )
        {
            // East (Correct) quadrant
        }
        else
        {
            // South, rotate by +pi/2
            int xtmp = -y;
            int ytmp = x;
            x = xtmp;
            y = ytmp;
            rotation += encodeAngle(M_PI/2);
        }
    }
    else
    {
        if ( x+y >= 0 )
        {
            // North, rotate by -pi/2
            int xtmp = y;
            int ytmp = -x;
            x = xtmp;
            y = ytmp;
            rotation += encodeAngle(-M_PI/2);
        }
        else
        {
            // West, rotate by pi
            x = -x;
            y = -y;
            rotation += encodeAngle(M_PI);
        }
    }
    if ( debug_ ) std::cout << "Coarse rotate" << std::endl;
    if ( debug_ ) printVals();

    if ( debug_ ) std::cout << "Starting iterations" << std::endl;
    for ( int i=1; i<=iterations_; ++i )
    {
        int sign = (y>=0) ? -1:1;
        int xtmp = x - sign*(y>>i);
        int ytmp = y + sign*(x>>i);
        x = xtmp;
        y = ytmp;
        rotation += sign*rotations_[i-1];
        if ( debug_ ) printVals();
    }
    
    // need a little extra room for the last multiplication (2*internalBits_ size)
    aMagnitude = ((long) x * (long) scaleFactor_)>>(2*internalBits_-outputBits_);

    // Xilinx seems to just mod to [-pi,pi]
    if ( rotation > encodeAngle(M_PI) ) rotation -= 2*encodeAngle(M_PI)+1;
    aPhi = (-rotation)>>(internalBits_-outputBits_);
}
