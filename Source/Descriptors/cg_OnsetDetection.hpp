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
        mOnsetIn.resize(0);
        mOnsetPadded.resize(0);
    }

    void init() override { mOnsetDetection->init(WINDOW_SIZE, FFT_SIZE, mOnsetDetectionFilterSize); }

    void reset() override
    {
        mOnsetIn.resize(NUM_SAMPLES_TO_PROCESS);
        mOnsetDetection.reset(new fluid::algorithm::OnsetDetectionFunctions(WINDOW_SIZE,
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
        mAllSamples.clear();
        mAllSamples.reserve(mOnsetDetectionUnusedSamples.size() + descriptorBuffer.getNumSamples());
        mOnsetDectectionVals.clear();
        auto * channelData = descriptorBuffer.getReadPointer(0);
        int nFramesDivider{};
        auto nSamplesODUnusedSamples{ mOnsetDetectionUnusedSamples.size() };
        auto nSamplesDescBuf{ descriptorBuffer.getNumSamples() };

        // get unprocessed samples from last processBlock call
        for (int i{}; i < nSamplesODUnusedSamples; ++i) {
            mAllSamples.push_back(static_cast<float>(mOnsetDetectionUnusedSamples[i]));
        }
        // get new samples
        for (int i{}; i < nSamplesDescBuf; ++i) {
            mAllSamples.push_back(channelData[i]);
        }
        for (int i{}; i < mAllSamples.size() / NUM_SAMPLES_TO_PROCESS; ++i) {
            std::fill(mOnsetIn.begin(), mOnsetIn.end(), 0); // necessary?
            for (int j{}; j < NUM_SAMPLES_TO_PROCESS; ++j) {
                mOnsetIn[j] = mAllSamples[j + (i * NUM_SAMPLES_TO_PROCESS)];
            }
            mOnsetPadded.resize(mOnsetIn.size() + WINDOW_SIZE + HOP_SIZE);
            fluid::index nOnsetFrames
                = static_cast<fluid::index>(floor((mOnsetPadded.size() - WINDOW_SIZE) / HOP_SIZE));
            nFramesDivider = static_cast<int>(nOnsetFrames);

            std::fill(mOnsetPadded.begin(), mOnsetPadded.end(), 0);
            mOnsetPadded(fluid::Slice(WINDOW_SIZE / 2, mOnsetIn.size())) <<= mOnsetIn;
            mOnsetDectectionVals.reserve(nOnsetFrames * mAllSamples.size() / NUM_SAMPLES_TO_PROCESS);
            for (int k = 0; k < nOnsetFrames; k++) {
                mWindowOD = mOnsetPadded(fluid::Slice(k * HOP_SIZE, WINDOW_SIZE));
                mOnsetDectectionVals.push_back(mOnsetDetection->processFrame(mWindowOD,
                                                                             mOnesetDetectionMetric,
                                                                             1,
                                                                             0 /*more than 0 gives assert*/,
                                                                             fluid::FluidDefaultAllocator()));
            }
        }

        // store unused samples
        if (mAllSamples.size() % NUM_SAMPLES_TO_PROCESS != 0) {
            mOnsetDetectionUnusedSamples.resize(mAllSamples.size() % NUM_SAMPLES_TO_PROCESS);
            for (int i = static_cast<int>(mAllSamples.size()) / NUM_SAMPLES_TO_PROCESS * NUM_SAMPLES_TO_PROCESS, j = 0;
                 i < mAllSamples.size();
                 ++i, ++j) {
                mOnsetDetectionUnusedSamples[j] = mAllSamples[i];
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
                mTimeSinceLastOnsetDetectionForProcessing = mTimeSinceLastOnsetDetectionDeque;
                std::sort(mTimeSinceLastOnsetDetectionForProcessing.begin(),
                          mTimeSinceLastOnsetDetectionForProcessing.end());
                auto median = mTimeSinceLastOnsetDetectionForProcessing.at(
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
            for (int i{}; i < mOnsetDectectionVals.size(); ++i) {
                if (mOnsetDectectionVals[i] >= mDescOnsetDetectionThreshold && mIsOnsetDetectionReady) {
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
                        mTimeSinceLastOnsetDetectionForProcessing = mTimeSinceLastOnsetDetectionDeque;
                        std::sort(mTimeSinceLastOnsetDetectionForProcessing.begin(),
                                  mTimeSinceLastOnsetDetectionForProcessing.end());
                        auto median = mTimeSinceLastOnsetDetectionForProcessing.at(
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
                } else if (mOnsetDectectionVals[i] < mDescOnsetDetectionThreshold) {
                    mIsOnsetDetectionReady = true;
                }
                if (mOnsetDetectionStartCountingSamples) {
                    mOnsetDetectionNumSamples += NUM_SAMPLES_TO_PROCESS;
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

    static constexpr fluid::index FFT_SIZE = 256;
    static constexpr fluid::index HOP_SIZE = 64;
    static constexpr fluid::index WINDOW_SIZE = 256;
    static constexpr int NUM_SAMPLES_TO_PROCESS{ 256 };

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
    std::deque<double> mTimeSinceLastOnsetDetectionForProcessing{};
    Direction mOnsetDetectionDirection{};
    int mSampleCounter{};
    bool mUseTimerButtonclickValue{};
    double mTimerButtonClickvalue{};
    std::vector<float> mAllSamples;
    std::vector<double> mOnsetDectectionVals{};
    fluid::RealVector mOnsetIn;
    fluid::RealVector mOnsetPadded;
    fluid::RealVectorView mWindowOD = mOnsetPadded(fluid::Slice(HOP_SIZE, WINDOW_SIZE));

    //==============================================================================
    JUCE_LEAK_DETECTOR(OnsetDetectionD)
};
} // namespace gris
