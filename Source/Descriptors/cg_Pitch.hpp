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
class PitchD : public Descriptor
{
public:
    //==============================================================================
    PitchD() { mID = DescriptorID::pitch; }

    void init() override { mPitchRunningStats->init(mRunningStatsHistory, 1); }

    void reset() override
    {
        mYin.reset(new fluid::algorithm::YINFFT{ mNBinsPitch, fluid::FluidDefaultAllocator() });
        mPitchRunningStats.reset(new fluid::algorithm::RunningStats());
        mStft.reset(new fluid::algorithm::STFT{ mWindowSizePitch, mFftSizePitch, mHopSizePitch });
    }

    double getValue() override { return mDescPitch; }

    void process(fluid::RealMatrix & pitchMat, fluid::algorithm::MultiStats & stats)
    {
        fluid::RealVector pitchStats = computeStats(pitchMat, stats);

        fluid::RealVector pitchMeanRes(1);
        fluid::RealVector pitchStdDevRes(1);

        fluid::RealVectorView pitchData = fluid::RealVectorView(pitchStats(fluid::Slice(0, 1)));
        fluid::RealVectorView pitchMeanOut = fluid::RealVectorView(pitchMeanRes);
        fluid::RealVectorView pitchStdDevOut = fluid::RealVectorView(pitchStdDevRes);

        mPitchRunningStats->process(pitchData, pitchMeanOut, pitchStdDevOut);

        mDescPitch = pitchMeanOut[0];
    }

    void yinProcess(fluid::RealVector & magnitude, fluid::RealVector & pitch, double mSampleRate)
    {
        mYin->processFrame(magnitude,
                           pitch,
                           static_cast<double>(mMinFreqPitch),
                           static_cast<double>(mMaxFreqPitch),
                           mSampleRate,
                           fluid::FluidDefaultAllocator());
    }

    fluid::RealVectorView calculateWindow(fluid::RealVector & padded, int & i)
    {
        return padded(fluid::Slice(i * mHopSizePitch, mWindowSizePitch));
    }

    fluid::RealVector calculatePadded(fluid::RealVector in) { return in.size() + mWindowSizePitch + mHopSizePitch; }

    fluid::index calculateFrames(fluid::RealVector padded)
    {
        return static_cast<fluid::index>(floor((padded.size() - mWindowSizePitch) / mHopSizePitch));
    }

    fluid::Slice paddedValue(fluid::RealVector in) { return fluid::Slice(mHalfWindowPitch, in.size()); }

    void setFrame(fluid::ComplexVector & frame) { frame.resize(mNBinsPitch); }

    void setMagnitude(fluid::RealVector & magnitude) { magnitude.resize(mNBinsPitch); }

    //==============================================================================
    // Stft stuff
    // Second argument is output
    void stftProcess(fluid::RealVectorView & window, fluid::ComplexVector & frame)
    {
        mStft->processFrame(window, frame);
    }

    // Second argument is output
    void stftMagnitude(fluid::ComplexVector & frame, fluid::RealVector & magnitude)
    {
        mStft->magnitude(frame, magnitude);
    }

private:
    //==============================================================================
    std::unique_ptr<fluid::algorithm::RunningStats> mPitchRunningStats;
    double mDescPitch{};
    std::unique_ptr<fluid::algorithm::YINFFT> mYin;
    std::unique_ptr<fluid::algorithm::STFT> mStft;

    // PITCH VALUE
    fluid::index mNBinsPitch = 16385;
    fluid::index mHopSizePitch = 32768;
    fluid::index mWindowSizePitch = 32768;
    fluid::index mHalfWindowPitch = mWindowSizePitch / 2;
    fluid::index mFftSizePitch = 32768;
    fluid::index mMinFreqPitch = 20;
    fluid::index mMaxFreqPitch = 10000;

    //==============================================================================
    JUCE_LEAK_DETECTOR(PitchD)
};
} // namespace gris
