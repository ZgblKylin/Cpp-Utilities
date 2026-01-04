#ifndef CPP_UTILITIES_DIMENSIONALANALYSIS_HPP
#define CPP_UTILITIES_DIMENSIONALANALYSIS_HPP

#include "../Common.h"
#include <tuple>
#include <ratio>
#include <cmath>

/**
 * \defgroup DimensionalAnalysis Dimensional Analysis
 * \brief Helper classes, typedefs and functions for dimensional analyse.
 * \details
 *   Provide helperful functionalities for dimensional analysis with
 *   pros:\n
 *   **Zero-cost abstraction** Using template to encapsulate
 *   functionalities, grants same memory-layout and runtime performance
 *   as primitive types.\n
 *   **Strong typed** All values in dimensional objects has its unique
     **unit**, calculation between them is guaranteed by compiler
 *   warning and **error**.\n
 *   **Non-standard unit** of same "type", like `yard` vs `meter`, is
 *   supported, and will be regarded like **same** unit, `std::ratio` is
 *   used for modifier of the unit. Calculations between same unit with
 *   different ratio is safe, ratios of each operand will be performed
 *   into the calculation.\n
 *   **Add/subtract** with **same** unit is allowed, whereas with
 *   different units will cause compile error.\n
 *   **Multiply/divide** values with **any** units is safe, a new value
 *   with **new unit** will be generated.\n
 *   **User defined unit** is supported. Helper types is provided to
 *   derive new unit from existsing units.\n
 *   \n
 *   **Sample Code**\n
 *   ```cpp
 *   using Dimensional::Quantity;
 *   using Dimensional::quantity_cast;
 *   Quantity<double, Dimensional::length> meters(1);
 *   Quantity<double, Dimensional::length, Dimensional::ratio_yard> yards(1.0);
 *   meters += yards;
 *   yards = meters;
 *   // 2.09361yard 1.9144meter 1.9144meter
 *   std::cout << yards.value() << "yard "
 *             << yards.standard_value() << "meter "
 *             << meters.value() << "meter" << std::endl;
 *   auto speed = yards / Quantity<double, Dimensional::time>(1);
 *   using ratio_km_per_h = std::ratio_divide<std::ratio<1000>, std::ratio<3600>>;
 *   using ratio_mile_per_h = std::ratio_divide<Dimensional::ratio_mile, std::ratio<3600>>;
 *   // 0.836127m/s 3.01006km/h 1.87036mile/h
 *   std::cout << speed.value() << "m/s "
 *             << quantity_cast<ratio_km_per_h>(speed).value() << "km/h "
 *             << quantity_cast<ratio_mile_per_h>(speed).value() << "mile/h"
 *             << std::endl;
 *   ```
 * @{
 */

UTILITIES_NAMESPACE_BEGIN

/**
 * \namespace Dimensional
 * \brief Namespace for all classes, typedefs and functions of dimensional
 *        analyse. See \ref DimensionalAnalysis for more instrucion.
 * \warning `using namespace` is not recommend, because some classes and typedefs
 *          will duplicate with existing symbols. Consider `using` keyword with
 *          specific symbol like `using Dimensional::Quantity` instead.
 * @{
 */
