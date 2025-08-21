/*
 This file is part of ControlGris.

 Developers: Hicheme BEN GAIED, Gaël LANE LÉPINE

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
class SpreadD : public Descriptor
{
public:
    //==============================================================================
    SpreadD() { mID = DescriptorID::spread; }

    void reset() override { mSpreadRunningStats.reset(new fluid::algorithm::RunningStats()); }

    void init() override { mSpreadRunningStats->init(mRunningStatsHistory, 1); }

    double getValue() override { return mDescSpread; }

    void process(fluid::RealVector & shapeStats)
    {
        fluid::RealVector spreadMeanRes = fluid::RealVector(1);
        fluid::RealVector spreadStdDevRes = fluid::RealVector(1);
        fluid::RealVectorView spreadData = fluid::RealVectorView(shapeStats(fluid::Slice(1 * 7, 1)));
        // we don't really need mean and stdDev...
        // fluid::RealVectorView spreadMeanOut = fluid::RealVectorView(spreadMeanRes);
        // fluid::RealVectorView spreadStdDevOut = fluid::RealVectorView(spreadStdDevRes);
        // mSpreadRunningStats->process(spreadData, spreadMeanOut, spreadStdDevOut);
        // mDescSpread = spreadMeanOut[0];
        mDescSpread = spreadData[0];
    }

private:
    //==============================================================================
    std::unique_ptr<fluid::algorithm::RunningStats> mSpreadRunningStats;
    double mDescSpread{};

    //==============================================================================
    JUCE_LEAK_DETECTOR(SpreadD)
};
} // namespace gris
