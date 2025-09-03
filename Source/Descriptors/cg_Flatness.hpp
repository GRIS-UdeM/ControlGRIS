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
class FlatnessD : public Descriptor
{
public:
    //==============================================================================
    FlatnessD()
    {
        mID = DescriptorID::noise;
        mFlatnessMeanRes = fluid::RealVector(1);
        mFlatnessStdDevRes = fluid::RealVector(1);
    }

    void reset() override { mFlatnessRunningStats.reset(new fluid::algorithm::RunningStats()); }

    void init() override { mFlatnessRunningStats->init(mRunningStatsHistory, 1); }

    double getValue() override { return mDescFlatness; }

    void process(fluid::RealVector & shapeStats)
    {
        fluid::RealVector flatnessMeanRes = fluid::RealVector(1);
        fluid::RealVector flatnessStdDevRes = fluid::RealVector(1);
        fluid::RealVectorView flatnessData = fluid::RealVectorView(shapeStats(fluid::Slice(5 * 7, 1)));
        // we don't really need mean and stdDev...
        // fluid::RealVectorView flatnessMeanOut = fluid::RealVectorView(flatnessMeanRes);
        // fluid::RealVectorView flatnessStdDevOut = fluid::RealVectorView(flatnessStdDevRes);
        // mFlatnessRunningStats->process(flatnessData, flatnessMeanOut, flatnessStdDevOut);
        // mDescFlatness = flatnessMeanOut[0];
        mDescFlatness = flatnessData[0];
    }

private:
    //==============================================================================
    std::unique_ptr<fluid::algorithm::RunningStats> mFlatnessRunningStats;
    double mDescFlatness{};

    fluid::RealVector mFlatnessMeanRes;
    fluid::RealVector mFlatnessStdDevRes;

    //==============================================================================
    JUCE_LEAK_DETECTOR(FlatnessD)
};
} // namespace gris
