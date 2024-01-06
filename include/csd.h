/*

  FPTOOL - a fixed-point math to VHDL generation tool

  Description:  A canonical signed digit type

  Author: Niels A. Moseley

*/

#ifndef csd_h
#define csd_h

#include <stdint.h>
#include <vector>

/** A canonical signed digit represents [-/+] 2^power */
struct csdigit_t
{
    int32_t sign{0};    // -1 or +1, 0 means undefined
    int32_t power{0};   // power of two
};

struct csd_t
{
    csd_t() = default;

    std::vector<csdigit_t>  digits;         // CSD representation
    double                  value{0.0};     // floating-point representation
    int32_t                 intBits{0};     // integer bits needed
    int32_t                 fracBits{0};    // fractional bits needed
};

/** convert a floating-point value to a CSD representation
    with a determined number of terms */
bool convertToCSD(const double v, uint32_t terms, csd_t &result);

/** convert a floating-point value to a CSD representation
    with a determined precision */
//bool convertToCSD(const double v, double precisionPercent, csd_t &result);

#endif
