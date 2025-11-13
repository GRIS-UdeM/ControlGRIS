/*
 This file is part of ControlGris.

 Developers: Gaël LANE LÉPINE

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

#include "SpatialParameters/cg_SpatialParameter.h"
#include "cg_ControlGrisAudioProcessor.hpp"
#include "cg_ControlGrisLookAndFeel.hpp"
#include "cg_NumSlider.h"
#include "cg_TextEditor.hpp"
#include "cg_constants.hpp"

namespace gris
{
//==============================================================================
class DataGraph
    : public juce::Component
    , private juce::Timer
{
public:
    //==============================================================================
    DataGraph(GrisLookAndFeel & grisLookAndFeel);
    ~DataGraph() override;

    void paint(juce::Graphics &) override;
    void resized() override;
    void timerCallback() override;

    void addToBuffer(double value);
    double readBufferMean();
    void setDescriptor(DescriptorID descId);

private:
    //==============================================================================
    GrisLookAndFeel & mGrisLookAndFeel;
    DescriptorID mDescId;
    std::deque<double> mGUIBuffer;
    juce::Atomic<double> mBuffer{};
    juce::Atomic<int> mBufferCount{};

    //==============================================================================
    JUCE_LEAK_DETECTOR(DataGraph)
}; // class DataGraph

//==============================================================================
class SectionSoundReactiveTrajectories final
    : public juce::Component
    , private juce::MultiTimer
{
private:
    //==============================================================================
    enum timerParamID { azimuth = 1, elevation, x, y, z, azimuthSpan, elevationSpan, datagraphUpdate };

    GrisLookAndFeel & mGrisLookAndFeel;
    ControlGrisAudioProcessor & mAudioProcessor;
    juce::AudioProcessorValueTreeState & mAPVTS;
    SpatMode mSpatMode;

    //==============================================================================
    // Spatial parameters section
    bool mXYParamLinked{};
    juce::ImageButton mPadLockButton;

    juce::Label mSpatialParameterLabel;

    juce::Label mChannelMixLabel;
    juce::ComboBox mChannelMixCombo;
    juce::Label mGainLabel;
    NumSlider mGainSlider;

    std::optional<std::reference_wrapper<juce::TextButton>> mLastUsedParameterDomeButton;
    std::optional<std::reference_wrapper<juce::TextButton>> mLastUsedParameterCubeButton;

    juce::TextButton mParameterAzimuthButton;
    juce::TextButton mParameterElevationButton;
    juce::TextButton mParameterXButton;
    juce::TextButton mParameterYButton;
    juce::TextButton mParameterZButton;
    juce::TextButton mParameterAzimuthOrXYSpanButton;
    juce::TextButton mParameterElevationOrZSpanButton;

    std::array<juce::TextButton *, 4> mParameterButtonDomeRefs; // just an array of references to dome parameter buttons
    std::array<juce::TextButton *, 5> mParameterButtonCubeRefs; // just an array of references to cube parameter buttons

    juce::ComboBox mParameterAzimuthDescriptorCombo;
    juce::ComboBox mParameterElevationDescriptorCombo;
    juce::ComboBox mParameterXDescriptorCombo;
    juce::ComboBox mParameterYDescriptorCombo;
    juce::ComboBox mParameterZDescriptorCombo;
    juce::ComboBox mParameterAzimuthOrXYSpanDescriptorCombo;
    juce::ComboBox mParameterElevationOrZSpanDescriptorCombo;

    juce::Label mParameterRangeLabel;

    NumSlider mParameterAzimuthRangeSlider;
    NumSlider mParameterElevationRangeSlider;
    NumSlider mParameterXRangeSlider;
    NumSlider mParameterYRangeSlider;
    NumSlider mParameterZRangeSlider;
    NumSlider mParameterAzimuthOrXYSpanRangeSlider;
    NumSlider mParameterElevationOrZSpanRangeSlider;

    juce::Label mParameterOffsetLabel;

    NumSlider mParameterAzimuthXOffsetSlider;
    NumSlider mParameterYOffsetSlider;
    NumSlider mParameterElevationZOffsetSlider;
    NumSlider mParameterAziXYSpanOffsetSlider;
    NumSlider mParameterEleZSpanOffsetSlider;

    juce::Label mParameterLapLabel;
    TextEd mParameterLapEditor;

    juce::TextButton mAudioAnalysisActivateButton;

    //==============================================================================
    // Audio anaylysis section
    juce::Label mAudioAnalysisLabel;
    std::optional<std::reference_wrapper<SpatialParameter>> mParameterToShow;
    DescriptorID mDescriptorIdToUse{ DescriptorID::invalid };
    DataGraph mDataGraph;

    juce::Label mDescriptorExpanderLabel;
    juce::Label mDescriptorThresholdLabel;
    juce::Label mDescriptorMinFreqLabel;
    juce::Label mDescriptorMaxFreqLabel;
    juce::Label mDescriptorMinTimeLabel;
    juce::Label mDescriptorMaxTimeLabel;
    juce::Label mDescriptorSmoothLabel;
    juce::Label mDescriptorSmoothCoefLabel;
    juce::Label mDescriptorMetricLabel;

    juce::ComboBox mDescriptorMetricCombo;

    NumSlider mDescriptorExpanderSlider;
    NumSlider mDescriptorThresholdSlider;
    NumSlider mDescriptorMinFreqSlider;
    NumSlider mDescriptorMaxFreqSlider;
    NumSlider mDescriptorMinTimeSlider;
    NumSlider mDescriptorMaxTimeSlider;
    NumSlider mDescriptorSmoothSlider;
    NumSlider mDescriptorSmoothCoefSlider;

    juce::TextButton mClickTimerButton;

    int mOnsetDetectiontimerCounterAzimuth{};
    int mOnsetDetectiontimerCounterElevation{};
    int mOnsetDetectiontimerCounterX{};
    int mOnsetDetectiontimerCounterY{};
    int mOnsetDetectiontimerCounterZ{};
    int mOnsetDetectiontimerCounterAzimuthSpan{};
    int mOnsetDetectiontimerCounterElevationSpan{};

    juce::Rectangle<int> mAreaAudioAnalysis;

public:
    //==============================================================================
    explicit SectionSoundReactiveTrajectories(GrisLookAndFeel & grisLookAndFeel,
                                              ControlGrisAudioProcessor & audioProcessor);
    //==============================================================================
    SectionSoundReactiveTrajectories() = delete;
    ~SectionSoundReactiveTrajectories() override = default;

    SectionSoundReactiveTrajectories(SectionSoundReactiveTrajectories const &) = delete;
    SectionSoundReactiveTrajectories(SectionSoundReactiveTrajectories &&) = delete;

    SectionSoundReactiveTrajectories & operator=(SectionSoundReactiveTrajectories const &) = delete;
    SectionSoundReactiveTrajectories & operator=(SectionSoundReactiveTrajectories &&) = delete;
    //==============================================================================

    //==============================================================================
    // overrides
    void paint(juce::Graphics & g) override;
    void resized() override;
    void timerCallback(int timerID) override;

    //==============================================================================
    void setSpatMode(SpatMode spatMode);
    void addNewParamValueToDataGraph();

    bool getAudioAnalysisActivateState();
    void setAudioAnalysisActivateState(bool state);

    void updateChannelMixCombo();

private:
    //==============================================================================
    void unselectAllParamButtons();
    void refreshDescriptorPanel();
    void loudnessSpreadNoiseDescriptorLayout();
    void pitchCentroidDescriptorLayout();
    void iterSpeedDescriptorLayout();
    void setAudioAnalysisComponentsInvisible();
    void changeMinMaxSlidersRange(int min, int max);

    //==============================================================================
    JUCE_LEAK_DETECTOR(SectionSoundReactiveTrajectories)
};
} // namespace gris
