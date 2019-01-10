#include <gtest/gtest.h>
#include <DimensionalAnalysis/DimensionalAnalysis.hpp>
#include <typeinfo>

UTILITIES_USING_NAMESPACE;
using namespace Dimensional;

template<typename Ratio>
double decimal()
{
    return static_cast<double>(Ratio::num) / Ratio::den;
}

TEST(Unit, Unit)
{
    using Type1 = Unit<0, 0, 0, 0, 0, 0, 0>;
    using Type2 = Unit<1, 0, 0, 0, 0, 0, 0>;
    EXPECT_NE(typeid(Type1), typeid(Type2));
    bool isSame = std::is_same<Type1, Type2>::value;
    ASSERT_FALSE(isSame);

    using TestUnit = Unit<0, 1, 2, 3, 4, 5, 6>;
    std::tuple<int, int, int, int, int, int, int> factors{0, 1, 2, 3, 4, 5, 6};
    EXPECT_EQ(TestUnit::factors, factors);
    EXPECT_EQ(TestUnit::factorLength, 0);
    EXPECT_EQ(TestUnit::factorMass, 1);
    EXPECT_EQ(TestUnit::factorTime, 2);
    EXPECT_EQ(TestUnit::factorCurrent, 3);
    EXPECT_EQ(TestUnit::factorTemperature, 4);
    EXPECT_EQ(TestUnit::factorAmountOfSubstance, 5);
    EXPECT_EQ(TestUnit::factorLuminousIntensity, 6);
}

TEST(Unit, baseUnit)
{
    std::tuple<int, int, int, int, int, int, int> scala{0, 0, 0, 0, 0, 0, 0};
    EXPECT_EQ(scala, Scala::factors);

    std::tuple<int, int, int, int, int, int, int> length{1, 0, 0, 0, 0, 0, 0};
    EXPECT_EQ(length, Length::factors);

    std::tuple<int, int, int, int, int, int, int> mass{0, 1, 0, 0, 0, 0, 0};
    EXPECT_EQ(mass, Mass::factors);

    std::tuple<int, int, int, int, int, int, int> time{0, 0, 1, 0, 0, 0, 0};
    EXPECT_EQ(time, Time::factors);

    std::tuple<int, int, int, int, int, int, int> current{0, 0, 0, 1, 0, 0, 0};
    EXPECT_EQ(current, Current::factors);

    std::tuple<int, int, int, int, int, int, int> temperature{0, 0, 0, 0, 1, 0, 0};
    EXPECT_EQ(temperature, Temperature::factors);

    std::tuple<int, int, int, int, int, int, int> amountOfSubstance{0, 0, 0, 0, 0, 1, 0};
    EXPECT_EQ(amountOfSubstance, AmountOfSubstance::factors);

    std::tuple<int, int, int, int, int, int, int> luminousIntensity{0, 0, 0, 0, 0, 0, 1};
    EXPECT_EQ(luminousIntensity, LuminousIntensity::factors);
}

TEST(Unit, calculation)
{
    using speed = UnitDivide<Length, Time>;
    std::tuple<int, int, int, int, int, int, int> factorSpeed{1, 0, -1, 0, 0, 0, 0};
    EXPECT_EQ(factorSpeed, speed::factors);

    using length = UnitMultiply<Speed, Time>;
    std::tuple<int, int, int, int, int, int, int> factorLength{1, 0, 0, 0, 0, 0, 0};
    EXPECT_EQ(factorLength, length::factors);

    using area = UnitPow<Length, 2>;
    std::tuple<int, int, int, int, int, int, int> factorArea{2, 0, 0, 0, 0, 0, 0};
    EXPECT_EQ(factorArea, area::factors);

    using len = UnitRoot<area, 2>;
    EXPECT_EQ(factorLength, len::factors);
}

