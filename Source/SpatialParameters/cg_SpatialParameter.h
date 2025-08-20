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

#if JUCE_LINUX
    #include "../PCH.h"
#endif

#include <JuceHeader.h>
#include <cmath>

#include "../Descriptors/cg_Descriptors.hpp"
#include "../cg_constants.hpp"
#include "cg_Smooth.hpp"
#include "cg_SpatParamHelperFunctions.h"

namespace gris
{
//==============================================================================
// class PanelView;
enum class ParameterID { invalid = -1, azimuth = 0, elevation, x, y, z, azimuthspan, elevationspan };

class SpatialParameter
{
public:
    //==============================================================================
    SpatialParameter() = delete;
    explicit SpatialParameter(juce::AudioProcessorValueTreeState & audioProcessorValueTreeState,
                              SpatParamHelperFunctions & functions);
    virtual ~SpatialParameter() = default;

    virtual void process(const DescriptorID & descID, double valueToProcess) = 0;

    virtual juce::String const & getParameterName() const;

    // virtual void addObserver(PanelView * observer);
    // virtual void removeObserver(PanelView * observer);
    // virtual void notifyObservers();

    double getDiffValue();
    double getValue();

    void setDescriptorToUse(DescriptorID descID);
    DescriptorID getDescriptorToUse();

    double processLoudness(double valueToProcess);
    double processPitch(double valueToProcess);
    double processCentroid(double valueToProcess);
    double processSpread(double valueToProcess);
    double processNoise(double valueToProcess);

    double processSmoothedLoudness(double targetValue);
    double processSmoothedPitch(double targetValue);
    double processSmoothedCentroid(double targetValue);
    double processSmoothedSpread(double targetValue);
    double processSmoothedNoise(double targetValue);
    double processSmoothedOnsetDetection(double targetValue);

    //====================================================================
    // Analysis
    bool needsSpectralAnalysis();
    bool shouldProcessLoudnessAnalysis();
    bool shouldProcessPitchAnalysis();
    bool shouldProcessCentroidAnalysis();
    bool shouldProcessSpreadAnalysis();
    bool shouldProcessNoiseAnalysis();
    bool shouldProcessOnsetDetectionAnalysis();

    //====================================================================
    ParameterID getParameterID() const { return paramID; }

    int getParamDescriptorComboBoxIndex() const { return paramDescriptorComboBoxIndex; }

    void setParamDescriptorComboBoxIndex(int index)
    {
        paramDescriptorComboBoxIndex = index;
        mAPVTS.state.setProperty({ juce::String(parameterName.removeCharacters(" ") + "_DescriptorIndex") },
                                 paramDescriptorComboBoxIndex,
                                 nullptr);
    }

    //====================================================================
    double getParamExpanderLoudness() const { return paramExpanderLoudness; }

    double getParamExpanderSpread() const { return paramExpanderSpread; }

    double getParamExpanderNoise() const { return paramExpanderNoise; }

    double getParamSmoothLoudness() const { return paramSmoothLoudness; }

    double getParamSmoothPitch() const { return paramSmoothPitch; }

    double getParamSmoothCentroid() const { return paramSmoothCentroid; }

    double getParamSmoothSpread() const { return paramSmoothSpread; }

    double getParamSmoothNoise() const { return paramSmoothNoise; }

    double getParamSmoothOnsetDetection() const { return paramSmoothOD; }

    double getParamSmoothCoefLoudness() const { return paramSmoothCoefLoudness; }

    double getParamSmoothCoefPitch() const { return paramSmoothCoefPitch; }

    double getParamSmoothCoefCentroid() const { return paramSmoothCoefCentroid; }

    double getParamSmoothCoefSpread() const { return paramSmoothCoefSpread; }

    double getParamSmoothCoefNoise() const { return paramSmoothCoefNoise; }

    double getParamSmoothCoefOnsetDetection() const { return paramSmoothCoefOD; }

    double getParamRangeLoudness() const { return paramRangeLoudness; }

    double getParamRangePitch() const { return paramRangePitch; }

    double getParamRangeCentroid() const { return paramRangeCentroid; }

    double getParamRangeSpread() const { return paramRangeSpread; }

    double getParamRangeNoise() const { return paramRangeNoise; }

    double getParamRangeOnsetDetection() const { return paramRangeOD; }

    double getParamLapLoudness() const { return paramLapLoudness; }

    double getParamLapPitch() const { return paramLapPitch; }

    double getParamLapCentroid() const { return paramLapCentroid; }

    double getParamLapSpread() const { return paramLapSpread; }

