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
class VspanDome : public SpatialParameter
{
public:
    //==============================================================================
    VspanDome(juce::AudioProcessorValueTreeState & audioProcessorValueTreeState, SpatParamHelperFunctions & functions)
        : SpatialParameter(audioProcessorValueTreeState, functions)
    {
        parameterName = juce::String("Elevation Span");
        paramID = ParameterID::elevationspan;
        setParametersState();
    }

    void process(const DescriptorID & descID, double valueToProcess) override
    {
        auto range{ 0.0 };
        auto offset{ 0.0 };
        auto smooth{ 0.0 };

        switch (descID) {
        case DescriptorID::loudness:
            range = paramRangeLoudness;
            offset = paramOffsetLoudness;
            smooth = processLoudness(valueToProcess);
            break;
        case DescriptorID::pitch:
            range = paramRangePitch;
            smooth = processPitch(valueToProcess);
            break;
        case DescriptorID::centroid:
            range = paramRangeCentroid;
            smooth = processCentroid(valueToProcess);
            break;
        case DescriptorID::spread:
            range = paramRangeSpread;
            offset = paramOffsetSpread;
            smooth = processSpread(valueToProcess);
            break;
        case DescriptorID::noise:
            range = paramRangeNoise;
            offset = paramOffsetNoise;
            smooth = processNoise(valueToProcess);
            break;
        case DescriptorID::iterationsSpeed:
            range = paramRangeOD;
            smooth = processSmoothedOnsetDetection(valueToProcess);
            break;
        case DescriptorID::invalid:
        default:
            break;
        }

        double clipMax = 0.999999;
        int multiplier = 100;

        double clip = juce::jlimit(0.0, clipMax, smooth);
        double inputRange = range * 0.01;
        res = clip * inputRange * multiplier;
        res += offset * multiplier;

        if (std::isnan(res)) {
            res = 0.0;
        }
    }

private:
    //==============================================================================
    JUCE_LEAK_DETECTOR(VspanDome)
};
} // namespace gris
