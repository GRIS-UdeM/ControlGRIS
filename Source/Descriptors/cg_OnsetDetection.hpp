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

#include <JuceHeader.h>

#include "cg_Descriptors.hpp"

namespace gris
{
//==============================================================================
class OnsetDetectionD : public Descriptor
{
    enum class Direction { up, down };

public:
    //==============================================================================
    OnsetDetectionD()
    {
        mID = DescriptorID::iterationsSpeed;
        mOnsetDetectionUnusedSamples.resize(0);
    }

    void init() override { mOnsetDetection->init(mWindowSize, mFftSize, mOnsetDetectionFilterSize); }

    void reset() override
    {
        mWindowSize = 256;
        mFftSize = 256;
        mHopSize = 64;
        mOnsetDetection.reset(new fluid::algorithm::OnsetDetectionFunctions(mWindowSize,
                                                                            mOnesetDetectionMetric,
                                                                            fluid::FluidDefaultAllocator()));
    }

    double getValue() override { return mDescOnsetDetectionCurrent; }

    void setOnsetDetectionThreshold(const float treshold)
    {
        mDescOnsetDetectionThreshold = treshold;
        mDescOnsetDetectionCurrent = 0.0;
    }

    void setOnesetDetectionMetric(const int metric)
    {
        mOnesetDetectionMetric = metric;
        mDescOnsetDetectionCurrent = 0.0;
    }

    void setOnsetDetectionMinTime(const double minTime)
    {
        mOnsetDetectionTimeMin = minTime * 1000;
        mDescOnsetDetectionCurrent = 0.0;
    }

    void setOnsetDetectionMaxTime(const double maxTime)
    {
        mOnsetDetectionTimeMax = maxTime * 1000;
        mDescOnsetDetectionCurrent = 0.0;
    }

    void setOnsetDetectionFromClick(double timeValue)
    {
        mTimerButtonClickvalue = timeValue;
        mUseTimerButtonclickValue = true;
    }