    double getParamLapNoise() const { return paramLapNoise; }

    double getParamLapOnsetDetection() const { return paramLapOD; }

    double getParamOffsetLoudness() const { return paramOffsetLoudness; }

    double getParamOffsetPitch() const { return paramOffsetPitch; }

    double getParamOffsetCentroid() const { return paramOffsetCentroid; }

    double getParamOffsetSpread() const { return paramOffsetSpread; }

    double getParamOffsetNoise() const { return paramOffsetNoise; }

    double getParamOffsetOnsetDetection() const { return paramOffsetOD; }

    double getParamMinFreqPitch() const { return paramMinFreqPitch; }

    double getParamMinFreqCentroid() const { return paramMinFreqCentroid; }

    double getParamMaxFreqPitch() const { return paramMaxFreqPitch; }

    double getParamMaxFreqCentroid() const { return paramMaxFreqCentroid; }

    int getParamMetricComboBoxIndex() const { return paramMetricComboBoxIndex; }

    double getParamThreshold() const { return paramThreshold; }

    double getParamMinTime() const { return paramMinTime; }

    double getParamMaxTime() const { return paramMaxTime; }

    void setParamExpanderLoudness(double value)
    {
        paramExpanderLoudness = value;
        mAPVTS.state.setProperty({ juce::String(parameterName.removeCharacters(" ") + "_ExpanderLoudness") },
                                 paramExpanderLoudness,
                                 nullptr);
    }

    void setParamExpanderSpread(double value)
    {
        paramExpanderSpread = value;
        mAPVTS.state.setProperty({ juce::String(parameterName.removeCharacters(" ") + "_ExpanderSpread") },
                                 paramExpanderSpread,
                                 nullptr);
    }

    void setParamExpanderNoise(double value)
    {
        paramExpanderNoise = value;
        mAPVTS.state.setProperty({ juce::String(parameterName.removeCharacters(" ") + "_ExpanderNoise") },
                                 paramExpanderNoise,
                                 nullptr);
    }

    void setParamSmoothLoudness(double value)
    {
        paramSmoothLoudness = value;
        mAPVTS.state.setProperty({ juce::String(parameterName.removeCharacters(" ") + "_SmoothLoudness") },
                                 paramSmoothLoudness,
                                 nullptr);
    }

    void setParamSmoothPitch(double value)
    {
        paramSmoothPitch = value;
        mAPVTS.state.setProperty({ juce::String(parameterName.removeCharacters(" ") + "_SmoothPitch") },
                                 paramSmoothPitch,
                                 nullptr);
    }

    void setParamSmoothCentroid(double value)
    {
        paramSmoothCentroid = value;
        mAPVTS.state.setProperty({ juce::String(parameterName.removeCharacters(" ") + "_SmoothCentroid") },
                                 paramSmoothCentroid,
                                 nullptr);
    }

    void setParamSmoothSpread(double value)
    {
        paramSmoothSpread = value;
        mAPVTS.state.setProperty({ juce::String(parameterName.removeCharacters(" ") + "_SmoothSpread") },
                                 paramSmoothSpread,
                                 nullptr);
    }

    void setParamSmoothNoise(double value)
    {
        paramSmoothNoise = value;
        mAPVTS.state.setProperty({ juce::String(parameterName.removeCharacters(" ") + "_SmoothNoise") },
                                 paramSmoothNoise,
                                 nullptr);
    }

    void setParamSmoothOnsetDetection(double value)
    {
        paramSmoothOD = value;
        mAPVTS.state.setProperty({ juce::String(parameterName.removeCharacters(" ") + "_SmoothOnsetDetection") },
                                 paramSmoothOD,
                                 nullptr);
    }

    void setParamSmoothCoefLoudness(double value)
    {
        paramSmoothCoefLoudness = value;
        mAPVTS.state.setProperty({ juce::String(parameterName.removeCharacters(" ") + "_SmoothCoefLoudness") },
                                 paramSmoothCoefLoudness,
                                 nullptr);
    }

    void setParamSmoothCoefPitch(double value)
    {
        paramSmoothCoefPitch = value;
        mAPVTS.state.setProperty({ juce::String(parameterName.removeCharacters(" ") + "_SmoothCoefPitch") },
                                 paramSmoothCoefPitch,
                                 nullptr);
    }

    void setParamSmoothCoefCentroid(double value)
    {
        paramSmoothCoefCentroid = value;
        mAPVTS.state.setProperty({ juce::String(parameterName.removeCharacters(" ") + "_SmoothCoefCentroid") },
                                 paramSmoothCoefCentroid,
                                 nullptr);
    }

