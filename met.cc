#include <memory>
#include <iostream>
#include <vector>
#include <cmath>

#include "CordicXilinx.h"

int main (int argc, char ** argv)
{
    CordicXilinx cordic(24, 19);

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
}
