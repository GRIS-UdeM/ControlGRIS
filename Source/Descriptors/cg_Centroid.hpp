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
class CentroidD : public Descriptor
{
public:
    //==============================================================================
    CentroidD() { mID = DescriptorID::centroid; }

    void reset() override { mCentroidRunningStats.reset(new fluid::algorithm::RunningStats()); }

    void init() override { mCentroidRunningStats->init(mRunningStatsHistory, 1); }

    double getValue() override { return mDescCentroid; }

    void process(fluid::RealVector & shapeStats)
    {
        fluid::RealVector centroidMeanRes = fluid::RealVector(1);
        fluid::RealVector centroidStdDevRes = fluid::RealVector(1);
        fluid::RealVectorView centroidData = fluid::RealVectorView(shapeStats(fluid::Slice(0, 1)));
        // we don't really need mean and stdDev...
        // fluid::RealVectorView centroidMeanOut = fluid::RealVectorView(centroidMeanRes);
        // fluid::RealVectorView centroidStdDevOut = fluid::RealVectorView(centroidStdDevRes);
        // mCentroidRunningStats->process(centroidData, centroidMeanOut, centroidStdDevOut);
        // mDescCentroid = centroidMeanOut[0];
        mDescCentroid = centroidData[0];
    }

private:
    //==============================================================================
    std::unique_ptr<fluid::algorithm::RunningStats> mCentroidRunningStats;
    double mDescCentroid{};

    //==============================================================================
    JUCE_LEAK_DETECTOR(CentroidD)
};
} // namespace gris