    void setParamSmoothCoefSpread(double value)
    {
        paramSmoothCoefSpread = value;
        mAPVTS.state.setProperty({ juce::String(parameterName.removeCharacters(" ") + "_SmoothCoefSpread") },
                                 paramSmoothCoefSpread,
                                 nullptr);
    }

    void setParamSmoothCoefNoise(double value)
    {
        paramSmoothCoefNoise = value;
        mAPVTS.state.setProperty({ juce::String(parameterName.removeCharacters(" ") + "_SmoothCoefNoise") },
                                 paramSmoothCoefNoise,
                                 nullptr);
    }

    void setParamSmoothCoefOnsetDetection(double value)
    {
        paramSmoothCoefOD = value;
        mAPVTS.state.setProperty({ juce::String(parameterName.removeCharacters(" ") + "_SmoothCoefOnsetDetection") },
                                 paramSmoothCoefOD,
                                 nullptr);
    }

    void setParamRangeLoudness(double value)
    {
        paramRangeLoudness = value;
        mAPVTS.state.setProperty({ juce::String(parameterName.removeCharacters(" ") + "_RangeLoudness") },
                                 paramRangeLoudness,
                                 nullptr);
    }

    void setParamRangePitch(double value)
    {
        paramRangePitch = value;
        mAPVTS.state.setProperty({ juce::String(parameterName.removeCharacters(" ") + "_RangePitch") },
                                 paramRangePitch,
                                 nullptr);
    }

    void setParamRangeCentroid(double value)
    {
        paramRangeCentroid = value;
        mAPVTS.state.setProperty({ juce::String(parameterName.removeCharacters(" ") + "_RangeCentroid") },
                                 paramRangeCentroid,
                                 nullptr);
    }

    void setParamRangeSpread(double value)
    {
        paramRangeSpread = value;
        mAPVTS.state.setProperty({ juce::String(parameterName.removeCharacters(" ") + "_RangeSpread") },
                                 paramRangeSpread,
                                 nullptr);
    }

    void setParamRangeNoise(double value)
    {
        paramRangeNoise = value;
        mAPVTS.state.setProperty({ juce::String(parameterName.removeCharacters(" ") + "_RangeNoise") },
                                 paramRangeNoise,
                                 nullptr);
    }

    void setParamRangeOnsetDetection(double value)
    {
        paramRangeOD = value;
        mAPVTS.state.setProperty({ juce::String(parameterName.removeCharacters(" ") + "_RangeOnsetDetection") },
                                 paramRangeOD,
                                 nullptr);
    }

    void setParamLapLoudness(double value)
    {
        paramLapLoudness = value;
        mAPVTS.state.setProperty({ juce::String(parameterName.removeCharacters(" ") + "_LapLoudness") },
                                 paramLapLoudness,
                                 nullptr);
    }

    void setParamLapPitch(double value)
    {
        paramLapPitch = value;
        mAPVTS.state.setProperty({ juce::String(parameterName.removeCharacters(" ") + "_LapPitch") },
                                 paramLapPitch,
                                 nullptr);
    }

    void setParamLapCentroid(double value)
    {
        paramLapCentroid = value;
        mAPVTS.state.setProperty({ juce::String(parameterName.removeCharacters(" ") + "_LapCentroid") },
                                 paramLapCentroid,
                                 nullptr);
    }

    void setParamLapSpread(double value)
    {
        paramLapSpread = value;
        mAPVTS.state.setProperty({ juce::String(parameterName.removeCharacters(" ") + "_LapSpread") },
                                 paramLapSpread,
                                 nullptr);
    }

    void setParamLapNoise(double value)
    {
        paramLapNoise = value;
        mAPVTS.state.setProperty({ juce::String(parameterName.removeCharacters(" ") + "_LapNoise") },
                                 paramLapNoise,
                                 nullptr);
    }

    void setParamLapOnsetDetection(double value)
    {
        paramLapOD = value;
        mAPVTS.state.setProperty({ juce::String(parameterName.removeCharacters(" ") + "_LapOnsetDetection") },
                                 paramLapOD,
                                 nullptr);
    }

    void setParamOffsetLoudness(double value)
    {
        paramOffsetLoudness = value;
        mAPVTS.state.setProperty({ juce::String(parameterName.removeCharacters(" ") + "_OffsetLoudness") },
                                 paramOffsetLoudness,
                                 nullptr);
    }

