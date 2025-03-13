/**************************************************************************
 * Copyright 2018 UdeM - GRIS - Olivier Belanger                          *
 *                                                                        *
 * This file is part of ControlGris, a multi-source spatialization plugin *
 *                                                                        *
 * ControlGris is free software: you can redistribute it and/or modify    *
 * it under the terms of the GNU Lesser General Public License as         *
 * published by the Free Software Foundation, either version 3 of the     *
 * License, or (at your option) any later version.                        *
 *                                                                        *
 * ControlGris is distributed in the hope that it will be useful,         *
 * but WITHOUT ANY WARRANTY; without even the implied warranty of         *
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the          *
 * GNU Lesser General Public License for more details.                    *
 *                                                                        *
 * You should have received a copy of the GNU Lesser General Public       *
 * License along with ControlGris.  If not, see                           *
 * <http://www.gnu.org/licenses/>.                                        *
 *************************************************************************/

#pragma once

#include "StructGRIS/cg_StrongTypes.hpp"

namespace gris
{
class Source;

//==============================================================================
struct SourceSnapshot {
    juce::Point<float> position;
    Radians z; // height in CUBE mode, elevation in DOME mode
    //==============================================================================
    SourceSnapshot() noexcept = default;
    ~SourceSnapshot() = default;

    SourceSnapshot(SourceSnapshot const &) = default;
    SourceSnapshot(SourceSnapshot &&) = default;

    SourceSnapshot & operator=(SourceSnapshot const &) = default;
    SourceSnapshot & operator=(SourceSnapshot &&) = default;
    //==============================================================================
    explicit SourceSnapshot(Source const & source) noexcept;

private:
    JUCE_LEAK_DETECTOR(SourceSnapshot)
}; // class SourceSnapshot

//==============================================================================
struct SourcesSnapshots {
    SourceSnapshot primary{};
    juce::Array<SourceSnapshot> secondaries{};
    //==============================================================================
    SourceSnapshot const & operator[](SourceIndex const index) const;
    //==============================================================================
    SourceSnapshot & operator[](SourceIndex const index);
    int size() const { return secondaries.size() + 1; }

    juce::String toString () const
    {
        juce::String ret;
        ret += primary.position.toString() + ", " + primary.z.toString() + "; ";
        for (auto const & snapshot : secondaries)
            ret += snapshot.position.toString() + ", " + snapshot.z.toString() + "; ";
       return ret;
    }

private:
    JUCE_LEAK_DETECTOR(SourceSnapshot)
}; // class SourcesSnapshots

} // namespace gris