TEST(Unit, derived)
{
    std::tuple<int, int, int, int, int, int, int> factorSpeed{1, 0, -1, 0, 0, 0, 0};
    EXPECT_EQ(factorSpeed, Speed::factors);

    std::tuple<int, int, int, int, int, int, int> factorAcceleration{1, 0, -2, 0, 0, 0, 0};
    EXPECT_EQ(factorAcceleration, Acceleration::factors);

    std::tuple<int, int, int, int, int, int, int> factorFrenquency{0, 0, -1, 0, 0, 0, 0};
    EXPECT_EQ(factorFrenquency, Frenquency::factors);

    std::tuple<int, int, int, int, int, int, int> factorForce{1, 1, -2, 0, 0, 0, 0};
    EXPECT_EQ(factorForce, Force::factors);

    std::tuple<int, int, int, int, int, int, int> factorArea{2, 0, 0, 0, 0, 0, 0};
    EXPECT_EQ(factorArea, Area::factors);

    std::tuple<int, int, int, int, int, int, int> factorVolume{3, 0, 0, 0, 0, 0, 0};
    EXPECT_EQ(factorVolume, Volume::factors);

    std::tuple<int, int, int, int, int, int, int> factorPressure{-1, 1, -2, 0, 0, 0, 0};
    EXPECT_EQ(factorPressure, Pressure::factors);

    std::tuple<int, int, int, int, int, int, int> factorEnergy{2, 1, -2, 0, 0, 0, 0};
    EXPECT_EQ(factorEnergy, Energy::factors);

    std::tuple<int, int, int, int, int, int, int> factorPower{2, 1, -3, 0, 0, 0, 0};
    EXPECT_EQ(factorPower, Power::factors);

    std::tuple<int, int, int, int, int, int, int> factorCharge{0, 0, 1, 1, 0, 0, 0};
    EXPECT_EQ(factorCharge, Charge::factors);

    std::tuple<int, int, int, int, int, int, int> factorVoltage{2, 1, -3, -1, 0, 0, 0};
    EXPECT_EQ(factorVoltage, Voltage::factors);

    std::tuple<int, int, int, int, int, int, int> factorElelctricCapacitance{-2, -1, 4, 2, 0, 0, 0};
    EXPECT_EQ(factorElelctricCapacitance, ElelctricCapacitance::factors);

    std::tuple<int, int, int, int, int, int, int> factorElectricResistance{2, 1, -3, -2, 0, 0, 0};
    EXPECT_EQ(factorElectricResistance, ElectricResistance::factors);

    std::tuple<int, int, int, int, int, int, int> factorElelctricConductance{-2, -1, 3, 2, 0, 0, 0};
    EXPECT_EQ(factorElelctricConductance, ElelctricConductance::factors);

    std::tuple<int, int, int, int, int, int, int> factorMagneticFlux{2, 1, -2, -1, 0, 0, 0};
    EXPECT_EQ(factorMagneticFlux, MagneticFlux::factors);

    std::tuple<int, int, int, int, int, int, int> factorMagnetFluxDensity{0, 1, -2, -1, 0, 0, 0};
    EXPECT_EQ(factorMagnetFluxDensity, MagnetFluxDensity::factors);

    std::tuple<int, int, int, int, int, int, int> factorInductance{2, 1, -2, -2, 0, 0, 0};
    EXPECT_EQ(factorInductance, Inductance::factors);

    std::tuple<int, int, int, int, int, int, int> factorIlluminance{-2, 0, 0, 0, 0, 0, 1};
    EXPECT_EQ(factorIlluminance, Illuminance::factors);

    std::tuple<int, int, int, int, int, int, int> factorRadioactivity{0, 0, -1, 0, 0, 0, 0};
    EXPECT_EQ(factorRadioactivity, Radioactivity::factors);

    std::tuple<int, int, int, int, int, int, int> factorAbsorbedDose{2, 0, -2, 0, 0, 0, 0};
    EXPECT_EQ(factorAbsorbedDose, AbsorbedDose::factors);

    std::tuple<int, int, int, int, int, int, int> factorEquivalentDose{2, 0, -2, 0, 0, 0, 0};
    EXPECT_EQ(factorEquivalentDose, EquivalentDose::factors);

    std::tuple<int, int, int, int, int, int, int> factorCatalyticActivity{0, 0, -1, 0, 0, 1, 0};
    EXPECT_EQ(factorCatalyticActivity, CatalyticActivity::factors);
}