    void setParamOffsetPitch(double value)
    {
        paramOffsetPitch = value;
        mAPVTS.state.setProperty({ juce::String(parameterName.removeCharacters(" ") + "_OffsetPitch") },
                                 paramOffsetPitch,
                                 nullptr);
    }

    void setParamOffsetCentroid(double value)
    {
        paramOffsetCentroid = value;
        mAPVTS.state.setProperty({ juce::String(parameterName.removeCharacters(" ") + "_OffsetCentroid") },
                                 paramOffsetCentroid,
                                 nullptr);
    }

    void setParamOffsetSpread(double value)
    {
        paramOffsetSpread = value;
        mAPVTS.state.setProperty({ juce::String(parameterName.removeCharacters(" ") + "_OffsetSpread") },
                                 paramOffsetSpread,
                                 nullptr);
    }

    void setParamOffsetNoise(double value)
    {
        paramOffsetNoise = value;
        mAPVTS.state.setProperty({ juce::String(parameterName.removeCharacters(" ") + "_OffsetNoise") },
                                 paramOffsetNoise,
                                 nullptr);
    }

    void setParamOffsetOnsetDetection(double value)
    {
        paramOffsetOD = value;
        mAPVTS.state.setProperty({ juce::String(parameterName.removeCharacters(" ") + "_OffsetOnsetDetection") },
                                 paramOffsetOD,
                                 nullptr);
    }

    void setParamMinFreqPitch(double value)
    {
        paramMinFreqPitch = value;
        mAPVTS.state.setProperty({ juce::String(parameterName.removeCharacters(" ") + "_MinFreqPitch") },
                                 paramMinFreqPitch,
                                 nullptr);
    }

    void setParamMinFreqCentroid(double value)
    {
        paramMinFreqCentroid = value;
        mAPVTS.state.setProperty({ juce::String(parameterName.removeCharacters(" ") + "_MinFreqCentroid") },
                                 paramMinFreqCentroid,
                                 nullptr);
    }

    void setParamMaxFreqPitch(double value)
    {
        paramMaxFreqPitch = value;
        mAPVTS.state.setProperty({ juce::String(parameterName.removeCharacters(" ") + "_MaxFreqPitch") },
                                 paramMaxFreqPitch,
                                 nullptr);
    }

    void setParamMaxFreqCentroid(double value)
    {
        paramMaxFreqCentroid = value;
        mAPVTS.state.setProperty({ juce::String(parameterName.removeCharacters(" ") + "_MaxFreqCentroid") },
                                 paramMaxFreqCentroid,
                                 nullptr);
    }

    void setParamMetricComboboxIndex(int value)
    {
        paramMetricComboBoxIndex = value;
        mAPVTS.state.setProperty({ juce::String(parameterName.removeCharacters(" ") + "_MetricOD") },
                                 paramMetricComboBoxIndex,
                                 nullptr);
    }

    void setParamThreshold(double threshold)
    {
        paramThreshold = threshold;
        mAPVTS.state.setProperty({ juce::String(parameterName.removeCharacters(" ") + "_ThresholdOD") },
                                 paramThreshold,
                                 nullptr);
    }

    void setParamMinTime(double minTime)
    {
        paramMinTime = minTime;
        mAPVTS.state.setProperty({ juce::String(parameterName.removeCharacters(" ") + "_MinTimeOD") },
                                 paramMinTime,
                                 nullptr);
    }

    void setParamMaxTime(double maxTime)
    {
        paramMaxTime = maxTime;
        mAPVTS.state.setProperty({ juce::String(parameterName.removeCharacters(" ") + "_MaxTimeOD") },
                                 paramMaxTime,
                                 nullptr);
    }

