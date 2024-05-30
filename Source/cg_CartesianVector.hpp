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

#include <cmath>
namespace gris
{
//==============================================================================
struct CartesianVector {
    float x;
    float y;
    float z;
    //==============================================================================
    [[nodiscard]] constexpr bool operator==(CartesianVector const & other) const noexcept
    {
        return x == other.x && y == other.y && z == other.z;
    }
    //==============================================================================
    [[nodiscard]] constexpr CartesianVector operator-(CartesianVector const & other) const noexcept
    {
        CartesianVector const result{ x - other.x, y - other.y, z - other.z };
        return result;
    }
    //==============================================================================
    [[nodiscard]] constexpr CartesianVector operator/(float const scalar) const noexcept
    {
        CartesianVector const result{ x / scalar, y / scalar, z / scalar };
        return result;
    }
    //==============================================================================
    /* Returns the vector length without the sqrt. */
    [[nodiscard]] constexpr float length2() const noexcept { return x * x + y * y + z * z; }
    //==============================================================================
    [[nodiscard]] float length() const noexcept { return std::sqrt(length2()); }
    //==============================================================================
    [[nodiscard]] CartesianVector crossProduct(CartesianVector const & other) const noexcept;
    //==============================================================================
    [[nodiscard]] constexpr CartesianVector operator-() const noexcept { return CartesianVector{ -x, -y, -z }; }
    //==============================================================================
    [[nodiscard]] constexpr float dotProduct(CartesianVector const & other) const noexcept
    {
        return x * other.x + y * other.y + z * other.z;
    }
    //==============================================================================
    [[nodiscard]] constexpr CartesianVector mean(CartesianVector const & other) const noexcept
    {
        auto const newX{ (x + other.x) * 0.5f };
        auto const newY{ (y + other.y) * 0.5f };
        auto const newZ{ (z + other.z) * 0.5f };

        CartesianVector const result{ newX, newY, newZ };
        return result;
    }
    //==============================================================================
    [[nodiscard]] float angleWith(CartesianVector const & other) const noexcept;

    [[nodiscard]] juce::Point<float> discardZ() const noexcept { return juce::Point<float>{ x, y }; }
};
} // namespace gris
