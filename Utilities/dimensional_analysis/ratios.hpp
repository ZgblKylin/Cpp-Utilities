#ifndef UTILITIES_RATIOS_HPP
#define UTILITIES_RATIOS_HPP

#include <utility>
#include <cstdint>
#include <cmath>
#include "../utilities_common.hpp"

UTILITIES_NAMESPACE_BEGIN

std::pair<intmax_t, intmax_t> approximateRatio(long double value, int n)
{
    auto mod = [](long double a, long double b) -> long double {
        while (a >= b) a -= b;
        return a;
    };
    auto gcd = [mod](long double x, long double y) -> std::pair<long double, long double> {
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

UTILITIES_NAMESPACE_END

#endif // UTILITIES_RATIOS_HPP
