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

#include "cg_SpatialParameter.h"
// #include "../PanelView.h"

namespace gris
{
//==============================================================================
SpatialParameter::SpatialParameter(juce::AudioProcessorValueTreeState & audioProcessorValueTreeState,
                                   SpatParamHelperFunctions & functions)
    : mAPVTS(audioProcessorValueTreeState)
    , mFunctions(functions)
    , mDescriptorToUse(DescriptorID::invalid)
{
}

//==============================================================================
juce::String const & SpatialParameter::getParameterName() const
{
    return parameterName;
}

////==============================================================================
// void SpatialParameter::addObserver(PanelView * observer)
//{
//     mObservers.push_back(observer);
// }
//
////==============================================================================
// void SpatialParameter::removeObserver(PanelView * observer)
//{
//     auto it = std::find(mObservers.begin(), mObservers.end(), observer);
//     if (it != mObservers.end()) {
//         mObservers.erase(it);
//     }
// }
//
////==============================================================================
// void SpatialParameter::notifyObservers()
//{
//     for (auto observer : mObservers) {
//         observer->addNewParamValueToDataGraph(lastRes);
//     }
// }

//==============================================================================
double SpatialParameter::getDiffValue()
{
    auto diff = lastRes - res;
    lastRes = res;
    // notifyObservers();
    return diff;
}

//==============================================================================
double SpatialParameter::getValue()
{
    return lastRes;
}

//==============================================================================
void SpatialParameter::setDescriptorToUse(DescriptorID descID)
{
    mDescriptorToUse = descID;
}

//==============================================================================
DescriptorID SpatialParameter::getDescriptorToUse()
{
    return mDescriptorToUse;
}

//==============================================================================
double SpatialParameter::processLoudness(double valueToProcess)
{
    valueToProcess = valueToProcess * (paramFactorLoudness * 0.02);
    valueToProcess = processSmoothedLoudness(valueToProcess);
    return valueToProcess;
}

//==============================================================================
double SpatialParameter::processPitch(double valueToProcess)
{
    auto val{ 0.0 };
    double minFreq = mFunctions.frequencyToMidiNoteNumber(paramMinFreqPitch);
    double maxFreq = mFunctions.frequencyToMidiNoteNumber(paramMaxFreqPitch);
    double zmap = mFunctions.zmap(valueToProcess,
                                  minFreq,
                                  maxFreq); // zmap can be nan if frequency range for analysis is not wide enough...
    val = processSmoothedPitch(zmap);
    return val;
}

//==============================================================================
double SpatialParameter::processCentroid(double valueToProcess)
{
    auto val{ 0.0 };
    double minFreq = mFunctions.frequencyToMidiNoteNumber(paramMinFreqCentroid);
    double maxFreq = mFunctions.frequencyToMidiNoteNumber(paramMaxFreqCentroid);
    double zmap = mFunctions.zmap(valueToProcess, minFreq, maxFreq);
    val = processSmoothedCentroid(zmap);
    return val;
}

//==============================================================================
double SpatialParameter::processSpread(double valueToProcess)
{
    auto val{ 0.0 };
    double scaleOne = paramFactorSpread;
    scaleOne = mFunctions.zmap(scaleOne, 100.0, 500.0);
    scaleOne = 1.0 - scaleOne;
    double power = pow(valueToProcess, scaleOne);
    double scaleExpr = mFunctions.scaleExpr(power);
    double scaleTwo = paramFactorSpread;
    scaleTwo = mFunctions.clip(scaleTwo);
    double valueToSmooth = scaleExpr * scaleTwo;
    val = processSmoothedSpread(valueToSmooth);
    return val;
}

//==============================================================================
double SpatialParameter::processNoise(double valueToProcess)
{
    valueToProcess = valueToProcess * (paramFactorNoise * 0.01);
    valueToProcess = processSmoothedNoise(valueToProcess);
    return valueToProcess;
}

//==============================================================================
double SpatialParameter::processSmoothedLoudness(double targetValue)
{
    return mSmoothLoudness.doSmoothing(targetValue, paramSmoothLoudness, paramSmoothCoefLoudness);
}

//==============================================================================
double SpatialParameter::processSmoothedPitch(double targetValue)
{
    return mSmoothPitch.doSmoothing(targetValue, paramSmoothPitch, paramSmoothCoefPitch);
}

//==============================================================================
double SpatialParameter::processSmoothedCentroid(double targetValue)
{
    return mSmoothCentroid.doSmoothing(targetValue, paramSmoothCentroid, paramSmoothCoefCentroid);
}

//==============================================================================
double SpatialParameter::processSmoothedSpread(double targetValue)
{
    return mSmoothSpread.doSmoothing(targetValue, paramSmoothSpread, paramSmoothCoefSpread);
}

//==============================================================================
double SpatialParameter::processSmoothedNoise(double targetValue)
{
    return mSmoothNoise.doSmoothing(targetValue, paramSmoothNoise, paramSmoothCoefNoise);
}

//==============================================================================
double SpatialParameter::processSmoothedOnsetDetection(double targetValue)
{
    return mSmoothOnsetDetection.doSmoothing(targetValue, paramSmoothOD, paramSmoothCoefOD);
}

//====================================================================
bool SpatialParameter::needsSpectralAnalysis()
{
    if (mDescriptorToUse == DescriptorID::centroid || mDescriptorToUse == DescriptorID::spread
        || mDescriptorToUse == DescriptorID::noise) {
        if ((paramRangeCentroid != 0 && paramMaxFreqCentroid > paramMinFreqCentroid)
            || (paramRangeSpread != 0 && paramFactorSpread > 0) || (paramRangeNoise != 0 && paramFactorNoise > 0)) {
            return true;
        }
    }
    return false;
}

//==============================================================================
bool SpatialParameter::shouldProcessLoudnessAnalysis()
{
    if (mDescriptorToUse == DescriptorID::loudness && paramFactorLoudness > 0 && paramRangeLoudness != 0) {
        return true;
    }
    return false;
}

//==============================================================================
bool SpatialParameter::shouldProcessPitchAnalysis()
{
    if (mDescriptorToUse == DescriptorID::pitch && paramMinFreqPitch < paramMaxFreqPitch && paramRangePitch != 0) {
        return true;
    }
    return false;
}

//==============================================================================
bool SpatialParameter::shouldProcessCentroidAnalysis()
{
    if (mDescriptorToUse == DescriptorID::centroid && paramMinFreqCentroid < paramMaxFreqCentroid
        && paramRangeCentroid != 0) {
        return true;
    }
    return false;
}

//==============================================================================
bool SpatialParameter::shouldProcessSpreadAnalysis()
{
    if (mDescriptorToUse == DescriptorID::spread && paramFactorSpread > 0 && paramRangeSpread != 0) {
        return true;
    }
    return false;
}

//==============================================================================
bool SpatialParameter::shouldProcessNoiseAnalysis()
{
    if (mDescriptorToUse == DescriptorID::noise && paramFactorNoise > 0 && paramRangeNoise != 0) {
        return true;
    }
    return false;
}

//==============================================================================
bool SpatialParameter::shouldProcessOnsetDetectionAnalysis()
{
    if (mDescriptorToUse == DescriptorID::iterationsSpeed && paramMinTime < paramMaxTime) {
        return true;
    }
    return false;
}
} // namespace gris
