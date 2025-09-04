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
    PitchD()
    {
        mID = DescriptorID::pitch;
        mPitchMeanRes = fluid::RealVector(1);
        mPitchStdDevRes = fluid::RealVector(1);
    }

    void init() override { mPitchRunningStats->init(mRunningStatsHistory, 1); }

    void reset() override
    {
        mYin.reset(new fluid::algorithm::YINFFT{ NBINS, fluid::FluidDefaultAllocator() });
        mPitchRunningStats.reset(new fluid::algorithm::RunningStats());
        mStft.reset(new fluid::algorithm::STFT{ WINDOW_SIZE, FFT_SIZE, HOP_SIZE });
    }

    double getValue() override { return mDescPitch; }

    void process(fluid::RealMatrix & pitchMat, fluid::algorithm::MultiStats & stats)
    {
        mPitchStats = computeStats(pitchMat, stats);

        fluid::RealVectorView pitchData = fluid::RealVectorView(mPitchStats(fluid::Slice(0, 1)));
        fluid::RealVectorView pitchMeanOut = fluid::RealVectorView(mPitchMeanRes);
        fluid::RealVectorView pitchStdDevOut = fluid::RealVectorView(mPitchStdDevRes);

        mPitchRunningStats->process(pitchData, pitchMeanOut, pitchStdDevOut);

        mDescPitch = pitchMeanOut[0];
    }

    void yinProcess(fluid::RealVector & magnitude, fluid::RealVector & pitch, double mSampleRate)
    {
        mYin->processFrame(magnitude, pitch, MIN_FREQ, MAX_FREQ, mSampleRate, fluid::FluidDefaultAllocator());
    }

    fluid::RealVectorView calculateWindow(fluid::RealVector & padded, int & i)
    {
        return padded(fluid::Slice(i * HOP_SIZE, WINDOW_SIZE));
    }

    fluid::RealVector calculatePadded(fluid::RealVector in) { return in.size() + WINDOW_SIZE + HOP_SIZE; }

    fluid::index calculateFrames(fluid::RealVector padded)
    {
        return static_cast<fluid::index>(floor((padded.size() - WINDOW_SIZE) / HOP_SIZE));
    }

    fluid::Slice paddedValue(fluid::RealVector in) { return fluid::Slice(HALF_WINDOW, in.size()); }

    void setFrame(fluid::ComplexVector & frame) { frame.resize(NBINS); }

    void setMagnitude(fluid::RealVector & magnitude) { magnitude.resize(NBINS); }

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

    static constexpr fluid::index NBINS = 16385;
    static constexpr fluid::index HOP_SIZE = 32768;
    static constexpr fluid::index WINDOW_SIZE = 32768;
    static constexpr fluid::index HALF_WINDOW = WINDOW_SIZE / 2;
    static constexpr fluid::index FFT_SIZE = 32768;
    static constexpr double MIN_FREQ = 20.0;
    static constexpr double MAX_FREQ = 10000.0;

    fluid::RealVector mPitchStats;
    fluid::RealVector mPitchMeanRes;
    fluid::RealVector mPitchStdDevRes;

    //==============================================================================
    JUCE_LEAK_DETECTOR(PitchD)
};
} // namespace gris
