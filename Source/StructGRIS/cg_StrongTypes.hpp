/*
 This file is part of SpatGRIS.

 Developers: Samuel Béland, Olivier Bélanger, Nicolas Masson

 SpatGRIS is free software: you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation, either version 3 of the License, or
 (at your option) any later version.

 SpatGRIS is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.

 You should have received a copy of the GNU General Public License
 along with SpatGRIS.  If not, see <http://www.gnu.org/licenses/>.
*/

#pragma once

#include <JuceHeader.h>
#include <type_traits>
namespace gris
{
//==============================================================================
class StrongIndexBase
{
};

//==============================================================================
template<typename T, typename Dummy, T StartsAt>
class StrongIndex : public StrongIndexBase
{
    T mValue;

    static_assert(std::is_integral_v<T>, "Underlying types should be integrals.");

public:
    //==============================================================================
    using type = T;
    static constexpr auto OFFSET = StartsAt;
    //==============================================================================
    StrongIndex() = default;
    explicit constexpr StrongIndex(type const & value) : mValue(value) {}
    //==============================================================================
    [[nodiscard]] constexpr bool operator==(StrongIndex const & other) const { return mValue == other.mValue; }
    [[nodiscard]] constexpr bool operator!=(StrongIndex const & other) const { return mValue != other.mValue; }
    [[nodiscard]] constexpr bool operator<(StrongIndex const & other) const { return mValue < other.mValue; }
    [[nodiscard]] constexpr bool operator>(StrongIndex const & other) const { return mValue > other.mValue; }
    [[nodiscard]] constexpr bool operator<=(StrongIndex const & other) const { return mValue <= other.mValue; }
    [[nodiscard]] constexpr bool operator>=(StrongIndex const & other) const { return mValue >= other.mValue; }
    //==============================================================================
    [[nodiscard]] constexpr StrongIndex operator+(StrongIndex const & other) const noexcept
    {
        return StrongIndex{ mValue + other.mValue };
    }
    //==============================================================================
    [[nodiscard]] constexpr type const & get() const { return mValue; }
    [[nodiscard]] juce::String toString() const { return juce::String{ mValue }; }
    //==============================================================================
    StrongIndex & operator++()
    {
        ++mValue;
        return *this;
    }
    StrongIndex operator++(int) { return StrongIndex{ mValue++ }; }
    StrongIndex & operator--()
    {
        --mValue;
        return *this;
    }
    //==============================================================================
    template<typename TargetType>
    [[nodiscard]] TargetType removeOffset() const
    {
        return narrow<TargetType>(mValue - OFFSET);
    }
};

//==============================================================================
class StrongFloatBase
{
};

//==============================================================================
template<typename T, typename Derived, typename Dummy>
class StrongFloat : public StrongFloatBase
{
    static_assert(std::is_floating_point_v<T>, "Underlying types should be floating points.");
    static_assert(std::is_floating_point_v<T>);

protected:
    //==============================================================================
    T mValue;

public:
    //==============================================================================
    using type = T;
    //==============================================================================
    StrongFloat() = default;
    explicit constexpr StrongFloat(T const & value) : mValue(value) {}
    //==============================================================================
    [[nodiscard]] constexpr bool operator==(Derived const & other) const { return mValue == other.mValue; }
    [[nodiscard]] constexpr bool operator!=(Derived const & other) const { return mValue != other.mValue; }
    [[nodiscard]] constexpr bool operator<(Derived const & other) const { return mValue < other.mValue; }
    [[nodiscard]] constexpr bool operator>(Derived const & other) const { return mValue > other.mValue; }
    [[nodiscard]] constexpr bool operator<=(Derived const & other) const { return mValue <= other.mValue; }
    [[nodiscard]] constexpr bool operator>=(Derived const & other) const { return mValue >= other.mValue; }
    //==============================================================================
    [[nodiscard]] constexpr type const & get() const { return mValue; }
    [[nodiscard]] juce::String toString(int const precision = 2) const { return juce::String{ mValue, precision }; }
    //==============================================================================
    [[nodiscard]] constexpr Derived operator-() const { return Derived{ -mValue }; }
    [[nodiscard]] constexpr Derived operator+(Derived const & other) const { return Derived{ mValue + other.mValue }; }
    [[nodiscard]] constexpr Derived operator-(Derived const & other) const { return Derived{ mValue - other.mValue }; }
    [[nodiscard]] constexpr Derived operator*(type const mod) const { return Derived{ mValue * mod }; }
    [[nodiscard]] constexpr Derived operator/(type const mod) const { return Derived{ mValue / mod }; }
    [[nodiscard]] constexpr type operator/(Derived const & other) const { return mValue / other.mValue; }
    //==============================================================================
    Derived & operator+=(Derived const & other) noexcept
    {
        mValue += other.mValue;
        return *static_cast<Derived *>(this);
    }
    Derived & operator-=(Derived const & other) noexcept
    {
        mValue -= other.mValue;
        return *static_cast<Derived *>(this);
    }
    Derived & operator*=(type const & mod) noexcept
    {
        mValue *= mod;
        return *static_cast<Derived *>(this);
    }
    Derived & operator/=(type const & mod) noexcept
    {
        jassert(mod != 0);
        mValue /= mod;
        return *static_cast<Derived *>(this);
    }
    //==============================================================================
    [[nodiscard]] constexpr Derived abs() const noexcept { return Derived{ std::abs(mValue) }; }
    [[nodiscard]] constexpr Derived clamped(Derived const & min, Derived const & max) const noexcept
    {
        auto const & self{ *static_cast<Derived const *>(this) };
        return (self < min ? min : (self > max ? max : self));
    }

protected:
    //==============================================================================
    [[nodiscard]] constexpr Derived centeredAroundZero(type const amplitude) const noexcept
    {
        auto const halfAmplitude{ amplitude / static_cast<type>(2) };
        auto const isTooSmall = [min = -halfAmplitude](type const value) -> bool { return value < min; };

        if (isTooSmall(mValue)) {
            auto newValue{ mValue + amplitude };
            while (isTooSmall(newValue)) {
                newValue += amplitude;
            }
            return Derived{ newValue };
        }

        auto const isTooHigh = [max = halfAmplitude](type const value) -> bool { return value >= max; };

        if (isTooHigh(mValue)) {
            auto newValue{ mValue - amplitude };
            while (isTooHigh(newValue)) {
                newValue -= amplitude;
            }
            return Derived{ newValue };
        }

        return *static_cast<Derived const *>(this);
    }
};

//==============================================================================
class Degrees final : public StrongFloat<float, Degrees, struct DegreesT>
{
public:
    static constexpr type DEGREE_PER_RADIAN{ static_cast<type>(360) / juce::MathConstants<type>::twoPi };
    //==============================================================================
    Degrees() = default;
    explicit constexpr Degrees(type const & value) : StrongFloat(value) {}
    //==============================================================================
    [[nodiscard]] constexpr Degrees centered() const noexcept { return centeredAroundZero(static_cast<type>(360)); }
    [[nodiscard]] constexpr Degrees madePositive() const noexcept
    {
        return Degrees{ mValue < 0 ? mValue + static_cast<type>(360) : mValue };
    }
    [[nodiscard]] constexpr type getAsDegrees() const noexcept { return mValue; }
    [[nodiscard]] constexpr type getAsRadians() const noexcept { return mValue / DEGREE_PER_RADIAN; }
    //==============================================================================
    [[nodiscard]] static Degrees angleOf(juce::Point<type> const & point) noexcept
    {
        if (point.getX() == 0.0f && point.getY() == 0.0f) {
            return Degrees{};
        }
        return Degrees{ std::atan2(point.getY(), point.getX()) * DEGREE_PER_RADIAN };
    }
};

//==============================================================================
class Radians final : public StrongFloat<float, Radians, struct RadiansT>
{
public:
    static constexpr type RADIAN_PER_DEGREE{ juce::MathConstants<type>::twoPi / static_cast<type>(360) };
    //==============================================================================
    Radians() = default;
    explicit constexpr Radians(type const & value) : StrongFloat(value) {}
    constexpr Radians(Degrees const & degrees) : StrongFloat(degrees.get() * RADIAN_PER_DEGREE) {}
    //==============================================================================
    [[nodiscard]] constexpr Radians centered() const noexcept
    {
        return centeredAroundZero(juce::MathConstants<type>::twoPi);
    }
    //==============================================================================
    [[nodiscard]] constexpr Radians madePositive() const noexcept
    {
        return Radians{ mValue < 0 ? mValue + juce::MathConstants<type>::twoPi : mValue };
    }
    //==============================================================================
    [[nodiscard]] constexpr Degrees toDegrees() const noexcept
    {
        return Degrees{ mValue * Degrees::DEGREE_PER_RADIAN };
    }
    //==============================================================================
    [[nodiscard]] constexpr operator Degrees() const noexcept { return toDegrees(); }
    [[nodiscard]] constexpr type getAsDegrees() const noexcept { return mValue * Degrees::DEGREE_PER_RADIAN; }
    [[nodiscard]] constexpr type getAsRadians() const noexcept { return mValue; }
    //==============================================================================
    [[nodiscard]] static Radians angleOf(juce::Point<type> const & point) noexcept
    {
        if (point.getX() == 0.0f && point.getY() == 0.0f) {
            return Radians{};
        }
        return Radians{ std::atan2(point.getY(), point.getX()) };
    }
};

//==============================================================================
class Normalized final : public StrongFloat<float, Normalized, struct NormalizedT>
{
public:
    Normalized() = default;
    explicit constexpr Normalized(type const & value) : StrongFloat(value) {}
};

//==============================================================================
using SourceId = StrongIndex<int, struct SourceIdT, 1>;
using SourceIndex = StrongIndex<int, struct SourceIndexT, 0>;

constexpr Radians QUARTER_PI{ juce::MathConstants<Radians::type>::halfPi / 2.0f };
constexpr Radians HALF_PI{ juce::MathConstants<Radians::type>::halfPi };
constexpr Radians PI{ juce::MathConstants<Radians::type>::pi };
constexpr Radians TWO_PI{ juce::MathConstants<Radians::type>::twoPi };
} // namespace gris