    void process(juce::AudioBuffer<float> & descriptorBuffer, double sampleRate, int blockSize)
    {
        std::vector<float> allSamples;
        std::vector<double> onsetDectectionVals{};
        auto * channelData = descriptorBuffer.getReadPointer(0);
        int nFramesDivider{};
        int numSamplesToProcess{ 256 };

        // get unprocessed samples from last processBlock call
        for (int i{}; i < mOnsetDetectionUnusedSamples.size(); ++i) {
            allSamples.push_back(static_cast<float>(mOnsetDetectionUnusedSamples[i]));
        }
        // get new samples
        for (int i{}; i < descriptorBuffer.getNumSamples(); ++i) {
            allSamples.push_back(channelData[i]);
        }
        for (int i{}; i < allSamples.size() / numSamplesToProcess; ++i) {
            fluid::RealVector onsetIn(numSamplesToProcess);
            for (int j{}; j < numSamplesToProcess; ++j) {
                onsetIn[j] = allSamples[j + (i * numSamplesToProcess)];
            }
            fluid::RealVector onsetPadded(onsetIn.size() + mWindowSize + mHopSize);
            fluid::index nOnsetFrames = static_cast<fluid::index>(floor((onsetPadded.size() - mWindowSize) / mHopSize));
            nFramesDivider = static_cast<int>(nOnsetFrames);

            std::fill(onsetPadded.begin(), onsetPadded.end(), 0);
            onsetPadded(fluid::Slice(mWindowSize / 2, onsetIn.size())) <<= onsetIn;
            for (int k = 0; k < nOnsetFrames; k++) {
                fluid::RealVectorView window = onsetPadded(fluid::Slice(k * mHopSize, mWindowSize));
                onsetDectectionVals.push_back(mOnsetDetection->processFrame(window,
                                                                            mOnesetDetectionMetric,
                                                                            1,
                                                                            0 /*more than 0 gives assert*/,
                                                                            fluid::FluidDefaultAllocator()));
            }
        }

        // store unused samples
        if (allSamples.size() % numSamplesToProcess != 0) {
            mOnsetDetectionUnusedSamples.resize(allSamples.size() % numSamplesToProcess);
            for (int i = static_cast<int>(allSamples.size()) / numSamplesToProcess * numSamplesToProcess, j = 0;
                 i < allSamples.size();
                 ++i, ++j) {
                mOnsetDetectionUnusedSamples[j] = allSamples[i];
            }
        } else {
            mOnsetDetectionUnusedSamples.resize(0);
        }

        if (mUseTimerButtonclickValue) {
            // when user clicks for Iterations Speed
            mTimeSinceLastOnsetDetectionDeque.push_back(mTimerButtonClickvalue);
            mUseTimerButtonclickValue = false;
            mSampleCounter = 0;

            if (mTimeSinceLastOnsetDetectionDeque.size() > 3) {
                mTimeSinceLastOnsetDetectionDeque.pop_front();
            }

            if (mTimeSinceLastOnsetDetectionDeque.size() == 3) {
                auto timeSinceLastOnsetDetectionVec = mTimeSinceLastOnsetDetectionDeque;
                std::sort(timeSinceLastOnsetDetectionVec.begin(), timeSinceLastOnsetDetectionVec.end());
                auto median = timeSinceLastOnsetDetectionVec.at(
                    2); // Not the median. The longest time appears to give better results

                if (median >= mOnsetDetectionTimeMin && median <= mOnsetDetectionTimeMax) {
                    mDescOnsetDetectionTarget
                        = juce::jmap(median, mOnsetDetectionTimeMin, mOnsetDetectionTimeMax, 1.0, 0.0);
                    mDescOnsetDetectionTarget = std::clamp(mDescOnsetDetectionTarget, 0.0, 1.0);
                    mDescOnsetDetectionTarget = std::pow(mDescOnsetDetectionTarget, 4);
                    mTimeToOnsetDetectionTarget = median * 0.25;
                    mTimeToOnsetDetectionZero = median * 5;
                    mDifferenceOnsetDetection = mDescOnsetDetectionTarget - mDescOnsetDetectionCurrent;
                    mOnsetDetectionIncrement
                        = (mDifferenceOnsetDetection / (mTimeToOnsetDetectionTarget * (blockSize / sampleRate * 1000)))
                          * 200;
                    mDescOnsetDetectionTarget > mDescOnsetDetectionCurrent ? mOnsetDetectionDirection = Direction::up
                                                                           : mOnsetDetectionDirection = Direction::down;
                }
            }
        } else {
            // get onset detection from audio
            for (int i{}; i < onsetDectectionVals.size(); ++i) {
                if (onsetDectectionVals[i] >= mDescOnsetDetectionThreshold && mIsOnsetDetectionReady) {
                    mSampleCounter = 0;
                    mIsOnsetDetectionReady = false;
                    mOnsetDetectionStartCountingSamples = true;
                    mTimeSinceLastOnsetDetectionDeque.push_back(mOnsetDetectionNumSamples / sampleRate * 1000
                                                                / nFramesDivider);
                    mOnsetDetectionNumSamples = 0;

                    if (mTimeSinceLastOnsetDetectionDeque.size() > 3) {
                        mTimeSinceLastOnsetDetectionDeque.pop_front();
                    }

                    if (mTimeSinceLastOnsetDetectionDeque.size() == 3) {
                        auto timeSinceLastOnsetDetectionVec = mTimeSinceLastOnsetDetectionDeque;
                        std::sort(timeSinceLastOnsetDetectionVec.begin(), timeSinceLastOnsetDetectionVec.end());
                        auto median = timeSinceLastOnsetDetectionVec.at(
                            2); // Not the median. The longest time appears to give better results

                        if (median < mOnsetDetectionTimeMin || median > mOnsetDetectionTimeMax) {
                            continue;
                        }

                        mDescOnsetDetectionTarget
                            = juce::jmap(median, mOnsetDetectionTimeMin, mOnsetDetectionTimeMax, 1.0, 0.0);
                        mDescOnsetDetectionTarget = std::clamp(mDescOnsetDetectionTarget, 0.0, 1.0);
                        mDescOnsetDetectionTarget = std::pow(mDescOnsetDetectionTarget, 4);
                        mTimeToOnsetDetectionTarget = median * 0.25;
                        mTimeToOnsetDetectionZero = median * 5;
                        mDifferenceOnsetDetection = mDescOnsetDetectionTarget - mDescOnsetDetectionCurrent;
                        mOnsetDetectionIncrement = (mDifferenceOnsetDetection
                                                    / (mTimeToOnsetDetectionTarget * (blockSize / sampleRate * 1000)))
                                                   * 200; // 200 could be adjusted for smoothed value
                        mDescOnsetDetectionTarget > mDescOnsetDetectionCurrent
                            ? mOnsetDetectionDirection = Direction::up
                            : mOnsetDetectionDirection = Direction::down;
                    }
                } else if (onsetDectectionVals[i] < mDescOnsetDetectionThreshold) {
                    mIsOnsetDetectionReady = true;
                }
                if (mOnsetDetectionStartCountingSamples) {
                    mOnsetDetectionNumSamples += numSamplesToProcess;
                }
            }
        }

        mDescOnsetDetectionCurrent += mOnsetDetectionIncrement;
        mDescOnsetDetectionCurrent = std::clamp(mDescOnsetDetectionCurrent, 0.0, 1.0);
        if (mDescOnsetDetectionCurrent > 0) {
            if ((mOnsetDetectionDirection == Direction::up && mDescOnsetDetectionCurrent >= mDescOnsetDetectionTarget)
                || (mOnsetDetectionDirection == Direction::down
                    && mDescOnsetDetectionCurrent <= mDescOnsetDetectionTarget)) {
                mOnsetDetectionDirection = Direction::down;
                mDescOnsetDetectionTarget = 0.0;
                mDifferenceOnsetDetection = mDescOnsetDetectionTarget - mDescOnsetDetectionCurrent;
                mOnsetDetectionIncrement
                    = (mDifferenceOnsetDetection / (mTimeToOnsetDetectionZero * (blockSize / sampleRate * 1000)))
                      * 200; // 200 could be adjusted for smoothed value
            }
        } else {
            mSampleCounter += blockSize;

            if (mSampleCounter / sampleRate * 1000 >= mOnsetDetectionTimeMax) {
                mTimeSinceLastOnsetDetectionDeque.clear();
            }
        }
    }

private:
    //==============================================================================
    std::unique_ptr<fluid::algorithm::OnsetDetectionFunctions> mOnsetDetection;