    //====================================================================
    void updateParameterState()
    {
        paramDescriptorComboBoxIndex
            = mAPVTS.state.getProperty({ juce::String(parameterName.removeCharacters(" ") + "_DescriptorIndex") })
                  .toString()
                  .getIntValue();

        paramExpanderLoudness
            = mAPVTS.state.getProperty({ juce::String(parameterName.removeCharacters(" ") + "_ExpanderLoudness") })
                  .toString()
                  .getDoubleValue();
        paramExpanderSpread
            = mAPVTS.state.getProperty({ juce::String(parameterName.removeCharacters(" ") + "_ExpanderSpread") })
                  .toString()
                  .getDoubleValue();
        paramExpanderNoise
            = mAPVTS.state.getProperty({ juce::String(parameterName.removeCharacters(" ") + "_ExpanderNoise") })
                  .toString()
                  .getDoubleValue();

        paramSmoothLoudness
            = mAPVTS.state.getProperty({ juce::String(parameterName.removeCharacters(" ") + "_SmoothLoudness") })
                  .toString()
                  .getDoubleValue();
        paramSmoothPitch
            = mAPVTS.state.getProperty({ juce::String(parameterName.removeCharacters(" ") + "_SmoothPitch") })
                  .toString()
                  .getDoubleValue();
        paramSmoothCentroid
            = mAPVTS.state.getProperty({ juce::String(parameterName.removeCharacters(" ") + "_SmoothCentroid") })
                  .toString()
                  .getDoubleValue();
        paramSmoothSpread
            = mAPVTS.state.getProperty({ juce::String(parameterName.removeCharacters(" ") + "_SmoothSpread") })
                  .toString()
                  .getDoubleValue();
        paramSmoothNoise
            = mAPVTS.state.getProperty({ juce::String(parameterName.removeCharacters(" ") + "_SmoothNoise") })
                  .toString()
                  .getDoubleValue();
        paramSmoothOD
            = mAPVTS.state.getProperty({ juce::String(parameterName.removeCharacters(" ") + "_SmoothOnsetDetection") })
                  .toString()
                  .getDoubleValue();

        paramSmoothCoefLoudness
            = mAPVTS.state.getProperty({ juce::String(parameterName.removeCharacters(" ") + "_SmoothCoefLoudness") })
                  .toString()
                  .getDoubleValue();
        paramSmoothCoefPitch
            = mAPVTS.state.getProperty({ juce::String(parameterName.removeCharacters(" ") + "_SmoothCoefPitch") })
                  .toString()
                  .getDoubleValue();
        paramSmoothCoefCentroid
            = mAPVTS.state.getProperty({ juce::String(parameterName.removeCharacters(" ") + "_SmoothCoefCentroid") })
                  .toString()
                  .getDoubleValue();
        paramSmoothCoefSpread
            = mAPVTS.state.getProperty({ juce::String(parameterName.removeCharacters(" ") + "_SmoothCoefSpread") })
                  .toString()
                  .getDoubleValue();
        paramSmoothCoefNoise
            = mAPVTS.state.getProperty({ juce::String(parameterName.removeCharacters(" ") + "_SmoothCoefNoise") })
                  .toString()
                  .getDoubleValue();
        paramSmoothCoefOD
            = mAPVTS.state
                  .getProperty({ juce::String(parameterName.removeCharacters(" ") + "_SmoothCoefOnsetDetection") })
                  .toString()
                  .getDoubleValue();

        paramRangeLoudness
            = mAPVTS.state.getProperty({ juce::String(parameterName.removeCharacters(" ") + "_RangeLoudness") })
                  .toString()
                  .getDoubleValue();
        paramRangePitch
            = mAPVTS.state.getProperty({ juce::String(parameterName.removeCharacters(" ") + "_RangePitch") })
                  .toString()
                  .getDoubleValue();
        paramRangeCentroid
            = mAPVTS.state.getProperty({ juce::String(parameterName.removeCharacters(" ") + "_RangeCentroid") })
                  .toString()
                  .getDoubleValue();
        paramRangeSpread
            = mAPVTS.state.getProperty({ juce::String(parameterName.removeCharacters(" ") + "_RangeSpread") })
                  .toString()
                  .getDoubleValue();
        paramRangeNoise
            = mAPVTS.state.getProperty({ juce::String(parameterName.removeCharacters(" ") + "_RangeNoise") })
                  .toString()
                  .getDoubleValue();
        paramRangeOD
            = mAPVTS.state.getProperty({ juce::String(parameterName.removeCharacters(" ") + "_RangeOnsetDetection") })
                  .toString()
                  .getDoubleValue();

        paramLapLoudness
            = mAPVTS.state.getProperty({ juce::String(parameterName.removeCharacters(" ") + "_LapLoudness") })
                  .toString()
                  .getDoubleValue();
        paramLapPitch = mAPVTS.state.getProperty({ juce::String(parameterName.removeCharacters(" ") + "_LapPitch") })
                            .toString()
                            .getDoubleValue();
        paramLapCentroid
            = mAPVTS.state.getProperty({ juce::String(parameterName.removeCharacters(" ") + "_LapCentroid") })
                  .toString()
                  .getDoubleValue();
        paramLapSpread = mAPVTS.state.getProperty({ juce::String(parameterName.removeCharacters(" ") + "_LapSpread") })
                             .toString()
                             .getDoubleValue();
        paramLapNoise = mAPVTS.state.getProperty({ juce::String(parameterName.removeCharacters(" ") + "_LapNoise") })
                            .toString()
                            .getDoubleValue();
        paramLapOD
            = mAPVTS.state.getProperty({ juce::String(parameterName.removeCharacters(" ") + "_LapOnsetDetection") })
                  .toString()
                  .getDoubleValue();

        paramOffsetLoudness
            = mAPVTS.state.getProperty({ juce::String(parameterName.removeCharacters(" ") + "_OffsetLoudness") })
                  .toString()
                  .getDoubleValue();
        paramOffsetPitch
            = mAPVTS.state.getProperty({ juce::String(parameterName.removeCharacters(" ") + "_OffsetPitch") })
                  .toString()
                  .getDoubleValue();
        paramOffsetCentroid
            = mAPVTS.state.getProperty({ juce::String(parameterName.removeCharacters(" ") + "_OffsetCentroid") })
                  .toString()
                  .getDoubleValue();
        paramOffsetSpread
            = mAPVTS.state.getProperty({ juce::String(parameterName.removeCharacters(" ") + "_OffsetSpread") })
                  .toString()
                  .getDoubleValue();
        paramOffsetNoise
            = mAPVTS.state.getProperty({ juce::String(parameterName.removeCharacters(" ") + "_OffsetNoise") })
                  .toString()
                  .getDoubleValue();
        paramOffsetOD
            = mAPVTS.state.getProperty({ juce::String(parameterName.removeCharacters(" ") + "_OffsetOnsetDetection") })
                  .toString()
                  .getDoubleValue();

        paramMinFreqPitch
            = mAPVTS.state.getProperty({ juce::String(parameterName.removeCharacters(" ") + "_MinFreqPitch") })
                  .toString()
                  .getDoubleValue();
        paramMinFreqCentroid
            = mAPVTS.state.getProperty({ juce::String(parameterName.removeCharacters(" ") + "_MinFreqCentroid") })
                  .toString()
                  .getDoubleValue();

        paramMaxFreqPitch
            = mAPVTS.state.getProperty({ juce::String(parameterName.removeCharacters(" ") + "_MaxFreqPitch") })
                  .toString()
                  .getDoubleValue();
        paramMaxFreqCentroid
            = mAPVTS.state.getProperty({ juce::String(parameterName.removeCharacters(" ") + "_MaxFreqCentroid") })
                  .toString()
                  .getDoubleValue();

        paramMetricComboBoxIndex
            = mAPVTS.state.getProperty({ juce::String(parameterName.removeCharacters(" ") + "_MetricOD") })
                  .toString()
                  .getIntValue();

        paramThreshold
            = mAPVTS.state.getProperty({ juce::String(parameterName.removeCharacters(" ") + "_ThresholdOD") })
                  .toString()
                  .getDoubleValue();

        paramMinTime = mAPVTS.state.getProperty({ juce::String(parameterName.removeCharacters(" ") + "_MinTimeOD") })
                           .toString()
                           .getDoubleValue();

        paramMaxTime = mAPVTS.state.getProperty({ juce::String(parameterName.removeCharacters(" ") + "_MaxTimeOD") })
                           .toString()
                           .getDoubleValue();
    }

