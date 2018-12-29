#pragma once

#include <utility>
#include <cstdint>
#include <cmath>
#include "../Common.h"

UTILITIES_NAMESPACE_BEGIN

/**
 * \addtogroup DimensionalAnalysis
 * @{
 */
namespace Dimensional {
/**
 * \brief Calculate approximate fraction from input decimal.
 * \details `#include <Ratios.hpp>`
 * \param value Decimal which fraction will generate from.
 * \param n     Max calculate precision. Not the bigger the better, too big
 *              value wiil caouse integer overflow, and some small value may
 *              product fraction with higher precision.
 * \return Numerator and denominator of the fraction with type `intmax_t`.
 */
std::pair<intmax_t, intmax_t> approximateRatio(long double value, int n)
{
    // Modulo operation for floating point
    auto mod = [](long double a, long double b) -> long double {
        while (a >= b) a -= b;
        return a;
    };
    // Greatest common divider for floating point
    auto gcd = [mod](long double x, long double y)
               -> std::pair<long double, long double> {
        long double a = x;
        long double b = y;
        long double c = mod(a, b);
        while (int(c) != 0)
        {
            a = b;
            b = c;
            c = mod(a, b);
        }
        return std::make_pair(x / b, y / b);
    };
    long double den = std::pow(static_cast<long double>(10), n);
    long double num = value * std::pow(static_cast<long double>(10), n);
    std::pair<long double, long double> pair = gcd(num, den);
    return std::make_pair(intmax_t(std::round(pair.first)),
                          intmax_t(std::round(pair.second)));
}

} // namespace dimensional
/** @} */

UTILITIES_NAMESPACE_END