namespace Dimensional {
/**
 * \brief The Unit struct is used to describe physical units.
 * \tparam length               Power factor of length unit.
 * \tparam mass                 Power factor of length mass unit.
 * \tparam time                 Power factor of length time unit.
 * \tparam current              Power factor of length current unit.
 * \tparam temperature          Power factor of length temperature unit.
 * \tparam amountOfSubstance    Power factor of amount_of_substance unit.
 * \tparam luminousIntensity    Power factor of luminous_intensity unit.
 * \details
 *   This template use 7 international base units to describe all physical units,
 *   to guarantee strong-typed unit analysis.\n
 *   Directly use this struct is not suggested, use `UnitMultiply`,
 *   `UnitDivide`, `UnitPower` and `UnitRoot` with `typedef/using` to
 *   generate derived unit with exisiting units.\n
 * sa DimensionalAnalysis, Quantity
 */
template<int length, int mass, int time, int current, int temperature, int amountOfSubstance, int luminousIntensity>
struct Unit
{
    /** \brief Type of the struct itself. */
    using type = Unit<length, mass, time, current, temperature, amountOfSubstance, luminousIntensity>;
#if __cplusplus >= 201402L
    /** \brief Power factors of 7 base units. */
    static constexpr std::tuple<int, int, int, int, int, int, int> factors{
        length, mass, time, current, temperature, amountOfSubstance, luminousIntensity
    };
#else
    /** \brief Power factors of 7 base units. */
    static const std::tuple<int, int, int, int, int, int, int> factors;
#endif
    /** \brief Power factor of length unit. */
    static constexpr int factorLength               = length;
    /** \brief Power factor of length mass unit. */
    static constexpr int factorMass                 = mass;
    /** \brief Power factor of length time unit. */
    static constexpr int factorTime                 = time;
    /** \brief Power factor of length current unit. */
    static constexpr int factorCurrent              = current;
    /** \brief Power factor of length temperature unit. */
    static constexpr int factorTemperature          = temperature;
    /** \brief Power factor of amount_of_substance unit unit. */
    static constexpr int factorAmountOfSubstance    = amountOfSubstance;
    /** \brief Power factor of luminous_intensity unit unit. */
    static constexpr int factorLuminousIntensity    = luminousIntensity;
};
#if __cplusplus < 201703L // Definition of static constexpr member is deprecated since C++17.
#if __cplusplus >= 201402L
template<int length, int mass, int time, int current, int temperature, int amountOfSubstance, int luminousIntensity>
constexpr std::tuple<int, int, int, int, int, int, int> Unit<length, mass, time, current, temperature, amountOfSubstance, luminousIntensity>::factors;
#else
template<int length, int mass, int time, int current, int temperature, int amountOfSubstance, int luminousIntensity>
const std::tuple<int, int, int, int, int, int, int> Unit<length, mass, time, current, temperature, amountOfSubstance, luminousIntensity>::factors{
    length, mass, time, current, temperature, amountOfSubstance, luminousIntensity
};
#endif
template<int length, int mass, int time, int current, int temperature, int amountOfSubstance, int luminousIntensity>
constexpr int Unit<length, mass, time, current, temperature, amountOfSubstance, luminousIntensity>::factorLength;
template<int length, int mass, int time, int current, int temperature, int amountOfSubstance, int luminousIntensity>
constexpr int Unit<length, mass, time, current, temperature, amountOfSubstance, luminousIntensity>::factorMass;
template<int length, int mass, int time, int current, int temperature, int amountOfSubstance, int luminousIntensity>
constexpr int Unit<length, mass, time, current, temperature, amountOfSubstance, luminousIntensity>::factorTime;
template<int length, int mass, int time, int current, int temperature, int amountOfSubstance, int luminousIntensity>
constexpr int Unit<length, mass, time, current, temperature, amountOfSubstance, luminousIntensity>::factorCurrent;
template<int length, int mass, int time, int current, int temperature, int amountOfSubstance, int luminousIntensity>
constexpr int Unit<length, mass, time, current, temperature, amountOfSubstance, luminousIntensity>::factorTemperature;
template<int length, int mass, int time, int current, int temperature, int amountOfSubstance, int luminousIntensity>
constexpr int Unit<length, mass, time, current, temperature, amountOfSubstance, luminousIntensity>::factorAmountOfSubstance;
template<int length, int mass, int time, int current, int temperature, int amountOfSubstance, int luminousIntensity>
constexpr int Unit<length, mass, time, current, temperature, amountOfSubstance, luminousIntensity>::factorLuminousIntensity;
#endif

/**
 * \name Unit Conversion
 * \relates Unit
 * \brief Helper typedef for unit conversion
 * @{
*/
/**
 * \brief The UnitMultiply type is an alias of multiply calculation with two
 *        units.
 * \tparam Unit1 First operand unit.
 * \tparam Unit2 Second operand unit.
 */
template<typename Unit1,  typename Unit2>
using UnitMultiply = Unit<
    Unit1::factorLength             + Unit2::factorLength,
    Unit1::factorMass               + Unit2::factorMass,
    Unit1::factorTime               + Unit2::factorTime,
    Unit1::factorCurrent            + Unit2::factorCurrent,
    Unit1::factorTemperature        + Unit2::factorTemperature,
    Unit1::factorAmountOfSubstance  + Unit2::factorAmountOfSubstance,
    Unit1::factorLuminousIntensity  + Unit2::factorLuminousIntensity
>;
/**
 * \brief The UnitDivide type is an alias of divide calculation with two units.
 * \tparam Unit1 First operand unit.
 * \tparam Unit2 Second operand unit.
 */
template<typename Unit1,  typename Unit2>
using UnitDivide = Unit<
    Unit1::factorLength             - Unit2::factorLength,
    Unit1::factorMass               - Unit2::factorMass,
    Unit1::factorTime               - Unit2::factorTime,
    Unit1::factorCurrent            - Unit2::factorCurrent,
    Unit1::factorTemperature        - Unit2::factorTemperature,
    Unit1::factorAmountOfSubstance  - Unit2::factorAmountOfSubstance,
    Unit1::factorLuminousIntensity  - Unit2::factorLuminousIntensity
>;
/**
 * \brief The UnitPow type is an alias of power calculation with two units.
 * \tparam U Operand unit.
 * \tparam n Factor of power calculation.
 */
template<typename U, int n>
using UnitPow = Unit<
    U::factorLength             * n,
    U::factorMass               * n,
    U::factorTime               * n,
    U::factorCurrent            * n,
    U::factorTemperature        * n,
    U::factorAmountOfSubstance  * n,
    U::factorLuminousIntensity  * n
>;
/**
 * \brief The UnitRoot type is an alias of power calculation with two units.
 * \tparam U Operand unit.
 * \tparam n Factor of root calculation.
 */
template<typename U, int n>
using UnitRoot = Unit<
    U::factorLength             / n,
    U::factorMass               / n,
    U::factorTime               / n,
    U::factorCurrent            / n,
    U::factorTemperature        / n,
    U::factorAmountOfSubstance  / n,
    U::factorLuminousIntensity  / n
>;
/** @} */

/**
 * \name Base Units
 * \relates Unit
 * \brief 7 international base units.
 * @{
*/
/**
 * \brief The scala type of non-unit.
*/
typedef Unit<0, 0, 0, 0, 0, 0, 0> Scala;
/**
 * \brief Length unit, called **meter**, with symbol `m`.
 */
typedef Unit<1, 0, 0, 0, 0, 0, 0> Length;
/**
 * \brief Mass unit, called **kilogram**, with symbol `kg`.
 */
typedef Unit<0, 1, 0, 0, 0, 0, 0> Mass;
/**
 * \brief Time unit, called **second**, with symbol `s`.
 */
typedef Unit<0, 0, 1, 0, 0, 0, 0> Time;
/**
 * \brief Electric current unit, called **ampere**, with symbol `A`.
 */
typedef Unit<0, 0, 0, 1, 0, 0, 0> Current;
/**
 * \brief Thermodynamic temperature unit, called **kelvin**, with symbol `K`.
 */
typedef Unit<0, 0, 0, 0, 1, 0, 0> Temperature;
/**
 * \brief Amount of substance unit, called **mole**, with symbol `mol`.
 */
typedef Unit<0, 0, 0, 0, 0, 1, 0> AmountOfSubstance;
/**
 * \brief Luminous intensity unit, called **candela**, with symbol `cd`.
*/
typedef Unit<0, 0, 0, 0, 0, 0, 1> LuminousIntensity;
/** @} */

/**
 * \name Derived Units
 * \relates Unit
 * \brief International System of Units derived from base units.
 * @{
 */
/**
 * \brief Speed unit, derived from \f$m/s\f$.
*/
typedef UnitDivide<Length, Time> Speed;
/**
 * \brief Acceleration unit, derived from \f$m/s^{2}\f$.
 */
typedef UnitDivide<Speed, Time> Acceleration;
/**
 * \brief Frequence unit, called **hertz**, with symbol `Hz`, derived from
 *        \f$s^{-1}\f$.
 */
typedef UnitDivide<Scala, Time> Frenquency;
// TODO radian
// TODO steradian
/**
 * \brief Force unit, called **newton**, with symbol `N`, derived from
 *        \f$kg \cdot m \cdot s^{-2}\f$.
 */
typedef UnitMultiply<Mass, Acceleration> Force;
/**
 * \brief Area unit, with symbol \f$m^{2}\f$.
 */
typedef UnitMultiply<Length, Length> Area;
/**
 * \brief Volume unit, with symbol \f$m^{3}\f$.
 */
typedef UnitMultiply<Area, Length> Volume;
/**
 * \brief Pressure unit, called **pascal**, with symbol `Pa`, derived from
 *        \f$N/m^{2}\f$ or \f$kg \cdot m^{-1} \cdot s^{-2}\f$.
 */
typedef UnitDivide<Force, Area> Pressure;
/**
 * \brief Enegy unit, called **joule**, with symbol `J`, derived from
 *        \f$N \cdot m\f$ or \f$kg \cdot m^{2} \cdot s^{-2}\f$.
 */
typedef UnitMultiply<Force, Length> Energy;
/**
 * \brief Power unit, called **watt**, with symbol `W`, derived from \f$J/s\f$
 *        or \f$kg \cdot m^{2} \cdot s^{-3}\f$.
 */
typedef UnitDivide<Energy, Time> Power;
/**
 * \brief Charge unit, called **coulomb**, with symbol `C`, derived from
 *        \f$s \cdot A\f$.
 */
typedef UnitMultiply<Time, Current> Charge;
/**
 * \brief Voltage unit, called **volt**, with symbol `V`, derived from \f$W/A\f$
 *        or \f$kg \cdot m^{2} \cdot s^{-3} \cdot A^{-1}\f$.
 */
typedef UnitDivide<Power, Current> Voltage;
/**
 * \brief Elelctric capacitance unit, called **farad**, with symbol `F`, derived
 *        from \f$C/V\f$ or \f$kg^{-1} \cdot m^{-2} \cdot s^{4} \cdot A^{2}\f$.
 */
typedef UnitDivide<Charge, Voltage> ElelctricCapacitance;
/**
 * \brief Electric resistance unit, called **ohm**, with symbol `Ω`, derived
 *        from \f$V/A\f$ or \f$kg \cdot m^{2} \cdot s^{-3} \cdot A^{-2}\f$.
 */
typedef UnitDivide<Voltage, Current> ElectricResistance;
/**
 * \brief Electric conductance unit, called **simens**, with symbol `S`, derived
 *        from \f$1/\Omega\f$ or
 *        \f$kg^{-1} \cdot m^{-2} \cdot s^{3} \cdot A^{2}\f$.
 */
typedef UnitDivide<Scala, ElectricResistance> ElelctricConductance;
/**
 * \brief Magnetic flux unit, called **webber**, with symbol Wb**, derived from
 *        \f$V \cdot s\f$ or \f$kg \cdot m^{2} \cdot s^{-2} \cdot A^{-1}\f$.
 */
typedef UnitMultiply<Voltage, Time> MagneticFlux;
/**
 * \brief Magnet flux density unit, called **tesla**, with symbol `T`, derived
 *        from \f$Wb/m^{2}\f$ or \f$kg \cdot s^{-2} \cdot A^{-1}\f$.
 */
typedef UnitDivide<MagneticFlux, Area> MagnetFluxDensity;
/**
 * \brief Electric unit, called **henry**, with symbol `H`, derived from
 *        \f$Wb/A\f$ or \f$kg \cdot m^{2} \cdot s^{-2} \cdot A^{-2}\f$.
 */
typedef UnitDivide<MagneticFlux, Current> Inductance;
/**
 * \brief Luminous flux unit, called **lumen**, with symbol `lm`, derived from
 *        \f$cd \cdot sr\f$.
 */
typedef LuminousIntensity Luminous;
/**
 * \brief Illuminance unit, called **lux**, with symbol `ls`, derived from
 *        \f$lm/m^{2}\f$ or \f$m^{-2} \cdot cd\f$.
 */
typedef UnitDivide<LuminousIntensity, Area> Illuminance;
/**
 * \brief Radioactivity unit of decays per second, called **becquerel**, with
 *        symbol `Bq` derived from \f$s^{-1}\f$.
 */
typedef UnitDivide<Scala, Time> Radioactivity;
/**
 * \brief Absorbed dose unit of ionising radiation, called **gray**, with symbol
 *        `Gy`, derived from \f$J/kg\f$ or \f$m^{2} \cdot s^{-2}\f$.
 */
typedef UnitDivide<Energy, Mass> AbsorbedDose;
/**
 * \brief Equivalent dose unit of ionising radiation, called **sievert**, with
 *        symbol `Sv`, derived from \f$J/kg\f$ or \f$m^{2} \cdot s^{-2}\f$.
 */
typedef UnitDivide<Energy, Mass> EquivalentDose;
/**
 * \brief Catalytic activity unit called **katal**, with symbol `kat`, derived
 *        from \f$mol \cdot s^{-1}\f$.
 */
typedef UnitDivide<AmountOfSubstance, Time> CatalyticActivity;
/** @} */

// Forward declarations
template<typename T, typename Unit, typename Ratio>
class Quantity;
template<typename NewRatio, typename T, typename U, typename Ratio>
Quantity<T, U, NewRatio> quantity_cast(Quantity<T, U, Ratio> x);

/**
 * \brief The Quantity struct is used to describe arithmetic values with units.
 * \tparam T        Arithmetic type for value.
 * \tparam U        Unit type for this physical quantity.
 * \tparam Ratio    Conversion ratio for nonstandard units such as feet or yard.
 * \details
 *   This template guarantee strong-typed safe calculation of physical values.\n
 *   Variables with different unit cannot add, subtract and compare with each
 *   other. Multiply, divide, power, root calculation will generate value with
 *   unit.\n
 *   Value is allowd to be described with different Ratio. Calculation with
 *   different ratios is safe, and result has same Ratio of first operand.\n
 * \sa DimensionalAnalysis, Unit
 */
template<typename T, typename U, typename Ratio = std::ratio<1>>
class Quantity
{
public:
    /** \brief Self type for this quantity struct. */
    using type = Quantity<T, U, Ratio>;
    /** \brief Value type for this quantity struct. */
    using value_type = T;
    /** \brief Unit type for this quantity struct. */
    using unit_type = U;
    /** \brief Ratio type for this quantity struct. */
    using ratio_type = Ratio;

