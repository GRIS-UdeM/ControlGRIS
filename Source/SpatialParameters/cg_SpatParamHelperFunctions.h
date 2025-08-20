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
#include <cmath>

namespace gris
{
//==============================================================================
class SpatParamHelperFunctions
{
public:
    //==============================================================================
    double frequencyToMidiNoteNumber(double frequency)
    {
        if (frequency <= 0.0) {
            return -1;
        }
        double midiNoteNumber = 69.0 + 12.0 * std::log2(frequency / 440.0);

        return midiNoteNumber;
    }

    double zmap(double value, double inputMin, double inputMax)
    {
        // clip value
        if (value < inputMin) {
            value = inputMin;
        } else if (value > inputMax) {
            value = inputMax;
        }

        // Formula : (val - minIn) / (maxIn - minIn) * (maxOut - minOut) + minOut
        return (value - inputMin) / (inputMax - inputMin) * (1.0 - 0.0) + 0.0;
    }

    double scaleExpr(double val)
    {
        if (val < 0.5) {
            return 4.0 * pow(val, 3);
        } else {
            return 0.5 * pow((2.0 * val - 2.0), 3) + 1.0;
        }
    }

    double clip(double value)
    {
        double clipedValue = juce::jlimit(0.0, 100.0, value);
        return clipedValue * 0.01;
    }

    double power(double input) { return pow(input, 0.4); }

private:
    //==============================================================================
    JUCE_LEAK_DETECTOR(SpatParamHelperFunctions)
};
} // namespace gris
