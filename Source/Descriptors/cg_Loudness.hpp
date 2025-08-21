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
#include <vector>

namespace gris
{
//==============================================================================
class LoudnessD : public Descriptor
{
public:
    //==============================================================================
    LoudnessD() { mID = DescriptorID::loudness; }

    void init(double mSampleRate)
    {
        mLoudness->init(mWindowSizeLoudness, mSampleRate);
        mLoudnessRunningStats->init(mRunningStatsHistory, 1);
    }

    void reset() override
    {
        mLoudness.reset(new fluid::algorithm::Loudness{ mWindowSizeLoudness });
        mLoudnessRunningStats.reset(new fluid::algorithm::RunningStats());
    }

    double getValue() override { return mDescLoudness; }

    void process(fluid::RealMatrix & loudnessMat, fluid::algorithm::MultiStats & stats)
    {
        fluid::RealVector loudnessStats = computeStats(loudnessMat, stats);

        fluid::RealVector loudnessMeanRes = fluid::RealVector(1);
        fluid::RealVector loudnessStdDevRes = fluid::RealVector(1);
        fluid::RealVectorView loudnessData = fluid::RealVectorView(loudnessStats(fluid::Slice(0, 1)));
        fluid::RealVectorView loudnessMeanOut = fluid::RealVectorView(loudnessMeanRes);
        fluid::RealVectorView loudnessStdDevOut = fluid::RealVectorView(loudnessStdDevRes);

        mLoudnessRunningStats->process(loudnessData, loudnessMeanOut, loudnessStdDevOut);

        mDescLoudness = loudnessMeanOut[0];
    }

    void loudnessProcess(fluid::RealVectorView & window, fluid::RealVector & loudnessDesc)
    {
        mLoudness->processFrame(window, loudnessDesc, true, true);
    }

    fluid::RealVectorView calculateWindow(fluid::RealVector & padded, int & i)
    {
        return padded(fluid::Slice(i * mHopSizeLoudness, mWindowSizeLoudness));
    }

    fluid::RealVector calculatePadded(fluid::RealVector in)
    {
        return in.size() + mWindowSizeLoudness + mHopSizeLoudness;
    }

    fluid::index calculateFrames(fluid::RealVector padded)
    {
        return static_cast<fluid::index>(floor((padded.size() - mWindowSizeLoudness) / mHopSizeLoudness));
    }

    fluid::Slice paddedValue(fluid::RealVector in) { return fluid::Slice(mHalfWindowLoudness, in.size()); }

private:
    //==============================================================================
    void init() override {}

    //==============================================================================
    std::unique_ptr<fluid::algorithm::RunningStats> mLoudnessRunningStats;
    double mDescLoudness{};
    std::unique_ptr<fluid::algorithm::Loudness> mLoudness;

    // LOUDNESS VALUE
    fluid::index mHopSizeLoudness = 256;
    fluid::index mWindowSizeLoudness = 512;
    fluid::index mHalfWindowLoudness = mWindowSizeLoudness / 2;

    //==============================================================================
    JUCE_LEAK_DETECTOR(LoudnessD)
};
} // namespace gris
