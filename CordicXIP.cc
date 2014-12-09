//#include "L1Trigger/L1TCalorimeter/interface/CordicXIP.h"
#include "CordicXIP.h"

#include <vector>
#include <iostream>
#include <iomanip> 
#include <math.h>
#include <byteswap.h>

#include "cordic_v6_0_bitacc_cmodel.h"
#include "gmp.h"

CordicXIP::CordicXIP(int inputBits, int magBits, int phiBits) :
    inputBits_(inputBits),
    magBits_(magBits),
    phiBits_(phiBits)
{
    xip_cordic_v6_0_config config;
    xip_cordic_v6_0_default_config(&config);
    config.CordicFunction = XIP_CORDIC_V6_0_F_TRANSLATE;
    config.CoarseRotate   = 1;
    config.DataFormat     = XIP_CORDIC_V6_0_FORMAT_SIG_FRAC;
    config.PhaseFormat    = XIP_CORDIC_V6_0_FORMAT_RAD;
    config.InputWidth     = inputBits_;
    config.OutputWidth    = magBits_;
    config.Precision      = 0;
    config.RoundMode      = XIP_CORDIC_V6_0_ROUND_TRUNCATE;
    config.ScaleComp      = XIP_CORDIC_V6_0_SCALE_EMB_MULT;
    //config.ScaleComp      = XIP_CORDIC_V6_0_SCALE_NONE;

    xip_handle_ = xip_cordic_v6_0_create(&config, [](void* dummy, int error, const char* msg)->void{std::cerr << msg << std::endl;}, 0);
}

CordicXIP::~CordicXIP()
{
    xip_cordic_v6_0_destroy(xip_handle_);
}

double CordicXIP::convertIntToXIPInput(int64_t input, const int bits)
{
    if ( bits > 63 ) std::cerr << "Not going to work!!" << std::endl;
    bool negative = input < 0;
    // abs. value 
    if ( negative ) input ^= -1ll;
    // clamp to max value
    input &= (1ll<<(bits-1)) - 1;
    // set negative sign at appropriate spot
    if ( negative ) input |= 1ll<<(bits-1);
    // fix endianess
    input = __bswap_64(input);
    double * out = reinterpret_cast<double*>(&input);
    return *out;
}

void CordicXIP::operator() ( int32_t aX , int32_t aY , int32_t& aPhi , uint32_t& aMagnitude )
{
    xip_array_complex* cartin = xip_array_complex_create();
    xip_array_complex_reserve_dim(cartin,1);
    cartin->dim_size = 1;
    cartin->dim[0] = 1;
    cartin->data_size = cartin->dim[0];
    if (xip_array_complex_reserve_data(cartin,cartin->data_size) != XIP_STATUS_OK ) {
      std::cerr << "ERROR: Unable to reserve memory for input data packet!" << std::endl;
    }

    xip_array_real* magout = xip_array_real_create();
    xip_array_real_reserve_dim(magout,1);
    magout->dim_size = 1;
    magout->dim[0] = 1;
    magout->data_size = magout->dim[0];
    if (xip_array_real_reserve_data(magout,magout->data_size) != XIP_STATUS_OK) {
      std::cerr << "ERROR: Unable to reserve memory for output data packet!" << std::endl;
    }
  
    xip_array_real* phaseout = xip_array_real_create();
    xip_array_real_reserve_dim(phaseout,1);
    phaseout->dim_size = 1;
    phaseout->dim[0] = 1;
    phaseout->data_size = phaseout->dim[0];
    if (xip_array_real_reserve_data(phaseout,phaseout->data_size) != XIP_STATUS_OK) {
      std::cerr << "ERROR: Unable to reserve memory for output data packet!" << std::endl;
    }

    xip_cordic_v6_0_xip_array_complex_set_data(cartin, {(double) aX, (double) aY}, 0);   
    auto printArray = [](void * a, size_t len)
    {
        auto p = (unsigned char *) a;
        size_t i;
        for ( i=0; i<len; ++i )
        {
            printf("%02x ", p[i]);
            if ( (i+1)%8 == 0 ) std::cout << std::endl;
        }
        if ( i%8 != 0 ) std::cout << std::endl;
    };

    auto status = xip_cordic_v6_0_translate(xip_handle_, cartin, magout, phaseout, 1);
    if ( status != XIP_STATUS_OK ) std::cout << "fuck " << std::endl;

    double mag_out_samp;
    double phase_out_samp;
    xip_cordic_v6_0_xip_array_real_get_data(magout, &mag_out_samp, 0);
    xip_cordic_v6_0_xip_array_real_get_data(phaseout, &phase_out_samp, 0);

    aMagnitude = (uint32_t) mag_out_samp;
    aPhi = (int32_t) phase_out_samp;
}