TEST(Quantity, Quantity)
{
    Quantity<double, Length> m(1);
    EXPECT_EQ(m.value(), 1);
    EXPECT_EQ(m.standard_value(), 1);

    Quantity<double, Length, std::centi> cm(1);
    EXPECT_EQ(cm.value(), 1);
    EXPECT_EQ(cm.standard_value(), 0.01);

    m = quantity_cast<std::ratio<1>>(cm);
    EXPECT_EQ(m.value(), 0.01);

    m.set_value(1);
    EXPECT_EQ(m.value(), 1);

    cm.set_value(100);
    EXPECT_EQ(cm, m);

    cm.set_standard_value(0.01);
    EXPECT_EQ(cm.value(), 1);

    Quantity<double, Length> temp(cm);
    EXPECT_EQ(temp, cm);

    temp += cm;
    EXPECT_EQ(temp.value(), 0.02);

    temp = cm;
    ASSERT_TRUE(temp == cm);
    ASSERT_TRUE(temp != m);
    ASSERT_TRUE(m > temp);
    ASSERT_TRUE(temp < m);
    ASSERT_TRUE(temp >= cm);
    ASSERT_TRUE(temp <= cm);

    EXPECT_EQ((m + cm).value(), 1.01);
    EXPECT_EQ((cm + m).value(), 101);

    EXPECT_EQ((m - cm).value(), 0.99);
    EXPECT_EQ((cm - m).value(), -99);

    EXPECT_EQ((m * cm).value(), 0.01);
    EXPECT_EQ(typeid(decltype(m * cm)::unit_type), typeid(Area));
    EXPECT_EQ((cm * m).value(), 100);

    EXPECT_EQ((m / cm).value(), 100);
    EXPECT_EQ(typeid(decltype(m / cm)::unit_type), typeid(Scala));
    EXPECT_EQ((cm / m).value(), 0.01);

    auto area = pow<2>(cm);
    EXPECT_EQ(area.value(), 0.0001);
    EXPECT_EQ(typeid(decltype(area)::unit_type), typeid(Area));

    auto length = root<2>(area);
    EXPECT_EQ(length.value(), 0.01);
    EXPECT_EQ(typeid(decltype(length)::unit_type), typeid(Length));
}

TEST(Quantity, ChineseUnits)
{
    EXPECT_DOUBLE_EQ(decimal<ratio_length_li>(), 500);
    EXPECT_DOUBLE_EQ(decimal<ratio_yin>(), 100.0 / 3);
    EXPECT_DOUBLE_EQ(decimal<ratio_zhang>(), 10.0 / 3);
    EXPECT_DOUBLE_EQ(decimal<ratio_xun>(), 5.0 / 3);
    EXPECT_DOUBLE_EQ(decimal<ratio_chi>(), 1.0 / 3);
    EXPECT_DOUBLE_EQ(decimal<ratio_cun>(), 1.0 / 30);
    EXPECT_DOUBLE_EQ(decimal<ratio_length_fen>(), 1.0 / 300);
    EXPECT_DOUBLE_EQ(decimal<ratio_length_li2>(), 1.0 / 3000);
    EXPECT_DOUBLE_EQ(decimal<ratio_length_hao>(), 1.0 / 30000);
    EXPECT_DOUBLE_EQ(decimal<ratio_length_si>(), 1.0 / 300000);
    EXPECT_DOUBLE_EQ(decimal<ratio_length_hu>(), 1.0 / 3000000);

    EXPECT_DOUBLE_EQ(decimal<ratio_qing>(), 100.0 * 1000 * 2 / 3);
    EXPECT_DOUBLE_EQ(decimal<ratio_mu>(), 1000.0 * 2 / 3);
    EXPECT_DOUBLE_EQ(decimal<ratio_gong>(), (5.0 / 3) * (5.0 / 3));

    EXPECT_DOUBLE_EQ(decimal<ratio_dan>(), 50);
    EXPECT_DOUBLE_EQ(decimal<ratio_jin>(), 0.5);
    EXPECT_DOUBLE_EQ(decimal<ratio_liang>(), 0.05);
    EXPECT_DOUBLE_EQ(decimal<ratio_qian>(), 0.005);
    EXPECT_DOUBLE_EQ(decimal<ratio_mass_fen>(), 0.0005);
    EXPECT_DOUBLE_EQ(decimal<ratio_mass_li>(), 0.00005);
    EXPECT_DOUBLE_EQ(decimal<ratio_mass_hao>(), 0.000005);
    EXPECT_DOUBLE_EQ(decimal<ratio_mass_si>(), 0.0000005);
    EXPECT_DOUBLE_EQ(decimal<ratio_mass_hu>(), 0.00000005);
}