    /**
     * \brief Default constructor
     * \param x Initial value, default is 0.
     */
    inline explicit Quantity(T x = 0)
        : v(x)
    {}

    /**
     * \brief Copy constructor
     * \param other Operand to be copied.
     */
    inline Quantity(const Quantity<T, U, Ratio>& other)
        : v(other.v)
    {}

    /**
     * \brief Copy constructor from same quantity type with different ratio.
     * \tparam OtherRatio   ratio of input operand.
     * \param other         Operand to be copied.
     */
    template<typename OtherRatio>
    inline Quantity(const Quantity<T, U, OtherRatio>& other)
        : v(quantity_cast<Ratio>(other).value())
    {}

    /**
     * \brief Get underlying value of the quantity, value represented with
     *        current unit and ratio will be returned.
     * \warning Return value is under the `Ratio` and may not be standard value.
     * \return underlying Value with `Ratio` converted.
     */
    inline T value() const
    { return v; }

    /**
     * \brief Set underlying value of the quantity.
     * \param value Value represented by `Ratio`.
     * \warning `value` is under the `Ratio` and may not be standard value.
     */
    inline void set_value(T value)
    { v = value;; }

    /**
     * \brief Get standard value of the quantity, `Ratio` is reverted to
     *        `std::ratio<1>`.
     * \return Standard value with `Ratio` reverted to `std::ratio<1>`.
     */
    inline T standard_value() const
    { return quantity_cast<std::ratio<1>>(*this).value(); }

