#include <memory>
#include <iostream>
#include <vector>
#include <tuple>
#include <cmath>
#include <cassert>

#include "CordicXilinx.h"

class Region
{
  public:
    Region(int crate, int card, int region, int etval) : icrate(crate), icard(card), irgn(region), et(etval)
    {
      assert(icrate >= 0 && icrate < 18);
      assert(icard >= 0 && icard < 7);
      assert((irgn & -2) == 0);
      assert(et>=0 && et<(1<<10));
    };
    int icrate;
    int icard;
    int irgn;
    int et;

    int iphi()
    {
      int phi_index = icrate % 9;
      if ((icard == 0) || (icard == 2) || (icard == 4))
        phi_index = phi_index * 2;
      else if ((icard == 1) || (icard == 3) || (icard == 5))
        phi_index = phi_index * 2 + 1;
      else if (icard == 6)
        phi_index = phi_index * 2 + irgn;
      return (22 - phi_index) % 18;
    };
};

std::tuple<int, int, int> doSumAndMET(std::vector<Region>& regionEt);
int cordicToMETPhi(int phase);

CordicXilinx cordic(24, 19);
std::array<long, 5> cosines
{
  static_cast<long>(pow(2,30)*cos( 0*M_PI/180)),
  static_cast<long>(pow(2,30)*cos(20*M_PI/180)),
  static_cast<long>(pow(2,30)*cos(40*M_PI/180)),
  static_cast<long>(pow(2,30)*cos(60*M_PI/180)),
  static_cast<long>(pow(2,30)*cos(80*M_PI/180))
};
std::array<long, 5> sines
{
  static_cast<long>(pow(2,30)*sin( 0*M_PI/180)),
  static_cast<long>(pow(2,30)*sin(20*M_PI/180)),
  static_cast<long>(pow(2,30)*sin(40*M_PI/180)),
  static_cast<long>(pow(2,30)*sin(60*M_PI/180)),
  static_cast<long>(pow(2,30)*sin(80*M_PI/180))
};

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
      std::vector<Region> regionET = {
        {1,  2, 1, 120},
        {3,  4, 0, 32},
        {15, 1, 1, 88}
      };
      auto result = doSumAndMET(regionET);
      std::cout << "sum_et: " << std::get<0>(result) << ", met: " << std::get<1>(result) << ", met phi: " << std::get<2>(result) << std::endl;
    }
}

std::tuple<int, int, int>
doSumAndMET(std::vector<Region>& regionEt)
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
  std::array<int, 18> sumEtaPos{};
  std::array<int, 18> sumEtaNeg{};
  for ( auto& r : regionEt )
  {
    if ( r.icrate < 9 )
      sumEtaNeg[r.iphi()] += r.et;
    else
      sumEtaPos[r.iphi()] += r.et;
  }

  std::array<int, 18> sumEta{};
  int sumEt(0);
  for(int i=0; i<sumEta.size(); ++i)
  {
    assert(sumEtaPos[i] >= 0 && sumEtaNeg[i] >= 0);
    sumEta[i] = sumEtaPos[i] + sumEtaNeg[i];
    sumEt += sumEta[i];
  }
  sumEt = (sumEt % (1<<12)) | ((sumEt >= (1<<12)) ? (1<<12):0);
  assert(sumEt>=0 && sumEt < (1<<13));

  // 0, 20, 40, 60, 80 degrees
  std::array<int, 5> sumsForCos{};
  std::array<int, 5> sumsForSin{};
  for(int iphi=0; iphi<sumEta.size(); ++iphi)
  {
    if ( iphi < 5 )
    {
      sumsForCos[iphi] += sumEta[iphi];
      sumsForSin[iphi] += sumEta[iphi];
    }
    else if ( iphi < 9 )
    {
      sumsForCos[9-iphi] -= sumEta[iphi];
      sumsForSin[9-iphi] += sumEta[iphi];
    }
    else if ( iphi < 14 )
    {
      sumsForCos[iphi-9] -= sumEta[iphi];
      sumsForSin[iphi-9] -= sumEta[iphi];
    }
    else
    {
      sumsForCos[18-iphi] += sumEta[iphi];
      sumsForSin[18-iphi] -= sumEta[iphi];
    }
  }

  long sumX(0l);
  long sumY(0l);
  for(int i=0; i<5; ++i)
  {
    sumX += sumsForCos[i]*cosines[i];
    sumY += sumsForSin[i]*sines[i];
  }
  assert(abs(sumX)<(1l<<48) && abs(sumY)<(1l<<48));
  int cordicX = sumX>>25;
  int cordicY = sumY>>25;

  uint32_t cordicMag(0);
  int cordicPhase(0);
  cordic(cordicX, cordicY, cordicPhase, cordicMag);

  int met  = (cordicMag % (1<<12)) | ((cordicMag >= (1<<12)) ? (1<<12):0);
  int metPhi = cordicToMETPhi(cordicPhase);
  assert(metPhi >=0 && metPhi < 72);

  return std::make_tuple(sumEt, met, metPhi);
}

// phase 3Q16 to 72 (5719)
int cordicToMETPhi(int phase)
{
  std::array<int, 73> values;
  for(int i=0; i<values.size(); ++i)
    values[i] = static_cast<int>(pow(2.,16)*(i-36)*M_PI/36);

  for(int i=0; i<values.size()-1; ++i)
    if ( phase >= values[i] && phase < values[i+1] )
      return i;
  return -1;
}