    void setParametersState()
    {
        mAPVTS.state.setProperty({ juce::String(parameterName.removeCharacters(" ") + "_DescriptorIndex") },
                                 paramDescriptorComboBoxIndex,
                                 nullptr);

        mAPVTS.state.setProperty({ juce::String(parameterName.removeCharacters(" ") + "_ExpanderLoudness") },
                                 paramExpanderLoudness,
                                 nullptr);
        mAPVTS.state.setProperty({ juce::String(parameterName.removeCharacters(" ") + "_ExpanderSpread") },
                                 paramExpanderSpread,
                                 nullptr);
        mAPVTS.state.setProperty({ juce::String(parameterName.removeCharacters(" ") + "_ExpanderNoise") },
                                 paramExpanderNoise,
                                 nullptr);

        mAPVTS.state.setProperty({ juce::String(parameterName.removeCharacters(" ") + "_SmoothLoudness") },
                                 paramSmoothLoudness,
                                 nullptr);
        mAPVTS.state.setProperty({ juce::String(parameterName.removeCharacters(" ") + "_SmoothPitch") },
                                 paramSmoothPitch,
                                 nullptr);
        mAPVTS.state.setProperty({ juce::String(parameterName.removeCharacters(" ") + "_SmoothCentroid") },
                                 paramSmoothCentroid,
                                 nullptr);
        mAPVTS.state.setProperty({ juce::String(parameterName.removeCharacters(" ") + "_SmoothSpread") },
                                 paramSmoothSpread,
                                 nullptr);
        mAPVTS.state.setProperty({ juce::String(parameterName.removeCharacters(" ") + "_SmoothNoise") },
                                 paramSmoothNoise,
                                 nullptr);
        mAPVTS.state.setProperty({ juce::String(parameterName.removeCharacters(" ") + "_SmoothOnsetDetection") },
                                 paramSmoothOD,
                                 nullptr);

        mAPVTS.state.setProperty({ juce::String(parameterName.removeCharacters(" ") + "_SmoothCoefLoudness") },
                                 paramSmoothCoefLoudness,
                                 nullptr);
        mAPVTS.state.setProperty({ juce::String(parameterName.removeCharacters(" ") + "_SmoothCoefPitch") },
                                 paramSmoothCoefPitch,
                                 nullptr);
        mAPVTS.state.setProperty({ juce::String(parameterName.removeCharacters(" ") + "_SmoothCoefCentroid") },
                                 paramSmoothCoefCentroid,
                                 nullptr);
        mAPVTS.state.setProperty({ juce::String(parameterName.removeCharacters(" ") + "_SmoothCoefSpread") },
                                 paramSmoothCoefSpread,
                                 nullptr);
        mAPVTS.state.setProperty({ juce::String(parameterName.removeCharacters(" ") + "_SmoothCoefNoise") },
                                 paramSmoothCoefNoise,
                                 nullptr);
        mAPVTS.state.setProperty({ juce::String(parameterName.removeCharacters(" ") + "_SmoothCoefOnsetDetection") },
                                 paramSmoothCoefOD,
                                 nullptr);

        mAPVTS.state.setProperty({ juce::String(parameterName.removeCharacters(" ") + "_RangeLoudness") },
                                 paramRangeLoudness,
                                 nullptr);
        mAPVTS.state.setProperty({ juce::String(parameterName.removeCharacters(" ") + "_RangePitch") },
                                 paramRangePitch,
                                 nullptr);
        mAPVTS.state.setProperty({ juce::String(parameterName.removeCharacters(" ") + "_RangeCentroid") },
                                 paramRangeCentroid,
                                 nullptr);
        mAPVTS.state.setProperty({ juce::String(parameterName.removeCharacters(" ") + "_RangeSpread") },
                                 paramRangeSpread,
                                 nullptr);
        mAPVTS.state.setProperty({ juce::String(parameterName.removeCharacters(" ") + "_RangeNoise") },
                                 paramRangeNoise,
                                 nullptr);
        mAPVTS.state.setProperty({ juce::String(parameterName.removeCharacters(" ") + "_RangeOnsetDetection") },
                                 paramRangeOD,
                                 nullptr);

        mAPVTS.state.setProperty({ juce::String(parameterName.removeCharacters(" ") + "_LapLoudness") },
                                 paramLapLoudness,
                                 nullptr);
        mAPVTS.state.setProperty({ juce::String(parameterName.removeCharacters(" ") + "_LapPitch") },
                                 paramLapPitch,
                                 nullptr);
        mAPVTS.state.setProperty({ juce::String(parameterName.removeCharacters(" ") + "_LapCentroid") },
                                 paramLapCentroid,
                                 nullptr);
        mAPVTS.state.setProperty({ juce::String(parameterName.removeCharacters(" ") + "_LapSpread") },
                                 paramLapSpread,
                                 nullptr);
        mAPVTS.state.setProperty({ juce::String(parameterName.removeCharacters(" ") + "_LapNoise") },
                                 paramLapNoise,
                                 nullptr);
        mAPVTS.state.setProperty({ juce::String(parameterName.removeCharacters(" ") + "_LapOnsetDetection") },
                                 paramLapOD,
                                 nullptr);

        mAPVTS.state.setProperty({ juce::String(parameterName.removeCharacters(" ") + "_OffsetLoudness") },
                                 paramOffsetLoudness,
                                 nullptr);
        mAPVTS.state.setProperty({ juce::String(parameterName.removeCharacters(" ") + "_OffsetPitch") },
                                 paramOffsetPitch,
                                 nullptr);
        mAPVTS.state.setProperty({ juce::String(parameterName.removeCharacters(" ") + "_OffsetCentroid") },
                                 paramOffsetCentroid,
                                 nullptr);
        mAPVTS.state.setProperty({ juce::String(parameterName.removeCharacters(" ") + "_OffsetSpread") },
                                 paramOffsetSpread,
                                 nullptr);
        mAPVTS.state.setProperty({ juce::String(parameterName.removeCharacters(" ") + "_OffsetNoise") },
                                 paramOffsetNoise,
                                 nullptr);
        mAPVTS.state.setProperty({ juce::String(parameterName.removeCharacters(" ") + "_OffsetOnsetDetection") },
                                 paramOffsetOD,
                                 nullptr);

        mAPVTS.state.setProperty({ juce::String(parameterName.removeCharacters(" ") + "_MinFreqPitch") },
                                 paramMinFreqPitch,
                                 nullptr);
        mAPVTS.state.setProperty({ juce::String(parameterName.removeCharacters(" ") + "_MinFreqCentroid") },
                                 paramMinFreqCentroid,
                                 nullptr);

        mAPVTS.state.setProperty({ juce::String(parameterName.removeCharacters(" ") + "_MaxFreqPitch") },
                                 paramMaxFreqPitch,
                                 nullptr);
        mAPVTS.state.setProperty({ juce::String(parameterName.removeCharacters(" ") + "_MaxFreqCentroid") },
                                 paramMaxFreqCentroid,
                                 nullptr);

        mAPVTS.state.setProperty({ juce::String(parameterName.removeCharacters(" ") + "_MetricOD") },
                                 paramMetricComboBoxIndex,
                                 nullptr);

        mAPVTS.state.setProperty({ juce::String(parameterName.removeCharacters(" ") + "_ThresholdOD") },
                                 paramThreshold,
                                 nullptr);

        mAPVTS.state.setProperty({ juce::String(parameterName.removeCharacters(" ") + "_MinTimeOD") },
                                 paramMinTime,
                                 nullptr);

        mAPVTS.state.setProperty({ juce::String(parameterName.removeCharacters(" ") + "_MaxTimeOD") },
                                 paramMaxTime,
                                 nullptr);
    }

protected:
    //==============================================================================
    juce::AudioProcessorValueTreeState & mAPVTS;
    SpatParamHelperFunctions & mFunctions;
    ParameterID paramID = ParameterID::invalid;
    double res{};
    double lastRes{};
    juce::String parameterName{};
    DescriptorID mDescriptorToUse;

