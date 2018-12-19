#ifndef DIMENSIONALANALYSIS_HPP
#define DIMENSIONALANALYSIS_HPP

#include <tuple>
#include <ratio>
#include <chrono>
#include "../utilities_common.hpp"

UTILITIES_NAMESPACE_BEGIN

namespace dimensions {
template<int length, int mass, int time, int current, int temperature, int amount_of_substance, int illuminace>
struct dimension_t
{
    using type = dimension_t<length, mass, time, current, temperature, amount_of_substance, illuminace>;
    static constexpr std::tuple<int, int, int, int, int, int, int> factors{
        length, mass, time, current, temperature, amount_of_substance, illuminace
    };
    static constexpr int factor_length              = std::get<0>(factors);
    static constexpr int factor_mass                = std::get<1>(factors);
    static constexpr int factor_time                = std::get<2>(factors);
    static constexpr int factor_current             = std::get<3>(factors);
    static constexpr int factor_temperature         = std::get<4>(factors);
    static constexpr int factor_amount_of_substance = std::get<5>(factors);
    static constexpr int factor_illuminace          = std::get<6>(factors);
};
using scala                 = dimension_t<0, 0, 0, 0, 0, 0, 0>;
using length                = dimension_t<1, 0, 0, 0, 0, 0, 0>; // m
using mass                  = dimension_t<0, 1, 0, 0, 0, 0, 0>; // kg
using time                  = dimension_t<0, 0, 1, 0, 0, 0, 0>; // s
using current               = dimension_t<0, 0, 0, 1, 0, 0, 0>; // A
using temperature           = dimension_t<0, 0, 0, 0, 1, 0, 0>; // K
using amount_of_substance   = dimension_t<0, 0, 0, 0, 0, 1, 0>; // mol
using illuminace            = dimension_t<0, 0, 0, 0, 0, 0, 1>; // cd

template<typename dimension1,  typename dimension2>
using dimension_multiply = dimension_t<
    dimension1::factor_length               + dimension2::factor_length,
    dimension1::factor_mass                 + dimension2::factor_mass,
    dimension1::factor_time                 + dimension2::factor_time,
    dimension1::factor_current              + dimension2::factor_current,
    dimension1::factor_temperature          + dimension2::factor_temperature,
    dimension1::factor_amount_of_substance  + dimension2::factor_amount_of_substance,
    dimension1::factor_illuminace           + dimension2::factor_illuminace
>;
template<typename dimension1,  typename dimension2>
using dimension_divide = dimension_t<
    dimension1::factor_length               - dimension2::factor_length,
    dimension1::factor_mass                 - dimension2::factor_mass,
    dimension1::factor_time                 - dimension2::factor_time,
    dimension1::factor_current              - dimension2::factor_current,
    dimension1::factor_temperature          - dimension2::factor_temperature,
    dimension1::factor_amount_of_substance  - dimension2::factor_amount_of_substance,
    dimension1::factor_illuminace           - dimension2::factor_illuminace
>;
template<typename dimension, int n>
using dimension_pow = dimension_t<
    dimension::factor_length              * n,
    dimension::factor_mass                * n,
    dimension::factor_time                * n,
    dimension::factor_current             * n,
    dimension::factor_temperature         * n,
    dimension::factor_amount_of_substance * n,
    dimension::factor_illuminace          * n
>;
template<typename Dimension, int n>
using dimension_root = dimension_t<
    Dimension::factor_length              / n,
    Dimension::factor_mass                / n,
    Dimension::factor_time                / n,
    Dimension::factor_current             / n,
    Dimension::factor_temperature         / n,
    Dimension::factor_amount_of_substance / n,
    Dimension::factor_illuminace          / n
>;

using frenquency    = dimension_divide<scala, time>;            // Hz 1/s
using speed         = dimension_divide<length, time>;           // Speed m/s
using acceleration  = dimension_divide<speed, time>;            // Acceleration m/s^2
using force         = dimension_multiply<mass, acceleration>;   // Newton kg⋅m/s^2
using area          = dimension_multiply<length, length>;       // Area m^2
using volume        = dimension_multiply<area, length>;         // Volume m^3
using pressure      = dimension_divide<force, area>;            // Pascal N/m^2

// forward declarations
template<typename T, typename dimension, typename ratio>
struct quantity;
template<typename new_ratio, typename T, typename dimension, typename ratio>
quantity<T, dimension, new_ratio> quantity_cast(quantity<T, dimension, ratio> x);

template<typename T, typename dimension, typename ratio = std::ratio<1>>
struct quantity
{
    using type = quantity<T, dimension, ratio>;
    using value_type = T;
    using dimension_type = dimension;
    using ratio_type = ratio;

