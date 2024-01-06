/*

  FPTOOL - a fixed-point math to VHDL generation tool

  Description:  A canonical signed digit type

  Author: Niels A. Moseley

*/

#include <cmath>
#include "csd.h"
#include <iostream>

/** convert a floating-point value to a CSD representation
    with a determined number of terms */
bool convertToCSD(const double v, uint32_t terms, csd_t &result)
{
    const double eps = 1e-20;
    result.value = 0.0;
    result.intBits = 0;
    result.fracBits = 0;
    double residue = v;
    for(uint32_t i=0; i<terms; i++)
    {
        if (fabs(residue) < eps)
            return true;

        bool sign = residue < 0.0;

        csdigit_t digit;
        if (sign)
        {
            digit.power = static_cast<int32_t>(round(log(-residue)/log(2.0)));

            double val = pow(2.0, static_cast<double>(digit.power));
            residue += val;
            result.value -= val;
            digit.sign = -1;
        }
        else
        {
            digit.power = static_cast<int32_t>(round(log(residue)/log(2.0)));

            double val = pow(2.0, static_cast<double>(digit.power));
            residue -= val;
            result.value += val;
            digit.sign = 1;
        }
        result.digits.push_back(digit);
    }
    result.intBits = result.digits[0].power+2;    // account for sign bit
    result.fracBits= -result.digits.back().power;

    return true;
}

#if 0
fplib::SFix convertCSDToSFix(const csd_t &csd)
{
    fplib::SFix num(csd.intBits, csd.fracBits);
    auto iter = csd.digits.begin();
    while(iter != csd.digits.end())
    {
        num.addPowerOfTwo(iter->power, iter->sign < 0);
        iter++;
    }
    return num;
}
#endif


/** convert a floating-point value to a CSD representation
    with a determined precision */
//bool convertToCSD(const double v, double precisionPercent, csd_t &result);
