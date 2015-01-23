#include <memory>
#include <iostream>
#include <vector>
#include <tuple>
#include <cmath>

#include "CordicXilinx.h"

std::tuple<int, int, int>
doSumAndMET(std::vector<std::pair<int, int>>& regionEt);

CordicXilinx cordic(24, 19);

int main (int argc, char ** argv)
{
    if ( argc > 2 )
    {
      int et = atoi(argv[1]);
      int phi = atoi(argv[2]);

      int sinphi = pow(2., 15)*sin(phi*M_PI/9)+0.5;
      int cosphi = pow(2., 15)*cos(phi*M_PI/9)+0.5;

      int ex = (et*cosphi) >> 10;
      int ey = (et*sinphi) >> 10;

      std::cout << "ex int: " << ex << " double: " << et*32*cos(phi*M_PI/9) << " diff: " << ex - et*32*cos(phi*M_PI/9) << std::endl;
      std::cout << "ey int: " << ey << " double: " << et*32*sin(phi*M_PI/9) << " diff: " << ey - et*32*sin(phi*M_PI/9) << std::endl;

      uint32_t sum_et;
      int sum_phi;
      cordic(ex, ey, sum_phi, sum_et);

      double phi_out = sum_phi*pow(2., -16)*9./M_PI;
      while ( phi_out < 0 ) phi_out += 18.;

      std::cout << "sum_et: " << sum_et << " phi: " << phi_out << std::endl;
    }
    else
    {
      std::vector<std::pair<int, int>> regionET = {
        {1, 12},
        {3, 9},
        {15, 1}
      };
      auto result = doSumAndMET(regionET);
      std::cout << "sum_et: " << std::get<0>(result) << ", met: " << std::get<1>(result) << ", met phi: " << std::get<2>(result) << std::endl;
    }
}

std::tuple<int, int, int>
doSumAndMET(std::vector<std::pair<int, int>>& regionEt)
{
// if any region et/ht has overflow bit, set sumET overflow bit
// met/mht same, breakout to function
// threshold 15 mht 
// sum phi above threshold for -eta for region 0,2,4,6.0 : 18 -eta numbers 15 bit
// sum phi above threshold for +eta for region 1,3,5,6.1 : 18 +eta numbers 15 bit
// sum -eta and +eta 15 bit
// sum all for sumET : 19 bit (22*18 fits in 9 bits, started with 10 bits)
// sumET mask to 12 bits both
// if larger set overflow, ovf is thirteenth bit in sumET/HT
// sum phi[x] and -phi[x+9] for x=0..8 : 16 bit 2'sC
// e.g 0-180, 20-200, .. also 280-100, etc. 4 numbers
// treat cos(+-60) special 1/2 e.g. shift 29 bits
// 30 bit sin/cos : 46 bit result still 2'sC
// add x and y values in 49 bit 2'sC
// shift (truncate) to 24 bit cordic input
// CORDIC
// MET get first 12 bits
// if >2^12, set ovf (13th bit)
// MHT get first 7
// if >2^7 set ovf (8th bit)
// phase 3Q16 to 72 (5719)
// by starting at -pi and going CCW around circle try to see if angle <= x < angle+1
// mht shift down truncate to 5
// So etsum from 180 to 185 degress is MET phi of 0
// 13 13 13 8 7 5
  return std::make_tuple(1, 1, 1);
}
