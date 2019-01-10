#include <gtest/gtest.h>
#include <DimensionalAnalysis/Ratios.hpp>

TEST(Ratios, approximateRatio)
{
    long double pi = 2.5l;
    std::pair<intmax_t, intmax_t> pair = Dimensional::approximateRatio(pi, 1);
    EXPECT_EQ(pair.first, 5);
    EXPECT_EQ(pair.second, 2);
}
