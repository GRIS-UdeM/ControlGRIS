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

#include "../cg_constants.hpp"
#include <cmath>

namespace gris
{
//==============================================================================
class Smooth
{
public:
    //==============================================================================
    double doSmoothing(double targetValue, double smooth, double smoothCoef)
    {
        smooth = juce::jmap(smooth, 0.0, 100.0, 0.0, 200.0);
        smooth = std::max(1.0, std::min(smooth, 200.0));

        if (mStartHistory || mSmoothHistory != smooth || smooth <= 1) {
            mCurrentValue = targetValue;
            mStartHistory = false;
            mSmoothHistory = smooth;
        } else {
            smoothCoef *= 0.01;
            smoothCoef = 1 - smoothCoef;
            if (smoothCoef == 0) {
                smoothCoef = ALMOST_ZERO;
            }
            double logsmooth = std::log(smooth);
            double normalizedsmooth = 1.0 / logsmooth;
            double adjustment = (targetValue - mCurrentValue) * (normalizedsmooth * smoothCoef);

            mCurrentValue += adjustment;
        }

        if (std::isnan(mCurrentValue)) {
            // Just a safety. Processing pitch can output nan value if range of min and max frequencies is too small
            mCurrentValue = -1.0;
        }
        return mCurrentValue;
    }

private:
    //==============================================================================
    double mCurrentValue = -1;
    double mSmoothHistory = -1;
    bool mStartHistory = true;

    //==============================================================================
    JUCE_LEAK_DETECTOR(Smooth)
};
} // namespace gris