    Smooth mSmoothLoudness;
    Smooth mSmoothPitch;
    Smooth mSmoothCentroid;
    Smooth mSmoothSpread;
    Smooth mSmoothNoise;
    Smooth mSmoothOnsetDetection;

    int paramDescriptorComboBoxIndex = 1;

    double paramExpanderLoudness = 100.0;
    double paramExpanderSpread = 100.0;
    double paramExpanderNoise = 100.0;

    double paramSmoothLoudness = 5.0;
    double paramSmoothPitch = 5.0;
    double paramSmoothCentroid = 5.0;
    double paramSmoothSpread = 5.0;
    double paramSmoothNoise = 5.0;
    double paramSmoothOD = 5.0;

    double paramSmoothCoefLoudness = 0.0;
    double paramSmoothCoefPitch = 0.0;
    double paramSmoothCoefCentroid = 0.0;
    double paramSmoothCoefSpread = 0.0;
    double paramSmoothCoefNoise = 0.0;
    double paramSmoothCoefOD = 0.0;

    double paramRangeLoudness = 100.0;
    double paramRangePitch = 100.0;
    double paramRangeCentroid = 100.0;
    double paramRangeSpread = 100.0;
    double paramRangeNoise = 100.0;
    double paramRangeOD = 100.0;

    double paramLapLoudness = 1.0;
    double paramLapPitch = 1.0;
    double paramLapCentroid = 1.0;
    double paramLapSpread = 1.0;
    double paramLapNoise = 1.0;
    double paramLapOD = 1.0;

    double paramOffsetLoudness = 0.0;
    double paramOffsetPitch = 0.0;
    double paramOffsetCentroid = 0.0;
    double paramOffsetSpread = 0.0;
    double paramOffsetNoise = 0.0;
    double paramOffsetOD = 0.0;

    double paramMinFreqPitch = 20.0;
    double paramMinFreqCentroid = 20.0;

    double paramMaxFreqPitch = 5000.0;
    double paramMaxFreqCentroid = 10000.0;

    int paramMetricComboBoxIndex = 4;
    double paramThreshold = 0.1;
    double paramMinTime = 0.1;
    double paramMaxTime = 10.0;

private:
    //==============================================================================
    // std::vector<PanelView *> mObservers;

    //==============================================================================
    JUCE_LEAK_DETECTOR(SpatialParameter)
};
} // namespace gris
