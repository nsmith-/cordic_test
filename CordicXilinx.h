#ifndef CordicXilinx_H
#define CordicXilinx_H

#include <vector>
#include <stdint.h>

class CordicXilinx
{
  public:
    CordicXilinx(int inputBits, int outputBits, int phiScale, bool debug=false);
    virtual ~CordicXilinx();

    void operator() ( int32_t aX , int32_t aY , int32_t& aPhi , uint32_t& aMagnitude );

    // Returns angle in fixed point according to internalBits_ precision
    // The integer part is 3 bits (i.e. -4 to 3)
    // Valid input is -pi < angle < pi
    int encodeAngle(const double angleFloat);

// remove from forward declaration
  private:
    int inputBits_;
    int outputBits_;
    int phiScale_;
    bool debug_;

    std::vector<int> rotations_;
    int iterations_;
    int internalBits_;
    int scaleFactor_;
};

#endif