    /**
     * \brief Get standard value of the quantity, `Ratio` is reverted to
     *        `std::ratio<1>`.
     * \return Standard value with `Ratio` reverted to `std::ratio<1>`.
     */
    inline void set_standard_value(T value)
    { v = quantity_cast<Ratio>(Quantity<T, U>(value)).value(); }

    /**
     * \brief Add & assignment operator overload, add value from same type with
     *        **maybe** different ratio, operand value will be converted by
     *        `ratio_type` before performing add.
     * \tparam  OtherRatio  Ratio of input operand.
     * \param   other       Operand to assign from.
     * \return Reference to self with value added.
     */
    template<typename OtherRatio>
    inline Quantity<T, U, Ratio>&
    operator+=(const Quantity<T, U, OtherRatio>& other)
    { v += quantity_cast<Ratio>(other).v; return *this; }

    /**
     * \brief Assisgnment operator overload.
     * \param other Operand to assign from.
     * \return Reference to self with value assigned.
     */
    inline Quantity<T, U, Ratio>&
    operator=(const Quantity<T, U, Ratio>& other)
    { v = other.v; return *this; }

    /**
     * \brief Assisgnment operator overload, assign value from same type but
     *        different ratio, operand value will be converted by
     *        `ratio_type` before performing assignment.
     * \tparam  OtherRatio  Ratio of input operand.
     * \param   other       Operand to assign from.
     * \return Reference to self with value assigned.
     */
    template<typename OtherRatio>
    inline Quantity<T, U, Ratio>&
    operator=(const Quantity<T, U, OtherRatio>& other)
    { v = quantity_cast<Ratio>(other).v; return *this; }

    /**
     * \brief Equality operator overload, value will be convered to same ratio
     *        before comparison.
     * \tparam  OtherRatio  Ratio of input operand.
     * \param   other       Other operand to be compared.
     * \return Boolean value for result of comparison.
     */
    template<typename OtherRatio>
    inline bool operator==(const Quantity<T, U, OtherRatio>& other) const
    { return v == quantity_cast<Ratio>(other).v; }

    /**
     * \brief Inequality operator overload, value will be convered to same ratio
     *        before comparison.
     * \tparam  OtherRatio  Ratio of input operand.
     * \param   other       Other operand to be compared.
     * \return Boolean value for result of comparison.
     */
    template<typename OtherRatio>
    inline bool operator!=(const Quantity<T, U, OtherRatio>& other) const
    { return !(*this == other); }

    /**
     * \brief Less than operator overload, value will be convered to same ratio
     *        before comparison.
     * \tparam  OtherRatio  Ratio of input operand.
     * \param   other       Other operand to be compared.
     * \return Boolean value for result of comparison.
     */
    template<typename OtherRatio>
    inline bool operator<(const Quantity<T, U, OtherRatio>& other) const
    { return v < quantity_cast<Ratio>(other).v; }

    /**
     * \brief Less or equal operator overload, value will be convered to same
     *        ratio before comparison.
     * \tparam  OtherRatio  Ratio of input operand.
     * \param   other       Other operand to be compared.
     * \return Boolean value for result of comparison.
     */
    template<typename OtherRatio>
    inline bool operator<=(const Quantity<T, U, OtherRatio>& other) const
    { return (*this < other) || (*this == other); }

    /**
     * \brief Larger than operator overload, value will be convered to same
     *        ratio before comparison.
     * \tparam  OtherRatio  Ratio of input operand.
     * \param   other       Other operand to be compared.
     * \return Boolean value for result of comparison.
     */
    template<typename OtherRatio>
    inline bool operator>(const Quantity<T, U, OtherRatio>& other) const
    { return !(*this <= other); }

