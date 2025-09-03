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

#if JUCE_LINUX
    #include "../PCH.h"
#endif

#include <JuceHeader.h>

namespace gris
{
//==============================================================================
enum class DescriptorID { invalid = -1, loudness = 0, pitch, centroid, spread, noise, iterationsSpeed };

class Descriptor
{
public:
    //==============================================================================
    Descriptor() = default;
    virtual ~Descriptor() = default;

    virtual void init() = 0;
    virtual void reset() = 0;
    virtual double getValue() = 0;

    virtual DescriptorID getID() { return mID; }

    static int toInt(DescriptorID descID) { return static_cast<int>(descID) + 2; }

    static DescriptorID fromInt(int value)
    {
        switch (value) {
        case 2:
            return DescriptorID::loudness;
        case 3:
            return DescriptorID::pitch;
        case 4:
            return DescriptorID::centroid;
        case 5:
            return DescriptorID::spread;
        case 6:
            return DescriptorID::noise;
        case 7:
            return DescriptorID::iterationsSpeed;
        default:
            return DescriptorID::invalid;
        }
    }

    fluid::RealVector computeStats(fluid::RealMatrixView matrix, fluid::algorithm::MultiStats stats)
    {
        mDim = matrix.cols();
        fluid::RealMatrix resMat(mDim, 7);
        mResult.resize(mDim * 7);
        stats.process(matrix.transpose(), resMat);

        for (int j = 0; j < mDim; j++) {
            mResult(fluid::Slice(j * 7, 7)) <<= resMat.row(j);
        }

        return mResult;
    }

protected:
    //==============================================================================
    DescriptorID mID{ DescriptorID::invalid };
    int mRunningStatsHistory = 1;

private:
    //==============================================================================
    fluid::index mDim;
    fluid::RealVector mResult;

    //==============================================================================
    JUCE_LEAK_DETECTOR(Descriptor)
};
} // namespace gris