    inline explicit quantity(T x = 0)
        : v(x)
    {}

    template<typename ratio2>
    inline quantity(const quantity<T, dimension, ratio2>& other)
        : v(quantity_cast<ratio>(other).value())
    {}

    inline T value() const
    { return v; }

    template<typename ratio2>
    inline type& operator=(const quantity<T, dimension, ratio2>& other)
    {
        v = quantity_cast<ratio>(other).v;
        return *this;
    }

    template<typename ratio2>
    inline bool operator==(const quantity<T, dimension, ratio2>& other) const
    { return v == quantity_cast<ratio>(other).v; }

    template<typename ratio2>
    inline bool operator!=(const quantity<T, dimension, ratio2>& other) const
    { return !(*this == other); }

    template<typename ratio2>
    inline bool operator<(const quantity<T, dimension, ratio2>& other) const
    { return v < quantity_cast<ratio>(other).v; }

    template<typename ratio2>
    inline bool operator<=(const quantity<T, dimension, ratio2>& other) const
    { return (*this < other) || (*this == other); }

    template<typename ratio2>
    inline bool operator>(const quantity<T, dimension, ratio2>& other) const
    { return !(*this <= other); }

    template<typename ratio2>
    inline bool operator>=(const quantity<T, dimension, ratio2>& other) const
    { return !(*this < other); }

private:
    T v;
};

template<typename new_ratio, typename T, typename dimension, typename ratio>
inline quantity<T, dimension, new_ratio> quantity_cast(quantity<T, dimension, ratio> x)
{
    using ratio_div = std::ratio_divide<ratio, new_ratio>;
    T val = x.value() * ratio_div::num / ratio_div::den;
    return quantity<T, dimension, new_ratio>(val);
}

using ratio_PI = std::ratio<80813362, 25723692>; // 3.14159266096017653069
using ratio_degree = std::ratio_divide<ratio_PI, std::ratio<180>>; // degree = rad * PI / 180

using ratio_mile = std::ratio_multiply<std::ratio<1609344ll, 1000000ll>, std::kilo>; // mile = 1.609344 km = 1.609344 * 1000 m
using ratio_yard = std::ratio_divide<ratio_mile, std::ratio<1760>>; // yard = 1/1760 yard
using ratio_feet = std::ratio_divide<ratio_yard, std::ratio<3>>; // feet = 1/3 yard
using ratio_inch = std::ratio_divide<ratio_feet, std::ratio<12>>; // inch = 1/12 feet
using ratio_pica = std::ratio_divide<ratio_inch, std::ratio<6>>; // pica = 1/6 inch
using ratio_len_point = std::ratio_divide<ratio_pica, std::ratio<12>>; // point = 1/12 pica

// ================ EN units ================
using ratio_en_fluid_dram = std::ratio_multiply<std::ratio<35516328125ll, 10000000000ll>, std::micro>; // fluid dram = 3.5516328125 ml = 3.5516328125 / 1,000,000 m*3
using ratio_en_fluid_ounce = std::ratio_multiply<ratio_en_fluid_dram, std::ratio<8>>; // fluid ounce = 8 fluind dram
using ratio_en_gill = std::ratio_multiply<ratio_en_fluid_ounce, std::ratio<5>>; // gill = 5 fluid ounce
using ratio_en_cup = std::ratio_multiply<ratio_en_gill, std::ratio<2>>; // cup = 2 gill
using ratio_en_pint = std::ratio_multiply<ratio_en_cup, std::ratio<2>>; // pint = cup
using ratio_en_quart = std::ratio_multiply<ratio_en_pint, std::ratio<2>>; // quart = 2 pint
using ratio_en_gallon = std::ratio_multiply<ratio_en_quart, std::ratio<4>>; // gallon = 4 quart
using ratio_en_peck = std::ratio_multiply<ratio_en_gallon, std::ratio<2>>; // peck = 2 gallon;
using ratio_en_bushel = std::ratio_multiply<ratio_en_peck, std::ratio<4>>; // bushel = 4 peck;

// ================ US units ================
// US wet volume units
using ratio_us_fluid_dram = std::ratio_multiply<std::ratio<36966911953125ll, 10000000000000ll>, std::micro>; // fluid dram = 3.6966911953125 ml = 3.6966911953125 / 1,000,000 m^3
using ratio_us_fluid_ounce = std::ratio_multiply<ratio_us_fluid_dram, std::ratio<8>>; // fluid ounce = 8 fluid dram
using ratio_us_teaspoons = std::ratio_divide<ratio_us_fluid_ounce, std::ratio<6>>; // teaspoons = 1/6 fluid ounce
using ratio_us_tablespoons = std::ratio_divide<ratio_us_fluid_ounce, std::ratio<2>>; // tablespoons = 1/2 fluid ounce
using ratio_us_gill = std::ratio_multiply<ratio_us_fluid_ounce, std::ratio<4>>; // gill = 4 fluid ounce
using ratio_us_cup = std::ratio_multiply<ratio_us_gill, std::ratio<2>>; // cup = 2 gill
using ratio_us_pint = std::ratio_multiply<ratio_us_cup, std::ratio<2>>; // pint = 2 cup
using ratio_us_quart = std::ratio_multiply<ratio_us_pint, std::ratio<2>>; // quart = 2 pint
using ratio_us_gallon = std::ratio_multiply<ratio_us_quart, std::ratio<4>>; // gallon = 4 quart
// US dry volume units
using ratio_us_dry_pint = std::ratio_multiply<std::ratio<5506104713575ll, 10000000000000ll>, std::milli>; // dry pint = 0.5506104713575 l = 0.5506104713575 / 1000 m^3
using ratio_us_dry_quart = std::ratio_multiply<ratio_us_dry_pint, std::ratio<2>>; // dry quart = 2 dry pint
using ratio_us_dry_gallon = std::ratio_multiply<ratio_us_dry_quart, std::ratio<4>>; // dry gallon = 4 dry quart
using ratio_us_dry_peck = std::ratio_multiply<ratio_us_dry_gallon, std::ratio<2>>; // dry peck = 2 dry gallon
using ratio_us_bushel = std::ratio_multiply<ratio_us_dry_peck, std::ratio<4>>; // bushel = 4 dry peck
} // namespace dimensions