    fluid::index mNBins = 513;
    fluid::index mFftSize = 2 * (mNBins - 1);
    fluid::index mHopSize = 1024;
    fluid::index mWindowSize = 1024;
    fluid::index mHalfWindow = mWindowSize / 2;
    fluid::index mNBands = 40;
    fluid::index mNCoefs = 13;

    fluid::index mOnsetDetectionFilterSize = 3;
    fluid::index mOnesetDetectionMetric = 9;
    double mOnsetDetectionTimeMin{ 100 };
    double mOnsetDetectionTimeMax{ 10000 };
    float mDescOnsetDetectionThreshold{ 0.1f };
    double mDescOnsetDetectionTarget{};
    double mDescOnsetDetectionCurrent{ 0.0 };
    double mTimeToOnsetDetectionTarget{};
    double mTimeToOnsetDetectionZero{};
    double mDifferenceOnsetDetection{};
    double mOnsetDetectionIncrement{};
    bool mOnsetDetectionStartCountingSamples{};
    bool mIsOnsetDetectionReady{ true };
    juce::uint64 mOnsetDetectionNumSamples{};
    fluid::RealVector mOnsetDetectionUnusedSamples;
    std::deque<double> mTimeSinceLastOnsetDetectionDeque{};
    Direction mOnsetDetectionDirection{};
    int mSampleCounter{};
    bool mUseTimerButtonclickValue{};
    double mTimerButtonClickvalue{};

    //==============================================================================
    JUCE_LEAK_DETECTOR(OnsetDetectionD)
};
} // namespace gris
