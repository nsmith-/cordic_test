#ifndef CordicXIP_H
#define CordicXIP_H

#include <vector>
#include <stdint.h>
#include "cordic_v6_0_bitacc_cmodel.h"

class CordicXIP
{
  public:
    CordicXIP(int inputBits, int magBits, int phiBits);
    virtual ~CordicXIP();

    void operator() ( int32_t aX , int32_t aY , int32_t& aPhi , uint32_t& aMagnitude );

    uint32_t tower( const double& aRadians );

// remove from forward declaration
  private:
    double convertIntToXIPInput(int64_t input, const int bits);

    int inputBits_;
    int magBits_;
    int phiBits_;
    xip_cordic_v6_0 * xip_handle_;
};

#endif