template<typename T, typename Dimension, typename ratio1, typename ratio2>
inline dimensions::quantity<T, Dimension, ratio1>
operator+(dimensions::quantity<T, Dimension, ratio1> x, dimensions::quantity<T, Dimension, ratio2> y)
{
    return dimensions::quantity<T, Dimension, ratio1>(x.value()
                                                      + dimensions::quantity_cast<ratio1>(y).value());
}

template<typename T, typename Dimension, typename ratio1, typename ratio2>
inline dimensions::quantity<T, Dimension, ratio1>
operator-(dimensions::quantity<T, Dimension, ratio1>& x, dimensions::quantity<T, Dimension, ratio2> y)
{
    return dimensions::quantity<T, Dimension, ratio1>(x.value()
                                                      - dimensions::quantity_cast<ratio1>(y).value());
}

template<typename T, typename dimension1, typename ratio1, typename dimension2, typename ratio2>
inline dimensions::quantity<T, dimensions::dimension_multiply<dimension1, dimension2>, ratio1>
operator*(dimensions::quantity<T, dimension1, ratio1> x, dimensions::quantity<T, dimension2, ratio2> y)
{
    using dim = dimensions::dimension_multiply<dimension1, dimension2>;
    return dimensions::quantity<T, dim, ratio1>(x.value()
                                                * dimensions::quantity_cast<ratio1>(y).value());
}

template<typename T, typename dimension1, typename ratio1, typename dimension2, typename ratio2>
inline dimensions::quantity<T, dimensions::dimension_divide<dimension1, dimension2>, ratio1>
operator/(dimensions::quantity<T, dimension1, ratio1> x, dimensions::quantity<T, dimension2, ratio2> y)
{
    using dim = dimensions::dimension_divide<dimension1, dimension2>;
    return dimensions::quantity<T, dim, ratio1>(x.value()
                                                / dimensions::quantity_cast<ratio1>(y).value());
}

template<int factor, typename T, typename Dimension, typename ratio>
inline dimensions::quantity<T, dimensions::dimension_pow<Dimension, factor>, std::ratio<1>>
pow(const dimensions::quantity<T, Dimension, ratio>& x)
{
    return dimensions::quantity<
               T,
               dimensions::dimension_pow<Dimension, factor>,
               std::ratio<1>
           >(std::pow(dimensions::quantity_cast<std::ratio<1>>(x).value(),
                      factor));
}

template<int factor, typename T, typename Dimension, typename ratio>
inline dimensions::quantity<T, dimensions::dimension_root<Dimension, factor>, std::ratio<1>>
root(const dimensions::quantity<T, Dimension, ratio>& x)
{
    return dimensions::quantity<
               T,
               dimensions::dimension_root<Dimension, factor>,
               std::ratio<1>
           >(std::pow(dimensions::quantity_cast<std::ratio<1>>(x).value(),
                      1.0 / factor));
}

UTILITIES_NAMESPACE_END

#endif // DIMENSIONALANALYSIS_HPP
