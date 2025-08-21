/*
 This file is part of ControlGris.

 Developers: Hicheme BEN GAIED, Ga'l LANE LƒPINE

 ControlGris is free software: you can redistribute it and/or modify
 it under the terms of the GNU Lesser General Public License as
 published by the Free Software Foundation, either version 3 of the
 License, or (at your option) any later version.

 ControlGris is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU Lesser General Public License for more details.

 You should have received a copy of the GNU Lesser General Public
 License along with ControlGris.  If not, see
 <http://www.gnu.org/licenses/>.
*/

//==============================================================================

#pragma once

#include "cg_Descriptors.hpp"

namespace gris
{
//==============================================================================
class StatsD : public Descriptor
{
public:
    //==============================================================================
    void reset() override { mStats.reset(new fluid::algorithm::MultiStats()); }

    void init() override { mStats->init(0, 0, 50, 100); }

    fluid::algorithm::MultiStats * getStats() const { return mStats.get(); }

private:
    //==============================================================================
    double getValue() override { return 0; }

    //==============================================================================
    std::unique_ptr<fluid::algorithm::MultiStats> mStats;

    //==============================================================================
    JUCE_LEAK_DETECTOR(StatsD)
};
} // namespace gris