TEST(Quantity, YardPound)
{
    EXPECT_DOUBLE_EQ(decimal<ratio_mile>(), 1609.344);
    EXPECT_DOUBLE_EQ(decimal<ratio_furlong>(), 201.168);
    EXPECT_DOUBLE_EQ(decimal<ratio_chain>(), 20.1168);
    EXPECT_DOUBLE_EQ(decimal<ratio_yard>(), 0.9144);
    EXPECT_DOUBLE_EQ(decimal<ratio_nail>(), 5.715 / 100);
    EXPECT_DOUBLE_EQ(decimal<ratio_feet>(), 30.48 / 100);
    EXPECT_DOUBLE_EQ(decimal<ratio_inch>(), 2.54 / 100);
    EXPECT_DOUBLE_EQ(decimal<ratio_pica>(), 2.54 / 6 / 100);
    EXPECT_DOUBLE_EQ(decimal<ratio_point>(), 2.54 / 72 / 100);

    EXPECT_DOUBLE_EQ(decimal<ratio_longton>(), 1016.0469088);
    EXPECT_DOUBLE_EQ(decimal<ratio_shortton>(), 907.18474);
    EXPECT_DOUBLE_EQ(decimal<ratio_long_hundredweight>(), 50.80234544);
    EXPECT_DOUBLE_EQ(decimal<ratio_short_hundredweight>(), 45.359237);
    EXPECT_DOUBLE_EQ(decimal<ratio_pound>(), 0.45359237);
    EXPECT_DOUBLE_EQ(decimal<ratio_ounce>(), 28.349523125 / 1000);
    EXPECT_DOUBLE_EQ(decimal<ratio_drachm>(), 1.7718451953125 / 1000);
    EXPECT_DOUBLE_EQ(decimal<ratio_grain>(), 64.79891 / 1000 / 1000);
}

TEST(Quantity, ImperialUnits)
{
    EXPECT_DOUBLE_EQ(decimal<ratio_en_hundredweight>(), 50.80234544);
    EXPECT_DOUBLE_EQ(decimal<ratio_en_quarter>(), 12.70058636);
    EXPECT_DOUBLE_EQ(decimal<ratio_en_stone>(), 6.35029318);
    EXPECT_DOUBLE_EQ(decimal<ratio_en_fluid_dram>(), 3.5516328125 / 1000 / 1000);
    EXPECT_DOUBLE_EQ(decimal<ratio_en_fluid_ounce>(), 28.4130625 / 1000 / 1000);
    EXPECT_DOUBLE_EQ(decimal<ratio_en_gill>(), 142.0653125 / 1000 / 1000);
    EXPECT_DOUBLE_EQ(decimal<ratio_en_cup>(), 284.130625 / 1000 / 1000);
    EXPECT_DOUBLE_EQ(decimal<ratio_en_pint>(), 568.26125 / 1000 / 1000);
    EXPECT_DOUBLE_EQ(decimal<ratio_en_quart>(), 1.1365225 / 1000);
    EXPECT_DOUBLE_EQ(decimal<ratio_en_gallon>(), 4.54609 / 1000);
    EXPECT_DOUBLE_EQ(decimal<ratio_en_peck>(), 9.09218 / 1000);
    EXPECT_DOUBLE_EQ(decimal<ratio_en_bushel>(), 36.36872 / 1000);
}

TEST(Quantity, USUnits)
{
    EXPECT_DOUBLE_EQ(decimal<ratio_us_hundredweight>(), 45.359237);
    EXPECT_DOUBLE_EQ(decimal<ratio_us_fluid_dram>(), 3.6966911953125 / 1000 / 1000);
    EXPECT_DOUBLE_EQ(decimal<ratio_us_fluid_ounce>(), 29.5735295625 / 1000 / 1000);
    EXPECT_DOUBLE_EQ(decimal<ratio_us_teaspoons>(), 4.92892159375 / 1000 / 1000);
    EXPECT_DOUBLE_EQ(decimal<ratio_us_tablespoons>(), 14.78676478125 / 1000 / 1000);
    EXPECT_DOUBLE_EQ(decimal<ratio_us_gill>(), 118.29411825 / 1000 / 1000);
    EXPECT_DOUBLE_EQ(decimal<ratio_us_cup>(), 236.5882365 / 1000 / 1000);
    EXPECT_DOUBLE_EQ(decimal<ratio_us_pint>(), 473.176473 / 1000 / 1000);
    EXPECT_DOUBLE_EQ(decimal<ratio_us_quart>(), 946.352946 / 1000 / 1000);
    EXPECT_DOUBLE_EQ(decimal<ratio_us_gallon>(), 3.785411784 / 1000);

    EXPECT_DOUBLE_EQ(decimal<ratio_us_dry_pint>(), 0.5506104713575 / 1000);
    EXPECT_DOUBLE_EQ(decimal<ratio_us_dry_quart>(), 1.101220942715 / 1000);
    EXPECT_DOUBLE_EQ(decimal<ratio_us_dry_gallon>(), 4.40488377086 / 1000);
    EXPECT_DOUBLE_EQ(decimal<ratio_us_dry_peck>(), 8.80976754172 / 1000);
    EXPECT_DOUBLE_EQ(decimal<ratio_us_bushel>(), 35.23907016688 / 1000);
}
