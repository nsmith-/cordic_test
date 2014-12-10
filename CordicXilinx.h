#ifndef CordicXilinx_H
#define CordicXilinx_H

#include <vector>
#include <stdint.h>

class CordicXilinx
{
  public:
    CordicXilinx(int inputBits, int outputBits, int phiScale, bool debug=false);

    void operator() ( int32_t xInput , int32_t yInput , int32_t& aPhi , uint32_t& aMagnitude ) const;

    // Returns angle in fixed point according to internalBits_ precision
    // The integer part is 3 bits (i.e. -4 to 3)
    // Valid input is -pi < angle < pi
    int encodeAngle(const double angleFloat) const;

// remove from forward declaration
  private:
    const int inputBits_;
    const int outputBits_;
    const int phiScale_;
    const bool debug_;

    std::vector<int> rotations_;
    int iterations_;
    int internalBits_;
    int scaleFactor_;
};

#endif