    /**
     * \brief Larger or equal operator overload, value will be convered to same
     *        ratio before comparison.
     * \tparam  OtherRatio  Ratio of input operand.
     * \param   other       Other operand to be compared.
     * \return Boolean value for result of comparison.
     */
    template<typename OtherRatio>
    inline bool operator>=(const Quantity<T, U, OtherRatio>& other) const
    { return !(*this < other); }

private:
    T v;
};

/**
 * \relates Quantity
 * \brief Plus operator overload, values will be converted to same ratio before
 *        calculation, both inputs should have same value type and unit type.
 * \tparam  T       Value type of operands.
 * \tparam  U       Unit of operands.
 * \tparam  Ratio1  Ratio of first operand.
 * \tparam  Ratio2  Ratio of second operand.
 * \param   lhs     First operand.
 * \param   rhs     Second operand.
 * \return Calculation result represened with `Ratio1.`
 */
template<typename T, typename U, typename Ratio1, typename Ratio2>
inline Quantity<T, U, Ratio1>
operator+(Quantity<T, U, Ratio1> lhs, Quantity<T, U, Ratio2> rhs)
{
    return Quantity<T, U, Ratio1>(lhs.value()
                                  + quantity_cast<Ratio1>(rhs).value());
}

/**
 * \relates Quantity
 * \brief Subtract operator overload, values will be converted to same ratio
 *        before calculation, both inputs should have same value type and unit
 *        type.
 * \tparam  T       Value type of operands.
 * \tparam  U       Unit of operands.
 * \tparam  Ratio1  Ratio of first operand.
 * \tparam  Ratio2  Ratio of second operand.
 * \param   lhs      First operand.
 * \param   rhs      Second operand.
 * \return Calculation result represened with `Ratio1.`
 */
template<typename T, typename U, typename Ratio1, typename Ratio2>
inline Quantity<T, U, Ratio1>
operator-(Quantity<T, U, Ratio1>& lhs, Quantity<T, U, Ratio2> rhs)
{
    return Quantity<T, U, Ratio1>(lhs.value()
                                  - quantity_cast<Ratio1>(rhs).value());
}

/**
 * \relates Quantity
 * \brief Multiply operator overload, values will be converted to same ratio
 *        before calculation, inputs can have different unit type, a new unit
 *        type will be generated.
 * \tparam  T       Value type of operands.
 * \tparam  Unit1   Unit of first operand.
 * \tparam  Ratio1  Ratio of first operand.
 * \tparam  Unit2   Unit of second operand.
 * \tparam  Ratio2  Ratio of second operand.
 * \param   lhs     First operand.
 * \param   rhs     Second operand.
 * \return Calculation result represened with `Ratio1.`
 */
template<typename T, typename Unit1, typename Ratio1, typename Unit2, typename Ratio2>
inline Quantity<T, UnitMultiply<Unit1, Unit2>, Ratio1>
operator*(Quantity<T, Unit1, Ratio1> lhs, Quantity<T, Unit2, Ratio2> rhs)
{
    using dim = UnitMultiply<Unit1, Unit2>;
    return Quantity<T, dim, Ratio1>(lhs.value()
                                    * quantity_cast<Ratio1>(rhs).value());
}

/**
 * \relates Quantity
 * \brief Divide operator overload, values will be converted to same ratio
 *        before calculation, inputs can have different unit type, a new unit
 *        type will be generated.
 * \tparam  T       Value type of operands.
 * \tparam  Unit1   Unit of first operand.
 * \tparam  Ratio1  Ratio of first operand.
 * \tparam  Unit2   Unit of second operand.
 * \tparam  Ratio2  Ratio of second operand.
 * \param   lhs     First operand.
 * \param   rhs     Second operand.
 * \return Calculation result represened with `Ratio1.`
 */
template<typename T, typename Unit1, typename Ratio1, typename Unit2, typename Ratio2>
inline Quantity<T, UnitDivide<Unit1, Unit2>, Ratio1>
operator/(Quantity<T, Unit1, Ratio1> lhs, Quantity<T, Unit2, Ratio2> rhs)
{
    using dim = UnitDivide<Unit1, Unit2>;
    return Quantity<T, dim, Ratio1>(lhs.value()
                                    / quantity_cast<Ratio1>(rhs).value());
}

/**
 * \relates Quantity
 * \brief Power calculation, performed both on value and unit, ratio will be
 *        casted to `std::ratio<1>`.
 * \tparam  factor  Factor of power calculation.
 * \tparam  T       Value type of operands.
 * \tparam  U       Unit of operand.
 * \tparam  Ratio   Ratio of operand.
 * \param   x       Quantity operand.
 * \return Calculation result with power performed both on value and unit.
 */
template<int factor, typename T, typename U, typename Ratio>
inline Quantity<T, UnitPow<U, factor>, std::ratio<1>>
pow(const Quantity<T, U, Ratio>& x)
{
    return Quantity<
               T,
               UnitPow<U, factor>,
               std::ratio<1>
           >(std::pow(quantity_cast<std::ratio<1>>(x).value(),
                      factor));
}

/**
 * \relates Quantity
 * \brief Root calculation, performed both on value and unit, ratio will be
 *        casted to `std::ratio<1>`.
 * \tparam  factor  Factor of root calculation.
 * \tparam  T       Value type of operands.
 * \tparam  U       Unit of operand.
 * \tparam  Ratio   Ratio of operand.
 * \param   x       Quantity operand.
 * \return Calculation result with root performed both on value and unit.
 */
template<int factor, typename T, typename U, typename Ratio>
inline Quantity<T, UnitRoot<U, factor>, std::ratio<1>>
root(const Quantity<T, U, Ratio>& x)
{
    return Quantity<
               T,
               UnitRoot<U, factor>,
               std::ratio<1>
           >(std::pow(quantity_cast<std::ratio<1>>(x).value(),
                      1.0 / factor));
}

/**
 * \relates Quantity
 * \brief Cast a quantity to another ratio, value will be converted too.
 *        Sample code for convert to standard value:
 *        `b = quantity_cast<std::ratio<1>>(a);`
 * \tparam  NewRatio    Ratio to be cast to.
 * \tparam  T           Value type of input.
 * \tparam  U           Unit of input.
 * \tparam  Ratio       Ratio of input.
 * \param   x           Operand to be cast.
 * \return New quantity cast to `NewRatio`.
 */
template<typename NewRatio, typename T, typename U, typename Ratio>
inline Quantity<T, U, NewRatio> quantity_cast(Quantity<T, U, Ratio> x)
{
    using ratio_div = std::ratio_divide<Ratio, NewRatio>;
    T val = x.value()
            * (static_cast<long double>(ratio_div::num) / ratio_div::den);
    return Quantity<T, U, NewRatio>(val);
}

/**
 * \name Common Ratio
 * \relates Quantity
 * \brief Common ratios for calculation.
 * @{
 */
/**
 * \brief Ratio to display π in approximate fraction,
 *        with high presion up to \f$3.14159266096017653069\f$.
 */
typedef std::ratio<80813362, 25723692> ratio_PI;
/**
 * \brief Ratio to convert degree into radian,
 *        using equation \f$rad = degree * \frac{pi}{180}\f$.
 */
typedef std::ratio_divide<ratio_PI, std::ratio<180>> ratio_degree;
/** @} */

/**
  * \name Chinese Units
  * \relates Quantity
  * \brief Ratios for Chinese units of mass and length.
  * @{
  */
/**
 * \brief Ratio to convert to meter.
 *        \f$1 li = 15 yin = 500 m\f$.
 */
typedef std::ratio<500, 1> ratio_length_li;
/**
 * \brief Ratio to convert to meter.
 *        \f$1 yin = \frac{1}{15} li = 10 zhang = 33.\overline{3} m\f$.
 */
typedef std::ratio_divide<ratio_length_li, std::ratio<15>> ratio_yin;
/**
 * \brief Ratio to convert to meter.
 *        \f$1 zhang = \frac{1}{10} yin = 10 chi = 3.\overline{3} m\f$.
 */
typedef std::ratio_divide<ratio_yin, std::ratio<10>> ratio_zhang;
/**
 * \brief Ratio to convert to meter.
 *        \f$1 xun = 1/2 zhang = 5 chi = 1.\overline{6} m\f$.
 */
typedef std::ratio_divide<ratio_zhang, std::ratio<2>> ratio_xun;
/**
 * \brief Ratio to convert to meter.
 *        \f$1 chi = 33.\overline{3} cm\f$.
 */
typedef std::ratio_divide<ratio_zhang, std::ratio<10>> ratio_chi;
/**
 * \brief Ratio to convert to meter.
 *        \f$1 cun = \frac{1}{10} chi = 3.\overline{3} cm\f$.
 */
typedef std::ratio_divide<ratio_chi, std::ratio<10>> ratio_cun;
/**
 * \brief Ratio to convert to meter.
 *        \f$1 fen = \frac{1}{10} cun = 3.\overline{3} mm\f$.
 */
typedef std::ratio_divide<ratio_cun, std::ratio<10>> ratio_length_fen;
/**
 * \brief Ratio to convert to meter.
 *        \f$1 li = \frac{1}{10} fen = 333.\overline{3} um\f$.
 */
typedef std::ratio_divide<ratio_length_fen, std::ratio<10>> ratio_length_li2;
/**
 * \brief Ratio to convert to meter.
 *        \f$1 hao = \frac{1}{10} li = 33.\overline{3} um\f$.
 */
typedef std::ratio_divide<ratio_length_li2, std::ratio<10>> ratio_length_hao;
/**
 * \brief Ratio to convert to meter.
 *        \f$1 si = \frac{1}{10} hao = 3.\overline{3} um\f$.
 */
typedef std::ratio_divide<ratio_length_hao, std::ratio<10>> ratio_length_si;
/**
 * \brief Ratio to convert to meter.
 *        \f$1 hu = \frac{1}{10} si = 333.\overline{3} nm\f$.
 */
typedef std::ratio_divide<ratio_length_si, std::ratio<10>> ratio_length_hu;
/**
 * \brief Ratio to convert to suqaremeter.
 *        \f$1 qing = 100 mu = 66666.\overline{6} m^{2}\f$.
 */
typedef std::ratio<200000, 3> ratio_qing;
/**
 * \brief Ratio to convert to suqaremeter.
 *        \f$1 mu = 240 gong = 666.\overline{6} m^{2}\f$.
 */
typedef std::ratio<2000, 3> ratio_mu;
/**
 * \brief Ratio to convert to suqaremeter.
 *        \f$1 gong = 1 xun^{2} = 2.\overline{7} m^{2}\f$.
 */
typedef std::ratio_divide<ratio_mu, std::ratio<240>> ratio_gong;
/**
 * \brief Ratio to convert to kilogram.
 *        \f$1 dan = 50 kg\f$.
 */
typedef std::ratio<50> ratio_dan;
/**
 * \brief Ratio to convert to kilogram, also called `market carry`.
 *        \f$1 jin = 500 g catty\f$.
 */
typedef std::ratio<1, 2> ratio_jin;
/**
 * \brief Ratio to convert to kilogram.
 *        \f$1 liang = \frac{1}{10} jin = 50 g\f$.
 */
typedef std::ratio_divide<ratio_jin, std::ratio<10>> ratio_liang;
/**
 * \brief Ratio to convert to kilogram.
 *        \f$1 qian = \frac{1}{10} liang = 5 g\f$.
 */
typedef std::ratio_divide<ratio_liang, std::ratio<10>> ratio_qian;
/**
 * \brief Ratio to convert to kilogram.
 *        \f$1 fen = \frac{1}{10} qian = 500 mg\f$.
 */
typedef std::ratio_divide<ratio_qian, std::ratio<10>> ratio_mass_fen;
/**
 * \brief Ratio to convert to kilogram.
 *        \f$1 li = \frac{1}{10} fen = 50 mg\f$.
 */
typedef std::ratio_divide<ratio_mass_fen, std::ratio<10>> ratio_mass_li;
/**
 * \brief Ratio to convert to kilogram.
 *        \f$1 hao = \frac{1}{10} li = 5 mg\f$.
 */
typedef std::ratio_divide<ratio_mass_li, std::ratio<10>> ratio_mass_hao;
/**
 * \brief Ratio to convert to kilogram.
 *        \f$1 si = \frac{1}{10} hao = 500 ug\f$.
 */
typedef std::ratio_divide<ratio_mass_hao, std::ratio<10>> ratio_mass_si;
/**
 * \brief Ratio to convert to kilogram.
 *        \f$1 hu = \frac{1}{10} si = 50 ug\f$.
 */
typedef std::ratio_divide<ratio_mass_si, std::ratio<10>> ratio_mass_hu;
/** @} */

/**
 * \name Yard Pound
 * \relates Quantity
 * \brief Ratios for International Avoirdupois System of yard and pound units.
 * @{
 */
/**
 * \brief Ratio to convert to meter.
 *        \f$1 mile = 1.609344 km\f$.
 */
typedef std::ratio_multiply<std::ratio<1609344ll, 1000000ll>, std::kilo> ratio_mile;
/**
 * \brief Ratio to convert to meter.
 *        \f$1 furlong = 201.168 m\f$.
 */
typedef std::ratio_divide<ratio_mile, std::ratio<8>> ratio_furlong;
/**
 * \brief Ratio to convert to meter.
 *        \f$1 chain = \frac{1}{10} furlong = \frac{1}{80} mile = 20.1168 m\f$.
 */
typedef std::ratio_divide<ratio_furlong, std::ratio<10>> ratio_chain;
/**
 * \brief Ratio to convert to meter.
 *        \f$1 yard = \frac{1}{22} chain = \frac{1}{1760} mile = 0.9144 m\f$.
 */
typedef std::ratio_divide<ratio_chain, std::ratio<22>> ratio_yard;
/**
 * \brief Ratio to convert to meter.
 *        \f$1 nail = \frac{1}{16} yard = \frac{9}{4} inch = 5.715 cm\f$.
 */
typedef std::ratio_divide<ratio_yard, std::ratio<16>> ratio_nail;
/**
 * \brief Ratio to convert to meter.
 *        \f$1 feet = \frac{1}{3} yard = 30.48 cm\f$.
 */
typedef std::ratio_divide<ratio_yard, std::ratio<3>> ratio_feet;
/**
 * \brief Ratio to convert to meter.
 *        \f$1 inch = \frac{1}{12} feet = 2.54 cm\f$.
 */
typedef std::ratio_divide<ratio_feet, std::ratio<12>> ratio_inch;
/**
 * \brief Ratio to convert to meter, with symbol `pc`.
 *        \f$1 pica = \frac{1}{6} inch = 4.2\overline{3} mm\f$.
 */
typedef std::ratio_divide<ratio_inch, std::ratio<6>> ratio_pica;
/**
 * \brief Ratio to convert to meter, with symbol 'pt'.
 *        \f$1 point = \frac{1}{12} pica = 0.352\overline{7} mm\f$.
 */
typedef std::ratio_divide<ratio_pica, std::ratio<12>> ratio_point;
/**
 * \brief Ratio to convert to kilogram.
 *        \f$1 longton = 2240 pound = 1016.0469088 kg\f$.
 */
typedef std::ratio<10160469088, 10000000> ratio_longton;
/**
 * \brief Ratio to convert to kilogram.
 *        \f$1 shortton = 2000 pound = 907.18474 kg\f$.
 */
typedef std::ratio<90718474, 100000> ratio_shortton;
/**
 * \brief Ratio to convert to kilogram.
 *        \f$1 long\ hundredweight = 112 pound = 50.80234544 kg\f$.
 */
typedef std::ratio<5080234544, 100000000> ratio_long_hundredweight;
/**
 * \brief Ratio to convert to kilogram.
 *        \f$1 short\ hundredweight = 100 pound = 45.359237 kg\f$.
 */
typedef std::ratio<45359237, 1000000> ratio_short_hundredweight;
/**
 * \brief Ratio to convert to kilogram.
 *        \f$1 pound = 0.45359237 kg\f$.
 */
typedef std::ratio<45359237, 100000000> ratio_pound;
/**
 * \brief Ratio to convert to kilogram.
 *        \f$1 ounce = \frac{1}{16} pound = 28.349523125 g\f$.
 */
typedef std::ratio_divide<ratio_pound, std::ratio<16>> ratio_ounce;
/**
 * \brief Ratio to convert to kilogram.
 *        \f$1 drachm = \frac{1}{16} ounce = 1.7718451953125 g\f$.
 */
typedef std::ratio_divide<ratio_ounce, std::ratio<16>> ratio_drachm;
/**
 * \brief Ratio to convert to kilogram.
 *        \f$1 grain = 64.79891 mg\f$.
 */
typedef std::ratio_multiply<std::ratio<6479891, 100000ll>, std::micro> ratio_grain;
/** @} */

/**
 * \name Imperial Units
 * \relates Quantity
 * \brief Imperial units of mass and volume
 * @{
 */
/**
 * \brief Ratio to convert to kilogram.
 *        \f$1 Imperial\ hundredweight = 8 stone = 112 pound = 50.80234544 kg\f$.
 */
typedef ratio_long_hundredweight ratio_en_hundredweight;
/**
 * \brief Ratio to convert to kilogram.
 *        \f$1 quarter = 2 stone = 28 pound = 12.70058636 kg\f$.
 */
typedef std::ratio_multiply<ratio_pound, std::ratio<28>> ratio_en_quarter;
/**
 * \brief Ratio to convert to kilogram.
 *        \f$1 stone = 14 pound = 6.35029318 kg\f$.
 */
typedef std::ratio_multiply<ratio_pound, std::ratio<14>> ratio_en_stone;
/**
 * \brief Ratio to convert to cubicmeter.
 *        \f$1 fluid\ dram = 3.5516328125 mL\f$.
 */
typedef std::ratio_multiply<std::ratio<35516328125ll, 10000000000ll>, std::micro> ratio_en_fluid_dram;
/**
 * \brief Ratio to convert to cubicmeter.
 *        \f$1 fluid\ ounce = 8 fluind\ dram = 28.4130625 mL\f$.
 */
typedef std::ratio_multiply<ratio_en_fluid_dram, std::ratio<8>> ratio_en_fluid_ounce;
/**
 * \brief Ratio to convert to cubicmeter.
 *        \f$1 gill = 5 fluid\ ounce = 142.0653125 mL\f$.
 */
typedef std::ratio_multiply<ratio_en_fluid_ounce, std::ratio<5>> ratio_en_gill;
/**
 * \brief Ratio to convert to cubicmeter.
 *        \f$1 cup = 2 gill = 284.130625 mL\f$.
 */
typedef std::ratio_multiply<ratio_en_gill, std::ratio<2>> ratio_en_cup;
/**
 * \brief Ratio to convert to cubicmeter.
 *        \f$1 pint = 2 cup = 568.26125 mL\f$.
 */
typedef std::ratio_multiply<ratio_en_cup, std::ratio<2>> ratio_en_pint;
/**
 * \brief Ratio to convert to cubicmeter.
 *        \f$1 quart = 2 pint = 1.1365225 L\f$.
 */
typedef std::ratio_multiply<ratio_en_pint, std::ratio<2>> ratio_en_quart;
/**
 * \brief Ratio to convert to cubicmeter.
 *        \f$1 gallon = 4 quart = 4.54609 L\f$.
 */
typedef std::ratio_multiply<ratio_en_quart, std::ratio<4>> ratio_en_gallon;
/**
 * \brief Ratio to convert to cubicmeter.
 *        \f$1 peck = 2 gallon = 9.09218 L\f$.
 */
typedef std::ratio_multiply<ratio_en_gallon, std::ratio<2>> ratio_en_peck;
/**
 * \brief Ratio to convert to cubicmeter.
 *        \f$1 bushel = 4 peck = 36.36872 L\f$.
 */
typedef std::ratio_multiply<ratio_en_peck, std::ratio<4>> ratio_en_bushel;
/** @} */

/**
 * \name US Units
 * \relates Quantity
 * \brief US units of mass and volume
 * @{
 */
/**
 * \brief Ratio to convert to kilogram
 *        \f$1 US hundredweight = 100 pound = 45.359237 kg\f$.
 */
typedef ratio_short_hundredweight ratio_us_hundredweight;
/**
 * \brief Ratio to convert to cubicmeter.
 *        \f$1 fluid\ dram = 3.6966911953125 mL\f$.
 */
typedef std::ratio_multiply<std::ratio<36966911953125ll, 10000000000000ll>, std::micro> ratio_us_fluid_dram;
/**
 * \brief Ratio to convert to cubicmeter.
 *        \f$1 fluid\ ounce = 8 fluid\ dram = 29.5735295625 mL\f$.
 */
typedef std::ratio_multiply<ratio_us_fluid_dram, std::ratio<8>> ratio_us_fluid_ounce;
/**
 * \brief Ratio to convert to cubicmeter.
 *        \f$1 teaspoons = \frac{1}{6} fluid\ ounce = 4.92892159375 mL\f$.
 */
typedef std::ratio_divide<ratio_us_fluid_ounce, std::ratio<6>> ratio_us_teaspoons;
/**
 * \brief Ratio to convert to cubicmeter.
 *        \f$1 tablespoons = \frac{1}{2} fluid\ ounce = 14.78676478125 mL\f$.
 */
typedef std::ratio_divide<ratio_us_fluid_ounce, std::ratio<2>> ratio_us_tablespoons;
/**
 * \brief Ratio to convert to cubicmeter.
 *        \f$1 gill = 4 fluid\ ounce = 118.29411825 mL\f$.
 */
typedef std::ratio_multiply<ratio_us_fluid_ounce, std::ratio<4>> ratio_us_gill;
/**
 * \brief Ratio to convert to cubicmeter.
 *        \f$1 cup = 2 gill = 236.5882365 mL\f$.
 */
typedef std::ratio_multiply<ratio_us_gill, std::ratio<2>> ratio_us_cup;
/**
 * \brief Ratio to convert to cubicmeter.
 *        \f$1 pint = 2 cup = 473.176473 mL\f$.
 */
typedef std::ratio_multiply<ratio_us_cup, std::ratio<2>> ratio_us_pint;
/**
 * \brief Ratio to convert to cubicmeter.
 *        \f$1 quart = 2 pint = 946.352946 mL\f$.
 */
typedef std::ratio_multiply<ratio_us_pint, std::ratio<2>> ratio_us_quart;
/**
 * \brief Ratio to convert to cubicmeter.
 *        \f$1 gallon = 4 quart = 3.785411784 L\f$.
 */
typedef std::ratio_multiply<ratio_us_quart, std::ratio<4>> ratio_us_gallon;
/**
 * \brief Ratio to convert to cubicmeter.
 *        \f$1 dry\ pint = 0.5506104713575 L\f$.
 */
typedef std::ratio_multiply<std::ratio<5506104713575ll, 10000000000000ll>, std::milli> ratio_us_dry_pint;
/**
 * \brief Ratio to convert to cubicmeter.
 *        \f$1 dry\ quart = 2 dry\ pint = 1.101220942715 L\f$.
 */
typedef std::ratio_multiply<ratio_us_dry_pint, std::ratio<2>> ratio_us_dry_quart;
/**
 * \brief Ratio to convert to cubicmeter.
 *        \f$1 dry\ gallon = 4 dry\ quart = 4.40488377086 L\f$.
 */
typedef std::ratio_multiply<ratio_us_dry_quart, std::ratio<4>> ratio_us_dry_gallon;
/**
 * \brief Ratio to convert to cubicmeter.
 *        \f$1 dry\ peck = 2 dry\ gallon = 8.80976754172 L\f$.
 */
typedef std::ratio_multiply<ratio_us_dry_gallon, std::ratio<2>> ratio_us_dry_peck;
/**
 * \brief Ratio to convert to cubicmeter.
 *        \f$1 bushel = 4 dry\ peck = 35.23907016688 L\f$.
 */
typedef std::ratio_multiply<ratio_us_dry_peck, std::ratio<4>> ratio_us_bushel;
/** @} */
} // namespace Dimensional
/** @} */

UTILITIES_NAMESPACE_END

/** @} */

#endif  // CPP_UTILITIES_DIMENSIONALANALYSIS_HPP
