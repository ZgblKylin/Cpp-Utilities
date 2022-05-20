#include <gtest/gtest.h>
#include <Utilities/DimensionalAnalysis/Ratios.hpp>

UTILITIES_USING_NAMESPACE;
using namespace Dimensional;

TEST(Ratios, approximateRatio)
{
    long double pi = 2.5l;
    std::pair<intmax_t, intmax_t> pair = approximateRatio(pi, 1);
    EXPECT_EQ(pair.first, 5);
    EXPECT_EQ(pair.second, 2);
}
