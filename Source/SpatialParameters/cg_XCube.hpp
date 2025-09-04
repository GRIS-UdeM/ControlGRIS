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
#include "cg_SpatialParameter.h"

namespace gris
{
//==============================================================================
class XCube : public SpatialParameter
{
public:
    //==============================================================================
    XCube(juce::AudioProcessorValueTreeState & audioProcessorValueTreeState, SpatParamHelperFunctions & functions)
        : SpatialParameter(audioProcessorValueTreeState, functions)
    {
        parameterName = juce::String("X");
        paramID = ParameterID::x;
        setParametersState();
    }

    void process(const DescriptorID & descID, double valueToProcess) override
    {
        auto range{ 0.0 };
        auto offset{ 0.0 };
        auto lap{ 1.0 };
        auto smooth{ 0.0 };

        switch (descID) {
        case DescriptorID::loudness:
            range = paramRangeLoudness;
            offset = paramOffsetLoudness;
            lap = paramLapLoudness;
            smooth = processLoudness(valueToProcess);
            break;
        case DescriptorID::pitch:
            range = paramRangePitch;
            lap = paramLapPitch;
            smooth = processPitch(valueToProcess);
            break;
        case DescriptorID::centroid:
            range = paramRangeCentroid;
            lap = paramLapCentroid;
            smooth = processCentroid(valueToProcess);
            break;
        case DescriptorID::spread:
            range = paramRangeSpread;
            offset = paramOffsetSpread;
            lap = paramLapSpread;
            smooth = processSpread(valueToProcess);
            break;
        case DescriptorID::noise:
            range = paramRangeNoise;
            offset = paramOffsetNoise;
            lap = paramLapNoise;
            smooth = processNoise(valueToProcess);
            break;
        case DescriptorID::iterationsSpeed:
            range = paramRangeOD;
            lap = paramLapOD;
            smooth = processSmoothedOnsetDetection(valueToProcess);
            break;
        case DescriptorID::invalid:
        default:
            break;
        }

        if (mActLikeAzimuth) {
            constexpr double clipMax = 1.0;
            constexpr int multiplier = 360;

            const double clip = juce::jlimit(0.0, clipMax, smooth);
            const double inputRange = range * 0.01;
            res = clip * inputRange * multiplier * lap;
            res += offset * multiplier;
        } else {
            constexpr double clipMax = 1.0;

            const double clipOne = juce::jlimit(0.0, clipMax, smooth);
            const double inputRange = range * 0.01;
            res = clipOne * inputRange;
            res *= 2.0; // to cover from -1 to 1 on gui
            res += offset;
        }

        if (std::isnan(res)) {
            jassertfalse;
            res = 0.0;
        }
    }

    void setActingLikeAzimuth(bool shoulActLikeAzimuth) { mActLikeAzimuth = shoulActLikeAzimuth; }

private:
    //==============================================================================
    bool mActLikeAzimuth{};

    //==============================================================================
    JUCE_LEAK_DETECTOR(XCube)
};
} // namespace gris
