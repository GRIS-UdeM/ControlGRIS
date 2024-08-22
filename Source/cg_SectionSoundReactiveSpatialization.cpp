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

#include "cg_SectionSoundReactiveSpatialization.h"

//==============================================================================
gris::SectionSoundReactiveSpatialization::SectionSoundReactiveSpatialization(GrisLookAndFeel & grisLookAndFeel,
    ControlGrisAudioProcessor & audioProcessor)
    : mGrisLookAndFeel(grisLookAndFeel)
    , mAudioProcessor(audioProcessor)
    , mDescriptorFactorSlider(grisLookAndFeel)
    , mDescriptorThresholdSlider(grisLookAndFeel)
    , mDescriptorMinFreqSlider(grisLookAndFeel)
    , mDescriptorMaxFreqSlider(grisLookAndFeel)
    , mDescriptorMinTimeSlider(grisLookAndFeel)
    , mDescriptorMaxTimeSlider(grisLookAndFeel)
    , mDescriptorSmoothSlider(grisLookAndFeel)
    , mDescriptorSmoothCoefSlider(grisLookAndFeel)
    , mParameterAzimuthRangeSlider(grisLookAndFeel)
    , mParameterElevationRangeSlider(grisLookAndFeel)
    , mParameterXRangeSlider(grisLookAndFeel)
    , mParameterYRangeSlider(grisLookAndFeel)
    , mParameterZRangeSlider(grisLookAndFeel)
    , mParameterAzimuthOrXYSpanRangeSlider(grisLookAndFeel)
    , mParameterElevationOrZSpanRangeSlider(grisLookAndFeel)
    , mParameterElevationZOffsetSlider(grisLookAndFeel)
    , mParameterEleZSpanOffsetSlider(grisLookAndFeel)
{
    auto const initRangeSlider = [&](NumSlider & slider) {
        slider.setNormalisableRange(juce::NormalisableRange<double>{ -100.0, 100.0, 0.1 });
        slider.setValue(100.0, juce::dontSendNotification);
        slider.setNumDecimalPlacesToDisplay(1);
        addAndMakeVisible(slider);
    };

    auto const initParameterDescCombo = [&](juce::ComboBox & combo) {
        combo.addItemList(AUDIO_DESCRIPTOR_TYPES, 1);
        combo.setSelectedId(1, juce::dontSendNotification);
        addAndMakeVisible(combo);
    };

    setName("Sound Reactive Spatialization");

    addAndMakeVisible(&mSpatialParameterLabel);
    mSpatialParameterLabel.setText("Spatial Parameters", juce::dontSendNotification);
    addAndMakeVisible(&mAudioAnalysisLabel);
    mAudioAnalysisLabel.setText("Audio Analysis", juce::dontSendNotification);

    //==============================================================================
    // Spatial Parameters

    // default values
    mParameterAzimuthRangeSlider.setDoubleClickReturnValue(true, 100.0);
    mParameterElevationRangeSlider.setDoubleClickReturnValue(true, 100.0);
    mParameterXRangeSlider.setDoubleClickReturnValue(true, 100.0);
    mParameterYRangeSlider.setDoubleClickReturnValue(true, 100.0);
    mParameterZRangeSlider.setDoubleClickReturnValue(true, 100.0);
    mParameterAzimuthOrXYSpanRangeSlider.setDoubleClickReturnValue(true, 100.0);
    mParameterElevationOrZSpanRangeSlider.setDoubleClickReturnValue(true, 100.0);
    mParameterElevationZOffsetSlider.setDoubleClickReturnValue(true, 0.0);
    mParameterEleZSpanOffsetSlider.setDoubleClickReturnValue(true, 0.0);

    mParameterAzimuthRangeSlider.setNumDecimalPlacesToDisplay(3);
    mParameterElevationRangeSlider.setNumDecimalPlacesToDisplay(3);
    mParameterXRangeSlider.setNumDecimalPlacesToDisplay(3);
    mParameterYRangeSlider.setNumDecimalPlacesToDisplay(3);
    mParameterZRangeSlider.setNumDecimalPlacesToDisplay(3);
    mParameterAzimuthOrXYSpanRangeSlider.setNumDecimalPlacesToDisplay(3);
    mParameterElevationOrZSpanRangeSlider.setNumDecimalPlacesToDisplay(3);
    mParameterElevationZOffsetSlider.setNumDecimalPlacesToDisplay(3);
    mParameterEleZSpanOffsetSlider.setNumDecimalPlacesToDisplay(3);

    addAndMakeVisible(&mParameterAzimuthButton);
    mParameterAzimuthButton.setButtonText("Azimuth");
    mParameterAzimuthButton.setClickingTogglesState(true);
    mParameterAzimuthButton.onClick = [this] {
        if (mParameterAzimuthButton.getToggleState()) {
            unselectAllParamButtons();
            mParameterAzimuthButton.setToggleState(true, juce::dontSendNotification);
            mParameterAzimuthDescriptorCombo.onChange();
            mLastUsedParameterDome = mParameterAzimuthButton;
        }
    };

    addAndMakeVisible(&mParameterElevationButton);
    mParameterElevationButton.setButtonText("Elevation");
    mParameterElevationButton.setClickingTogglesState(true);
    mParameterElevationButton.onClick = [this] {
        if (mParameterElevationButton.getToggleState()) {
            unselectAllParamButtons();
            mParameterElevationButton.setToggleState(true, juce::dontSendNotification);
            mParameterElevationDescriptorCombo.onChange();
            mLastUsedParameterDome = mParameterElevationButton;
        }
    };

    addAndMakeVisible(&mParameterXButton);
    mParameterXButton.setButtonText("X");
    mParameterXButton.setClickingTogglesState(true);
    mParameterXButton.onClick = [this] {
        if (mParameterXButton.getToggleState()) {
            unselectAllParamButtons();
            mParameterXButton.setToggleState(true, juce::dontSendNotification);
            mParameterXDescriptorCombo.onChange();
            mLastUsedParameterCube = mParameterXButton;
        }
    };

    addAndMakeVisible(&mParameterYButton);
    mParameterYButton.setButtonText("Y");
    mParameterYButton.setClickingTogglesState(true);
    mParameterYButton.onClick = [this] {
        if (mParameterYButton.getToggleState()) {
            unselectAllParamButtons();
            mParameterYButton.setToggleState(true, juce::dontSendNotification);
            mParameterYDescriptorCombo.onChange();
            mLastUsedParameterCube = mParameterYButton;
        }
    };
    
    addAndMakeVisible(&mParameterZButton);
    mParameterZButton.setButtonText("Z");
    mParameterZButton.setClickingTogglesState(true);
    mParameterZButton.onClick = [this] {
        if (mParameterZButton.getToggleState()) {
            unselectAllParamButtons();
            mParameterZButton.setToggleState(true, juce::dontSendNotification);
            mParameterZDescriptorCombo.onChange();
            mLastUsedParameterCube = mParameterZButton;
        }
    };

    addAndMakeVisible(&mParameterAzimuthOrXYSpanButton);
    mParameterAzimuthOrXYSpanButton.setButtonText("Azimuth Span");
    mParameterAzimuthOrXYSpanButton.setClickingTogglesState(true);
    mParameterAzimuthOrXYSpanButton.onClick = [this] {
        if (mParameterAzimuthOrXYSpanButton.getToggleState()) {
            unselectAllParamButtons();
            mParameterAzimuthOrXYSpanButton.setToggleState(true, juce::dontSendNotification);
            mParameterAzimuthOrXYSpanDescriptorCombo.onChange();
            if (mSpatMode == SpatMode::dome) {
                mLastUsedParameterDome = mParameterAzimuthOrXYSpanButton;
            }
            else {
                mLastUsedParameterCube = mParameterAzimuthOrXYSpanButton;
            }
        }
    };

    addAndMakeVisible(&mParameterElevationOrZSpanButton);
    mParameterElevationOrZSpanButton.setButtonText("Elevation Span");
    mParameterElevationOrZSpanButton.setClickingTogglesState(true);
    mParameterElevationOrZSpanButton.onClick = [this] {
        if (mParameterElevationOrZSpanButton.getToggleState()) {
            unselectAllParamButtons();
            mParameterElevationOrZSpanButton.setToggleState(true, juce::dontSendNotification);
            mParameterElevationOrZSpanDescriptorCombo.onChange();
            if (mSpatMode == SpatMode::dome) {
                mLastUsedParameterDome = mParameterElevationOrZSpanButton;
            } else {
                mLastUsedParameterCube = mParameterElevationOrZSpanButton;
            }
        }
    };

    initParameterDescCombo(mParameterAzimuthDescriptorCombo);
    mParameterAzimuthDescriptorCombo.onChange = [this] {
        unselectAllParamButtons();
        mParameterAzimuthButton.setToggleState(true, juce::dontSendNotification);
        mParameterToShow = mAudioProcessor.getAzimuthDome();
        mDescriptorIdToUse = Descriptor::fromInt(mParameterAzimuthDescriptorCombo.getSelectedId()); 
        refreshDescriptorPanel();
    };

    initParameterDescCombo(mParameterElevationDescriptorCombo);
    mParameterElevationDescriptorCombo.onChange = [this] {
        unselectAllParamButtons();
        mParameterElevationButton.setToggleState(true, juce::dontSendNotification);
        mParameterToShow = mAudioProcessor.getElevationDome();
        mDescriptorIdToUse = Descriptor::fromInt(mParameterElevationDescriptorCombo.getSelectedId());
        refreshDescriptorPanel();
    };

    initParameterDescCombo(mParameterXDescriptorCombo);
    mParameterXDescriptorCombo.onChange = [this] {
        unselectAllParamButtons();
        mParameterXButton.setToggleState(true, juce::dontSendNotification);
        mParameterToShow = mAudioProcessor.getXCube();
        mDescriptorIdToUse = Descriptor::fromInt(mParameterXDescriptorCombo.getSelectedId());
        refreshDescriptorPanel();
    };
    
    initParameterDescCombo(mParameterYDescriptorCombo);
    mParameterYDescriptorCombo.onChange = [this] {
        unselectAllParamButtons();
        mParameterYButton.setToggleState(true, juce::dontSendNotification);
        mParameterToShow = mAudioProcessor.getYCube();
        mDescriptorIdToUse = Descriptor::fromInt(mParameterYDescriptorCombo.getSelectedId());
        refreshDescriptorPanel();
    };

    initParameterDescCombo(mParameterZDescriptorCombo);
    mParameterZDescriptorCombo.onChange = [this] {
        unselectAllParamButtons();
        mParameterZButton.setToggleState(true, juce::dontSendNotification);
        mParameterToShow = mAudioProcessor.getZCube();
        mDescriptorIdToUse = Descriptor::fromInt(mParameterZDescriptorCombo.getSelectedId());
        refreshDescriptorPanel();
    };

    initParameterDescCombo(mParameterAzimuthOrXYSpanDescriptorCombo);
    mParameterAzimuthOrXYSpanDescriptorCombo.onChange = [this] {
        unselectAllParamButtons();
        mParameterAzimuthOrXYSpanButton.setToggleState(true, juce::dontSendNotification);
        if (mSpatMode == SpatMode::dome) {
            mParameterToShow = mAudioProcessor.getHSpanDome();
        } else {
            mParameterToShow = mAudioProcessor.getHSpanCube();
        }
        mDescriptorIdToUse = Descriptor::fromInt(mParameterAzimuthOrXYSpanDescriptorCombo.getSelectedId());
        refreshDescriptorPanel();
    };

    initParameterDescCombo(mParameterElevationOrZSpanDescriptorCombo);
    mParameterElevationOrZSpanDescriptorCombo.onChange = [this] {
        unselectAllParamButtons();
        mParameterElevationOrZSpanButton.setToggleState(true, juce::dontSendNotification);
        if (mSpatMode == SpatMode::dome) {
            mParameterToShow = mAudioProcessor.getVSpanDome();
        } else {
            mParameterToShow = mAudioProcessor.getVSpanCube();
        }
        mDescriptorIdToUse = Descriptor::fromInt(mParameterElevationOrZSpanDescriptorCombo.getSelectedId());
        refreshDescriptorPanel();
    };

    //mSourcePlacementCombo.onChange = [this] {
    //    mListeners.call([&](Listener & l) {
    //        l.sourcesPlacementChangedCallback(static_cast<SourcePlacement>(mSourcePlacementCombo.getSelectedId()));
    //        mSourcePlacementCombo.setSelectedId(0, juce::NotificationType::dontSendNotification);
    //    });
    //};

    addAndMakeVisible(&mParameterRangeLabel);
    mParameterRangeLabel.setText("Range", juce::dontSendNotification);

    initRangeSlider(mParameterAzimuthRangeSlider);
    initRangeSlider(mParameterElevationRangeSlider);
    initRangeSlider(mParameterXRangeSlider);
    initRangeSlider(mParameterYRangeSlider);
    initRangeSlider(mParameterZRangeSlider);
    initRangeSlider(mParameterAzimuthOrXYSpanRangeSlider);
    initRangeSlider(mParameterElevationOrZSpanRangeSlider);

    addAndMakeVisible(&mParameterOffsetLabel);
    mParameterOffsetLabel.setText("Offset", juce::dontSendNotification);

    addAndMakeVisible(&mParameterElevationZOffsetSlider);
    mParameterElevationZOffsetSlider.setNormalisableRange(juce::NormalisableRange<double>{ 0.0, 90.0, 0.1 });
    mParameterElevationZOffsetSlider.setValue(0.0, juce::dontSendNotification);

    addAndMakeVisible(&mParameterEleZSpanOffsetSlider);
    mParameterEleZSpanOffsetSlider.setNormalisableRange(juce::NormalisableRange<double>{ 0.0, 1.0, 0.01 });
    mParameterEleZSpanOffsetSlider.setValue(0.0, juce::dontSendNotification);

    addAndMakeVisible(&mParameterLapLabel);
    mParameterLapLabel.setText("Lap", juce::dontSendNotification);

    addAndMakeVisible(&mParameterLapCombo);
    mParameterLapCombo.addItemList({ "1", "2", "3", "4" }, 1);
    mParameterLapCombo.setSelectedId(1);

    //==============================================================================
    // Audio Analysis

    mDescriptorMetricLabel.setText("Metric", juce::dontSendNotification);
    mDescriptorFactorLabel.setText("Factor", juce::dontSendNotification);
    mDescriptorThresholdLabel.setText("Threshold", juce::dontSendNotification);
    mDescriptorMinFreqLabel.setText("Min. Freq", juce::dontSendNotification);
    mDescriptorMaxFreqLabel.setText("Max. Freq", juce::dontSendNotification);
    mDescriptorMinTimeLabel.setText("Min. Time", juce::dontSendNotification);
    mDescriptorMaxTimeLabel.setText("Max. Time", juce::dontSendNotification);
    mDescriptorSmoothLabel.setText("Smooth", juce::dontSendNotification);
    mDescriptorSmoothCoefLabel.setText("Smooth Coef.", juce::dontSendNotification);

    // default values
    mDescriptorFactorSlider.setDoubleClickReturnValue(true, 100.0);
    mDescriptorSmoothSlider.setDoubleClickReturnValue(true, 5.0);
    mDescriptorSmoothCoefSlider.setDoubleClickReturnValue(true, 0.0);
    mDescriptorMinFreqSlider.setDoubleClickReturnValue(true, 20.0);
    mDescriptorMaxFreqSlider.setDoubleClickReturnValue(true, 10000.0);
    mDescriptorThresholdSlider.setDoubleClickReturnValue(true, 0.1);
    mDescriptorMinTimeSlider.setDoubleClickReturnValue(true, 0.1);
    mDescriptorMaxTimeSlider.setDoubleClickReturnValue(true, 10.0);

    mDescriptorFactorSlider.setNumDecimalPlacesToDisplay(1);
    mDescriptorSmoothSlider.setNumDecimalPlacesToDisplay(3);
    mDescriptorSmoothCoefSlider.setNumDecimalPlacesToDisplay(3);
    mDescriptorMinFreqSlider.setNumDecimalPlacesToDisplay(1);
    mDescriptorMaxFreqSlider.setNumDecimalPlacesToDisplay(3);
    mDescriptorThresholdSlider.setNumDecimalPlacesToDisplay(3);
    mDescriptorMinTimeSlider.setNumDecimalPlacesToDisplay(3);
    mDescriptorMaxTimeSlider.setNumDecimalPlacesToDisplay(3);

    mDescriptorFactorSlider.setRange(0, 500);
    mDescriptorSmoothSlider.setRange(0, 100);
    mDescriptorSmoothCoefSlider.setRange(0, 100);
    mDescriptorMinFreqSlider.setRange(20, 20000);
    mDescriptorMaxFreqSlider.setRange(20, 20000);
    mDescriptorThresholdSlider.setRange(0.0, 1.0);
    mDescriptorMinTimeSlider.setRange(0.0, 30.0 + ALMOST_ZERO);
    mDescriptorMaxTimeSlider.setRange(ALMOST_ZERO, 30.0);

    mDescriptorFactorSlider.onValueChange = [this] {
        if (mParameterToShow) {
            auto & param = mParameterToShow->get();
            auto value = mDescriptorFactorSlider.getValue();
            switch (mDescriptorIdToUse) {
            case DescriptorID::loudness:
                param.setParamFactorLoudness(value);
                break;
            case DescriptorID::spread:
                param.setParamFactorSpread(value);
                break;
            case DescriptorID::noise:
                param.setParamFactorNoise(value);
                break;
            case DescriptorID::pitch:
            case DescriptorID::centroid:
            case DescriptorID::iterationsSpeed:
            case DescriptorID::invalid:
            default:
                break;
            }
        }
    };

    mDescriptorSmoothSlider.onValueChange = [this] {
        if (mParameterToShow) {
            auto & param = mParameterToShow->get();
            auto value = mDescriptorSmoothSlider.getValue();
            switch (mDescriptorIdToUse) {
            case DescriptorID::loudness:
                param.setParamSmoothLoudness(value);
                break;
            case DescriptorID::spread:
                param.setParamSmoothSpread(value);
                break;
            case DescriptorID::noise:
                param.setParamSmoothNoise(value);
                break;
            case DescriptorID::pitch:
                param.setParamSmoothPitch(value);
                break;
            case DescriptorID::centroid:
                param.setParamSmoothCentroid(value);
                break;
            case DescriptorID::iterationsSpeed:
                param.setParamSmoothOnsetDetection(value);
                break;
            case DescriptorID::invalid:
            default:
                break;
            }
        }
    };

    mDescriptorSmoothCoefSlider.onValueChange = [this] {
        if (mParameterToShow) {
            auto & param = mParameterToShow->get();
            auto value = mDescriptorSmoothCoefSlider.getValue();
            switch (mDescriptorIdToUse) {
            case DescriptorID::loudness:
                param.setParamSmoothCoefLoudness(value);
                break;
            case DescriptorID::spread:
                param.setParamSmoothCoefSpread(value);
                break;
            case DescriptorID::noise:
                param.setParamSmoothCoefNoise(value);
                break;
            case DescriptorID::pitch:
                param.setParamSmoothCoefPitch(value);
                break;
            case DescriptorID::centroid:
                param.setParamSmoothCoefCentroid(value);
                break;
            case DescriptorID::iterationsSpeed:
                param.setParamSmoothCoefOnsetDetection(value);
                break;
            case DescriptorID::invalid:
            default:
                break;
            }
        }
    };

    mDescriptorMinFreqSlider.onValueChange = [this] {
        if (mParameterToShow) {
            auto & param = mParameterToShow->get();
            auto value = mDescriptorMinFreqSlider.getValue();
            switch (mDescriptorIdToUse) {
            case DescriptorID::pitch:
                param.setParamMinFreqPitch(value);
                break;
            case DescriptorID::centroid:
                param.setParamMinFreqCentroid(value);
                break;
            case DescriptorID::loudness:
            case DescriptorID::spread:
            case DescriptorID::noise:
            case DescriptorID::iterationsSpeed:
            case DescriptorID::invalid:
            default:
                break;
            }
        }
    };

    mDescriptorMaxFreqSlider.onValueChange = [this] {
        if (mParameterToShow) {
            auto & param = mParameterToShow->get();
            auto value = mDescriptorMaxFreqSlider.getValue();
            switch (mDescriptorIdToUse) {
            case DescriptorID::pitch:
                param.setParamMaxFreqPitch(value);
                break;
            case DescriptorID::centroid:
                param.setParamMaxFreqCentroid(value);
                break;
            case DescriptorID::loudness:
            case DescriptorID::spread:
            case DescriptorID::noise:
            case DescriptorID::iterationsSpeed:
            case DescriptorID::invalid:
            default:
                break;
            }
        }
    };

    mDescriptorThresholdSlider.onValueChange = [this] {
        if (mParameterToShow) {
            auto & param = mParameterToShow->get();
            auto value = mDescriptorThresholdSlider.getValue();
            param.setParamThreshold(value);
        }
    };

    mDescriptorMinTimeSlider.onValueChange = [this] {
        if (mParameterToShow) {
            auto & param = mParameterToShow->get();
            auto value = mDescriptorMinTimeSlider.getValue();
            param.setParamMinTime(value);
        }
    };

    mDescriptorMaxTimeSlider.onValueChange = [this] {
        if (mParameterToShow) {
            auto & param = mParameterToShow->get();
            auto value = mDescriptorMaxTimeSlider.getValue();
            param.setParamMaxTime(value);
        }
    };

    addAndMakeVisible(&mDescriptorFactorSlider);
    addAndMakeVisible(&mDescriptorSmoothSlider);
    addAndMakeVisible(&mDescriptorSmoothCoefSlider);
    addAndMakeVisible(&mDescriptorMinFreqSlider);
    addAndMakeVisible(&mDescriptorMaxFreqSlider);
    addAndMakeVisible(&mDescriptorThresholdSlider);
    addAndMakeVisible(&mDescriptorMinTimeSlider);
    addAndMakeVisible(&mDescriptorMaxTimeSlider);

    addAndMakeVisible(&mDescriptorMetricLabel);
    addAndMakeVisible(&mDescriptorFactorLabel);
    addAndMakeVisible(&mDescriptorThresholdLabel);
    addAndMakeVisible(&mDescriptorMinFreqLabel);
    addAndMakeVisible(&mDescriptorMaxFreqLabel);
    addAndMakeVisible(&mDescriptorMinTimeLabel);
    addAndMakeVisible(&mDescriptorMaxTimeLabel);
    addAndMakeVisible(&mDescriptorSmoothLabel);
    addAndMakeVisible(&mDescriptorSmoothCoefLabel);

    addAndMakeVisible(&mDescriptorMetricCombo);
    mDescriptorMetricCombo.addItemList(ONSET_DETECTION_METRIC_TYPES, 1);
    mDescriptorMetricCombo.setSelectedId(1);
    mDescriptorMetricCombo.onChange = [this] {
        if (mParameterToShow) {
            auto& param = mParameterToShow->get();
            param.setParamMetricComboboxIndex(mDescriptorMetricCombo.getSelectedId());
            mAudioProcessor.setOnsetDetectionMetric(param.getParameterID(), mDescriptorMetricCombo.getSelectedId());
        }
    };

    addAndMakeVisible(&mClickTimerButton);
    mClickTimerButton.setButtonText("click me!");
    mClickTimerButton.onClick = [this] {
        if (mParameterToShow) {
            auto & param = mParameterToShow->get();
            auto counter{ 0 };
            switch (param.getParameterID()) {
            case ParameterID::azimuth:
                counter = mOnsetDetectiontimerCounterAzimuth;
                mOnsetDetectiontimerCounterAzimuth = 0;
                startTimer(timerParamID::azimuth, 5);
                break;
            case ParameterID::elevation:
                counter = mOnsetDetectiontimerCounterElevation;
                mOnsetDetectiontimerCounterElevation = 0;
                startTimer(timerParamID::elevation, 5);
                break;
            case ParameterID::x:
                counter = mOnsetDetectiontimerCounterX;
                mOnsetDetectiontimerCounterX = 0;
                startTimer(timerParamID::x, 5);
                break;
            case ParameterID::y:
                counter = mOnsetDetectiontimerCounterY;
                mOnsetDetectiontimerCounterY = 0;
                startTimer(timerParamID::y, 5);
                break;
            case ParameterID::z:
                counter = mOnsetDetectiontimerCounterZ;
                mOnsetDetectiontimerCounterZ = 0;
                startTimer(timerParamID::z, 5);
                break;
            case ParameterID::azimuthspan:
                counter = mOnsetDetectiontimerCounterAzimuthSpan;
                mOnsetDetectiontimerCounterAzimuthSpan = 0;
                startTimer(timerParamID::azimuthSpan, 5);
                break;
            case ParameterID::elevationspan:
                counter = mOnsetDetectiontimerCounterElevationSpan;
                mOnsetDetectiontimerCounterElevationSpan = 0;
                startTimer(timerParamID::elevationSpan, 5);
                break;
            case ParameterID::invalid:
            default:
                break;
            }
            mAudioProcessor.setOnsetDetectionFromClick(param.getParameterID(), static_cast<double>(counter));
        }
    };

    addAndMakeVisible(&mDataGraph);

    setSpatMode(mAudioProcessor.getSpatMode());
}

//==============================================================================
void gris::SectionSoundReactiveSpatialization::paint(juce::Graphics & g)
{
    g.fillAll(mGrisLookAndFeel.findColour(juce::ResizableWindow::backgroundColourId));

    //g.setColour(mGrisLookAndFeel.getDarkColor());
    g.setColour(juce::Colour(100, 100, 100));
    //g.drawLine(juce::Line<float>(0.0f, 20.0f, (float)getWidth(), 20.0f));
    g.drawHorizontalLine(20, 0.0f, static_cast<float>(getWidth()));
    g.drawVerticalLine(350, 0.0f, static_cast<float>(getHeight()));

    // linked parameters
    if (mSpatMode == SpatMode::cube) {
        if (mXYParamLinked) {
            g.setColour(juce::Colours::orange);
            g.drawLine(juce::Line<float>(5.0f, 42.0f, 17.0f, 42.0f), 2.0f);
            g.drawLine(juce::Line<float>(5.0f, 62.0f, 17.0f, 62.0f), 2.0f);
            g.drawLine(juce::Line<float>(5.0f, 42.0f, 5.0f, 62.0f), 2.0f);
        } else {
            g.setColour(juce::Colours::black);
            float dashLengths[4] = { 2.0f, 2.0f, 2.0f, 2.0f };
            g.drawDashedLine(juce::Line<float>(5.0f, 42.0f, 17.0f, 42.0f), dashLengths, 4, 2.0f);
            g.drawDashedLine(juce::Line<float>(5.0f, 62.0f, 17.0f, 62.0f), dashLengths, 4, 2.0f);
            g.drawDashedLine(juce::Line<float>(6.0f, 41.0f, 6.0f, 61.0f), dashLengths, 4, 2.0f);
        }
    }
}

//==============================================================================
void gris::SectionSoundReactiveSpatialization::resized()
{
    auto area{ getLocalBounds() };
    auto bannerArea{ area.removeFromTop(20) };
    auto bannerSpatParam{ bannerArea.removeFromLeft(350) };
    auto bannerAudioAnalysis{ bannerArea };
    auto areaSpatParams{ area.removeFromLeft(350) };
    mAreaAudioAnalysis = area; // we're using it in the audio analysis ui section

    mSpatialParameterLabel.setBounds(5, 3, 140, 15);
    mAudioAnalysisLabel.setBounds(bannerAudioAnalysis.getTopLeft().getX() + 5, 3, 140, 15);

    mParameterYButton.setEnabled(true);
    mParameterYDescriptorCombo.setEnabled(true);
    mParameterYRangeSlider.setEnabled(true);

    if (mSpatMode == SpatMode::dome) {
        mParameterXButton.setVisible(false);
        mParameterYButton.setVisible(false);
        mParameterZButton.setVisible(false);
        mParameterAzimuthButton.setVisible(true);
        mParameterElevationButton.setVisible(true);
        mParameterAzimuthDescriptorCombo.setVisible(true);
        mParameterElevationDescriptorCombo.setVisible(true);
        mParameterXDescriptorCombo.setVisible(false);
        mParameterYDescriptorCombo.setVisible(false);
        mParameterZDescriptorCombo.setVisible(false);
        mParameterAzimuthRangeSlider.setVisible(true);
        mParameterElevationRangeSlider.setVisible(true);
        mParameterXRangeSlider.setVisible(false);
        mParameterYRangeSlider.setVisible(false);
        mParameterZRangeSlider.setVisible(false);
        mParameterLapLabel.setVisible(true);
        mParameterLapCombo.setVisible(true);

        mParameterAzimuthButton.setBounds(areaSpatParams.getTopLeft().getX() + 17,
                                          areaSpatParams.getTopLeft().getY() + 15,
                                          80,
                                          15);
        mParameterElevationButton.setBounds(mParameterAzimuthButton.getBounds().getTopLeft().getX(),
                                            mParameterAzimuthButton.getBounds().getBottom() + 5,
                                            80,
                                            15);
        mParameterAzimuthOrXYSpanButton.setBounds(mParameterAzimuthButton.getBounds().getTopLeft().getX(),
                                                  mParameterElevationButton.getBounds().getBottom() + 5,
                                                  80,
                                                  15);
        mParameterElevationOrZSpanButton.setBounds(mParameterAzimuthButton.getBounds().getTopLeft().getX(),
                                                   mParameterAzimuthOrXYSpanButton.getBounds().getBottom() + 5,
                                                   80,
                                                   15);

        mParameterAzimuthDescriptorCombo.setBounds(mParameterAzimuthButton.getRight() + 5,
                                         areaSpatParams.getTopLeft().getY() + 15,
                                         100,
                                         15);
        mParameterElevationDescriptorCombo.setBounds(mParameterAzimuthButton.getRight() + 5,
                                           mParameterAzimuthDescriptorCombo.getBounds().getBottom() + 5,
                                           100,
                                           15);
        mParameterAzimuthOrXYSpanDescriptorCombo.setBounds(mParameterAzimuthButton.getRight() + 5,
                                                 mParameterElevationDescriptorCombo.getBounds().getBottom() + 5,
                                                 100,
                                                 15);
        mParameterElevationOrZSpanDescriptorCombo.setBounds(mParameterAzimuthButton.getRight() + 5,
                                                  mParameterAzimuthOrXYSpanDescriptorCombo.getBounds().getBottom() + 5,
                                                  100,
                                                  15);

        mParameterRangeLabel.setBounds(mParameterAzimuthDescriptorCombo.getRight() + 3,
                                              areaSpatParams.getTopLeft().getY() + 2,
                                              40,
                                              15);

        mParameterAzimuthRangeSlider.setBounds(mParameterAzimuthDescriptorCombo.getRight() + 5,
                                               areaSpatParams.getTopLeft().getY() + 17,
                                               35,
                                               12);
        mParameterElevationRangeSlider.setBounds(mParameterAzimuthDescriptorCombo.getRight() + 5,
                                                 mParameterAzimuthRangeSlider.getBounds().getBottomLeft().getY() + 8,
                                                 35,
                                                 12);
        mParameterAzimuthOrXYSpanRangeSlider.setBounds(mParameterAzimuthDescriptorCombo.getRight() + 5,
                                                       mParameterElevationRangeSlider.getBounds().getBottomLeft().getY()
                                                           + 8,
                                                       35,
                                                       12);
        mParameterElevationOrZSpanRangeSlider.setBounds(
            mParameterAzimuthDescriptorCombo.getRight() + 5,
            mParameterAzimuthOrXYSpanRangeSlider.getBounds().getBottomLeft().getY() + 8,
            35,
            12);

        mParameterOffsetLabel.setBounds(mParameterAzimuthRangeSlider.getRight() + 3,
                                        areaSpatParams.getTopLeft().getY() + 2,
                                        40,
                                        15);

        mParameterElevationZOffsetSlider.setBounds(mParameterAzimuthRangeSlider.getRight() + 5,
                                                   mParameterElevationRangeSlider.getBounds().getTopLeft().getY(),
                                                   35,
                                                   12);
        mParameterEleZSpanOffsetSlider.setBounds(mParameterAzimuthRangeSlider.getRight() + 5,
                                                 mParameterElevationOrZSpanRangeSlider.getBounds().getTopLeft().getY(),
                                                 35,
                                                 12);

        mParameterLapLabel.setBounds(mParameterOffsetLabel.getRight() + 7,
                                     areaSpatParams.getTopLeft().getY() + 2,
                                     40,
                                     15);

        mParameterLapCombo.setBounds(mParameterOffsetLabel.getRight() + 5,
                                     mParameterAzimuthDescriptorCombo.getBounds().getTopLeft().getY(),
                                     30,
                                     15);
    } else {
        mParameterXButton.setVisible(true);
        mParameterYButton.setVisible(true);
        mParameterZButton.setVisible(true);
        mParameterAzimuthButton.setVisible(false);
        mParameterElevationButton.setVisible(false);
        mParameterAzimuthDescriptorCombo.setVisible(false);
        mParameterElevationDescriptorCombo.setVisible(false);
        mParameterXDescriptorCombo.setVisible(true);
        mParameterYDescriptorCombo.setVisible(true);
        mParameterZDescriptorCombo.setVisible(true);
        mParameterAzimuthRangeSlider.setVisible(false);
        mParameterElevationRangeSlider.setVisible(false);
        mParameterXRangeSlider.setVisible(true);
        mParameterYRangeSlider.setVisible(true);
        mParameterZRangeSlider.setVisible(true);
        mParameterLapLabel.setVisible(false);
        mParameterLapCombo.setVisible(false);

        mParameterXButton.setBounds(areaSpatParams.getTopLeft().getX() + 17,
                                    areaSpatParams.getTopLeft().getY() + 15,
                                    80,
                                    15);
        mParameterYButton.setBounds(mParameterXButton.getBounds().getTopLeft().getX(),
                                    mParameterXButton.getBounds().getBottom() + 5,
                                    80,
                                    15);
        mParameterZButton.setBounds(mParameterXButton.getBounds().getTopLeft().getX(),
                                    mParameterYButton.getBounds().getBottom() + 5,
                                    80,
                                    15);
        mParameterAzimuthOrXYSpanButton.setBounds(mParameterXButton.getBounds().getTopLeft().getX(),
                                                  mParameterZButton.getBounds().getBottom() + 5,
                                                  80,
                                                  15);
        mParameterElevationOrZSpanButton.setBounds(mParameterXButton.getBounds().getTopLeft().getX(),
                                                   mParameterAzimuthOrXYSpanButton.getBounds().getBottom() + 5,
                                                   80,
                                                   15);

        mParameterXDescriptorCombo.setBounds(mParameterXButton.getRight() + 5, areaSpatParams.getTopLeft().getY() + 15, 100, 15);
        mParameterYDescriptorCombo.setBounds(mParameterXButton.getRight() + 5,
                                   mParameterXDescriptorCombo.getBounds().getBottom() + 5,
                                   100,
                                   15);
        mParameterZDescriptorCombo.setBounds(mParameterXButton.getRight() + 5,
                                   mParameterYDescriptorCombo.getBounds().getBottom() + 5,
                                   100,
                                   15);
        mParameterAzimuthOrXYSpanDescriptorCombo.setBounds(mParameterXButton.getRight() + 5,
                                                 mParameterZDescriptorCombo.getBounds().getBottom() + 5,
                                                 100,
                                                 15);
        mParameterElevationOrZSpanDescriptorCombo.setBounds(mParameterXButton.getRight() + 5,
                                                  mParameterAzimuthOrXYSpanDescriptorCombo.getBounds().getBottom() + 5,
                                                  100,
                                                  15);

        mParameterRangeLabel.setBounds(mParameterXDescriptorCombo.getRight() + 3,
                                       areaSpatParams.getTopLeft().getY() + 2,
                                       40,
                                       15);

        mParameterXRangeSlider.setBounds(mParameterXDescriptorCombo.getRight() + 5,
                                         areaSpatParams.getTopLeft().getY() + 17,
                                         35,
                                         12);
        mParameterYRangeSlider.setBounds(mParameterXDescriptorCombo.getRight() + 5,
                                         mParameterXRangeSlider.getBounds().getBottomLeft().getY() + 8,
                                         35,
                                         12);
        mParameterZRangeSlider.setBounds(mParameterXDescriptorCombo.getRight() + 5,
                                         mParameterYRangeSlider.getBounds().getBottomLeft().getY() + 8,
                                         35,
                                         12);
        mParameterAzimuthOrXYSpanRangeSlider.setBounds(mParameterXDescriptorCombo.getRight() + 5,
                                                       mParameterZRangeSlider.getBounds().getBottomLeft().getY() + 8,
                                                       35,
                                                       12);
        mParameterElevationOrZSpanRangeSlider.setBounds(
            mParameterXDescriptorCombo.getRight() + 5,
            mParameterAzimuthOrXYSpanRangeSlider.getBounds().getBottomLeft().getY() + 8,
            35,
            12);

        mParameterOffsetLabel.setBounds(mParameterXRangeSlider.getRight() + 3,
                                        areaSpatParams.getTopLeft().getY() + 2,
                                        40,
                                        15);

        mParameterElevationZOffsetSlider.setBounds(mParameterXRangeSlider.getRight() + 5,
                                                   mParameterZRangeSlider.getBounds().getTopLeft().getY(),
                                                   35,
                                                   12);
        mParameterEleZSpanOffsetSlider.setBounds(mParameterXRangeSlider.getRight() + 5,
                                                 mParameterElevationOrZSpanRangeSlider.getBounds().getTopLeft().getY(),
                                                 35,
                                                 12);

        mParameterLapLabel.setBounds(mParameterOffsetLabel.getRight() + 7,
                                     areaSpatParams.getTopLeft().getY() + 2,
                                     40,
                                     15);

        mParameterLapCombo.setBounds(mParameterOffsetLabel.getRight() + 5,
                                     mParameterXDescriptorCombo.getBounds().getTopLeft().getY(),
                                     30,
                                     15);

        if (mXYParamLinked) {
            mParameterLapLabel.setVisible(true);
            mParameterLapCombo.setVisible(true);

            mParameterYButton.setEnabled(false);
            mParameterYDescriptorCombo.setEnabled(false);
            mParameterYRangeSlider.setEnabled(false);
        }
    }
}

//==============================================================================
void gris::SectionSoundReactiveSpatialization::mouseDown(juce::MouseEvent const & event)
{
    // Area where the XYLinked line is shown.
    juce::Rectangle<float> const xyLinkedLineArea{ 1.0f, 41.0f, 15.0f, 22.0f };
    if (xyLinkedLineArea.contains(event.getMouseDownPosition().toFloat())) {
        mXYParamLinked = !mXYParamLinked;

        if (mLastUsedParameterCube) {
            auto & param = mLastUsedParameterCube->get();
            if (&param == &mParameterYButton && mParameterYButton.getToggleState()) {
                mParameterXButton.triggerClick();
            }
        }

        resized();
        repaint();
    }
}

//==============================================================================
void gris::SectionSoundReactiveSpatialization::timerCallback(int timerID)
{
    auto const checkTimer = [&](int timerID, int & counter) {
        if (counter >= 1000 * 120) {
            stopTimer(timerID);
            counter = 0;
        }
    };

    auto const updateButtonText = [&](int timerID, int & counter) {
        if (counter >= 1000 * 120 || counter == 0) {
            mClickTimerButton.setButtonText(juce::String("click me!"));
        } else {
            if (counter < 1000) {
                mClickTimerButton.setButtonText(juce::String(counter) + juce::String(" ms"));
            } else {
                auto buttonTimerValue = static_cast<double>(counter) / 1000;
                auto formattedTimerValue = juce::String(buttonTimerValue, 2);
                mClickTimerButton.setButtonText(formattedTimerValue + juce::String(" s"));
            }
        }
    };

    auto selectedTimeToShow{ 0 };

    switch (timerID) {
    case timerParamID::azimuth:
        mOnsetDetectiontimerCounterAzimuth += getTimerInterval(1);
        checkTimer(1, mOnsetDetectiontimerCounterAzimuth);
        break;
    case timerParamID::elevation:
        mOnsetDetectiontimerCounterElevation += getTimerInterval(2);
        checkTimer(2, mOnsetDetectiontimerCounterElevation);
        break;
    case timerParamID::x:
        mOnsetDetectiontimerCounterX += getTimerInterval(3);
        checkTimer(3, mOnsetDetectiontimerCounterX);
        break;
    case timerParamID::y:
        mOnsetDetectiontimerCounterY += getTimerInterval(4);
        checkTimer(4, mOnsetDetectiontimerCounterY);
        break;
    case timerParamID::z:
        mOnsetDetectiontimerCounterZ += getTimerInterval(5);
        checkTimer(5, mOnsetDetectiontimerCounterZ);
        break;
    case timerParamID::azimuthSpan:
        mOnsetDetectiontimerCounterAzimuthSpan += getTimerInterval(6);
        checkTimer(6, mOnsetDetectiontimerCounterAzimuthSpan);
        break;
    case timerParamID::elevationSpan:
        mOnsetDetectiontimerCounterElevationSpan += getTimerInterval(7);
        checkTimer(7, mOnsetDetectiontimerCounterElevationSpan);
        break;
    default:
        break;
    }

    if (mParameterToShow) {
        auto & param = mParameterToShow->get();
        switch (param.getParameterID()) {
        case ParameterID::azimuth:
            selectedTimeToShow = mOnsetDetectiontimerCounterAzimuth;
            updateButtonText(timerID, mOnsetDetectiontimerCounterAzimuth);
            break;
        case ParameterID::elevation:
            selectedTimeToShow = mOnsetDetectiontimerCounterElevation;
            updateButtonText(timerID, mOnsetDetectiontimerCounterElevation);
            break;
        case ParameterID::x:
            selectedTimeToShow = mOnsetDetectiontimerCounterX;
            updateButtonText(timerID, mOnsetDetectiontimerCounterX);
            break;
        case ParameterID::y:
            selectedTimeToShow = mOnsetDetectiontimerCounterY;
            updateButtonText(timerID, mOnsetDetectiontimerCounterY);
            break;
        case ParameterID::z:
            selectedTimeToShow = mOnsetDetectiontimerCounterZ;
            updateButtonText(timerID, mOnsetDetectiontimerCounterZ);
            break;
        case ParameterID::azimuthspan:
            selectedTimeToShow = mOnsetDetectiontimerCounterAzimuthSpan;
            updateButtonText(timerID, mOnsetDetectiontimerCounterAzimuthSpan);
            break;
        case ParameterID::elevationspan:
            selectedTimeToShow = mOnsetDetectiontimerCounterElevationSpan;
            updateButtonText(timerID, mOnsetDetectiontimerCounterElevationSpan);
            break;
        case ParameterID::invalid:
        default:
            break;
        }
    }
}

//==============================================================================
void gris::SectionSoundReactiveSpatialization::setSpatMode(SpatMode spatMode)
{
    mSpatMode = spatMode;

    unselectAllParamButtons();

    if (mSpatMode == SpatMode::cube) {
        mParameterAzimuthOrXYSpanButton.setButtonText("X-Y Span");
        mParameterElevationOrZSpanButton.setButtonText("Z Span");
        if (mLastUsedParameterCube) {
            auto & button = mLastUsedParameterCube->get();
            button.triggerClick();
        }
    } else {
        mParameterAzimuthOrXYSpanButton.setButtonText("Azimuth Span");
        mParameterElevationOrZSpanButton.setButtonText("Elevation Span");
        if (mLastUsedParameterDome) {
            auto & button = mLastUsedParameterDome->get();
            button.triggerClick();
        }
    }

    resized();
    refreshDescriptorPanel();
}

//==============================================================================
void gris::SectionSoundReactiveSpatialization::unselectAllParamButtons()
{
    mParameterAzimuthButton.setToggleState(false, juce::dontSendNotification);
    mParameterElevationButton.setToggleState(false, juce::dontSendNotification);
    mParameterXButton.setToggleState(false, juce::dontSendNotification);
    mParameterYButton.setToggleState(false, juce::dontSendNotification);
    mParameterZButton.setToggleState(false, juce::dontSendNotification);
    mParameterAzimuthOrXYSpanButton.setToggleState(false, juce::dontSendNotification);
    mParameterElevationOrZSpanButton.setToggleState(false, juce::dontSendNotification);
}

//==============================================================================
void gris::SectionSoundReactiveSpatialization::refreshDescriptorPanel()
{
    mDataGraph.setSpatialParameter(mParameterToShow);

    if (mParameterToShow) {
        switch (mDescriptorIdToUse) {
        case DescriptorID::loudness:
            if (mParameterToShow) {
                auto & param = mParameterToShow->get();
                mDescriptorFactorSlider.setValue(param.getParamFactorLoudness());
                mDescriptorSmoothSlider.setValue(param.getParamSmoothLoudness());
                mDescriptorSmoothCoefSlider.setValue(param.getParamSmoothCoefLoudness());
            }
            loudnessSpreadNoiseDescriptorLayout();
            break;
        case DescriptorID::spread:
            if (mParameterToShow) {
                auto & param = mParameterToShow->get();
                mDescriptorFactorSlider.setValue(param.getParamFactorSpread());
                mDescriptorSmoothSlider.setValue(param.getParamSmoothSpread());
                mDescriptorSmoothCoefSlider.setValue(param.getParamSmoothCoefSpread());
            }
            loudnessSpreadNoiseDescriptorLayout();
            break;
        case DescriptorID::noise:
            if (mParameterToShow) {
                auto & param = mParameterToShow->get();
                mDescriptorFactorSlider.setValue(param.getParamFactorNoise());
                mDescriptorSmoothSlider.setValue(param.getParamSmoothNoise());
                mDescriptorSmoothCoefSlider.setValue(param.getParamSmoothCoefNoise());
            }
            loudnessSpreadNoiseDescriptorLayout();
            break;
        case DescriptorID::pitch:
            if (mParameterToShow) {
                auto & param = mParameterToShow->get();
                mDescriptorMinFreqSlider.setValue(param.getParamMinFreqPitch());
                mDescriptorMaxFreqSlider.setValue(param.getParamMaxFreqPitch());
                mDescriptorSmoothSlider.setValue(param.getParamSmoothPitch());
                mDescriptorSmoothCoefSlider.setValue(param.getParamSmoothCoefPitch());
            }
            pitchCentroidDescriptorLayout();
            break;
        case DescriptorID::centroid:
            if (mParameterToShow) {
                auto & param = mParameterToShow->get();
                mDescriptorMinFreqSlider.setValue(param.getParamMinFreqCentroid());
                mDescriptorMaxFreqSlider.setValue(param.getParamMaxFreqCentroid());
                mDescriptorSmoothSlider.setValue(param.getParamSmoothCentroid());
                mDescriptorSmoothCoefSlider.setValue(param.getParamSmoothCoefCentroid());
            }
            pitchCentroidDescriptorLayout();
            break;
        case DescriptorID::iterationsSpeed:
            if (mParameterToShow) {
                auto & param = mParameterToShow->get();
                mDescriptorMetricCombo.setSelectedId(param.getParamMetricComboBoxIndex());
                mDescriptorThresholdSlider.setValue(param.getParamThreshold());
                mDescriptorMinTimeSlider.setValue(param.getParamMinTime());
                mDescriptorMaxTimeSlider.setValue(param.getParamMaxTime());
                mDescriptorSmoothSlider.setValue(param.getParamSmoothOnsetDetection());
                mDescriptorSmoothCoefSlider.setValue(param.getParamSmoothCoefOnsetDetection());
            }
            iterSpeedDescriptorLayout();
            break;
        case DescriptorID::invalid:
        default:
            setAudioAnalysisComponentsInvisible();
            break;
        }
    }
}

//==============================================================================
void gris::SectionSoundReactiveSpatialization::loudnessSpreadNoiseDescriptorLayout()
{
    setAudioAnalysisComponentsInvisible();

    mDescriptorFactorLabel.setVisible(true);
    mDescriptorSmoothLabel.setVisible(true);
    mDescriptorSmoothCoefLabel.setVisible(true);

    mDescriptorFactorSlider.setVisible(true);
    mDescriptorSmoothSlider.setVisible(true);
    mDescriptorSmoothCoefSlider.setVisible(true);

    mDataGraph.setVisible(true);

    auto area = mAreaAudioAnalysis;

    mDescriptorFactorLabel.setBounds(area.getTopLeft().getX() + 5, area.getTopLeft().getY() + 15, 75, 15);
    mDescriptorSmoothLabel.setBounds(mDescriptorFactorLabel.getBounds().getTopLeft().getX(),
                                     mDescriptorFactorLabel.getBounds().getBottom() + 5,
                                     75,
                                     15);
    mDescriptorSmoothCoefLabel.setBounds(mDescriptorSmoothLabel.getBounds().getTopLeft().getX(),
                                         mDescriptorSmoothLabel.getBounds().getBottom() + 5,
                                         75,
                                         15);
    mDescriptorFactorSlider.setBounds(mDescriptorFactorLabel.getBounds().getRight(),
                                      mDescriptorFactorLabel.getBounds().getY(),
                                      35,
                                      12);
    mDescriptorSmoothSlider.setBounds(mDescriptorSmoothLabel.getBounds().getRight(),
                                      mDescriptorSmoothLabel.getBounds().getY(),
                                      35,
                                      12);
    mDescriptorSmoothCoefSlider.setBounds(mDescriptorSmoothCoefLabel.getBounds().getRight(),
                                          mDescriptorSmoothCoefLabel.getBounds().getY(),
                                          35,
                                          12);

    mDataGraph.setBounds(mAreaAudioAnalysis.getX() + 150, mAreaAudioAnalysis.getY() + 15, 80, 80);
}

//==============================================================================
void gris::SectionSoundReactiveSpatialization::pitchCentroidDescriptorLayout()
{
    setAudioAnalysisComponentsInvisible();

    mDescriptorMinFreqLabel.setVisible(true);
    mDescriptorMaxFreqLabel.setVisible(true);
    mDescriptorSmoothLabel.setVisible(true);
    mDescriptorSmoothCoefLabel.setVisible(true);

    mDescriptorMinFreqSlider.setVisible(true);
    mDescriptorMaxFreqSlider.setVisible(true);
    mDescriptorSmoothSlider.setVisible(true);
    mDescriptorSmoothCoefSlider.setVisible(true);

    mDataGraph.setVisible(true);

    auto area = mAreaAudioAnalysis;

    mDescriptorMinFreqLabel.setBounds(area.getTopLeft().getX() + 5, area.getTopLeft().getY() + 15, 75, 15);
    mDescriptorMaxFreqLabel.setBounds(mDescriptorMinFreqLabel.getBounds().getTopLeft().getX(),
                                      mDescriptorMinFreqLabel.getBounds().getBottom() + 5,
                                      75,
                                      15);
    mDescriptorSmoothLabel.setBounds(mDescriptorMaxFreqLabel.getBounds().getTopLeft().getX(),
                                     mDescriptorMaxFreqLabel.getBounds().getBottom() + 5,
                                     75,
                                     15);
    mDescriptorSmoothCoefLabel.setBounds(mDescriptorSmoothLabel.getBounds().getTopLeft().getX(),
                                         mDescriptorSmoothLabel.getBounds().getBottom() + 5,
                                         75,
                                         15);
    mDescriptorMinFreqSlider.setBounds(mDescriptorMinFreqLabel.getBounds().getRight(),
                                       mDescriptorMinFreqLabel.getBounds().getY(),
                                       35,
                                       12);
    mDescriptorMaxFreqSlider.setBounds(mDescriptorMaxFreqLabel.getBounds().getRight(),
                                       mDescriptorMaxFreqLabel.getBounds().getY(),
                                       35,
                                       12);
    mDescriptorSmoothSlider.setBounds(mDescriptorSmoothLabel.getBounds().getRight(),
                                      mDescriptorSmoothLabel.getBounds().getY(),
                                      35,
                                      12);
    mDescriptorSmoothCoefSlider.setBounds(mDescriptorSmoothCoefLabel.getBounds().getRight(),
                                          mDescriptorSmoothCoefLabel.getBounds().getY(),
                                          35,
                                          12);

    mDataGraph.setBounds(mAreaAudioAnalysis.getX() + 150, mAreaAudioAnalysis.getY() + 15, 80, 80);
}

//==============================================================================
void gris::SectionSoundReactiveSpatialization::iterSpeedDescriptorLayout()
{
    setAudioAnalysisComponentsInvisible();

    mDescriptorMetricLabel.setVisible(true);
    mDescriptorThresholdLabel.setVisible(true);
    mDescriptorMinTimeLabel.setVisible(true);
    mDescriptorMaxTimeLabel.setVisible(true);
    mDescriptorSmoothLabel.setVisible(true);
    mDescriptorSmoothCoefLabel.setVisible(true);

    mDescriptorMetricCombo.setVisible(true);

    mDescriptorThresholdSlider.setVisible(true);
    mDescriptorMinTimeSlider.setVisible(true);
    mDescriptorMaxTimeSlider.setVisible(true);
    mDescriptorSmoothSlider.setVisible(true);
    mDescriptorSmoothCoefSlider.setVisible(true);

    mDataGraph.setVisible(true);
    mClickTimerButton.setVisible(true);


    auto area = mAreaAudioAnalysis;

    mDescriptorMetricLabel.setBounds(area.getTopLeft().getX() + 5, area.getTopLeft().getY() + 15, 75, 15);
    mDescriptorThresholdLabel.setBounds(mDescriptorMetricLabel.getBounds().getTopLeft().getX(),
                                        mDescriptorMetricLabel.getBounds().getBottom() + 5,
                                        75,
                                        15);
    mDescriptorMinTimeLabel.setBounds(mDescriptorThresholdLabel.getBounds().getTopLeft().getX(),
                                      mDescriptorThresholdLabel.getBounds().getBottom() + 5,
                                      75,
                                      15);
    mDescriptorMaxTimeLabel.setBounds(mDescriptorMinTimeLabel.getBounds().getTopLeft().getX(),
                                      mDescriptorMinTimeLabel.getBounds().getBottom() + 5,
                                      75,
                                      15);
    mDescriptorSmoothLabel.setBounds(mDescriptorMaxTimeLabel.getBounds().getTopLeft().getX(),
                                     mDescriptorMaxTimeLabel.getBounds().getBottom() + 5,
                                     75,
                                     15);
    mDescriptorSmoothCoefLabel.setBounds(mDescriptorSmoothLabel.getBounds().getTopLeft().getX(),
                                         mDescriptorSmoothLabel.getBounds().getBottom() + 5,
                                         75,
                                         15);

    mDescriptorMetricCombo.setBounds(mDescriptorMetricLabel.getRight(), area.getTopLeft().getY() + 15, 150, 15);

    mDescriptorThresholdSlider.setBounds(mDescriptorThresholdLabel.getBounds().getRight(),
                                         mDescriptorThresholdLabel.getBounds().getY(),
                                         35,
                                         12);
    mDescriptorMinTimeSlider.setBounds(mDescriptorMinTimeLabel.getBounds().getRight(),
                                       mDescriptorMinTimeLabel.getBounds().getY(),
                                       35,
                                       12);
    mDescriptorMaxTimeSlider.setBounds(mDescriptorMaxTimeLabel.getBounds().getRight(),
                                       mDescriptorMaxTimeLabel.getBounds().getY(),
                                       35,
                                       12);
    mDescriptorSmoothSlider.setBounds(mDescriptorSmoothLabel.getBounds().getRight(),
                                      mDescriptorSmoothLabel.getBounds().getY(),
                                      35,
                                      12);
    mDescriptorSmoothCoefSlider.setBounds(mDescriptorSmoothCoefLabel.getBounds().getRight(),
                                          mDescriptorSmoothCoefLabel.getBounds().getY(),
                                          35,
                                          12);

    mDataGraph.setBounds(mAreaAudioAnalysis.getX() + 150, mAreaAudioAnalysis.getY() + 35, 80, 80);
    mClickTimerButton.setBounds(mDataGraph.getBounds().getX() + 15, mDataGraph.getBottom() + 3, 50, 15);
}

//==============================================================================
void gris::SectionSoundReactiveSpatialization::setAudioAnalysisComponentsInvisible()
{
    mDescriptorMetricLabel.setVisible(false);
    mDescriptorFactorLabel.setVisible(false);
    mDescriptorThresholdLabel.setVisible(false);
    mDescriptorMinFreqLabel.setVisible(false);
    mDescriptorMaxFreqLabel.setVisible(false);
    mDescriptorMinTimeLabel.setVisible(false);
    mDescriptorMaxTimeLabel.setVisible(false);
    mDescriptorSmoothLabel.setVisible(false);
    mDescriptorSmoothCoefLabel.setVisible(false);

    mDescriptorMetricCombo.setVisible(false);

    mDescriptorFactorSlider.setVisible(false);
    mDescriptorThresholdSlider.setVisible(false);
    mDescriptorMinFreqSlider.setVisible(false);
    mDescriptorMaxFreqSlider.setVisible(false);
    mDescriptorMinTimeSlider.setVisible(false);
    mDescriptorMaxTimeSlider.setVisible(false);
    mDescriptorSmoothSlider.setVisible(false);
    mDescriptorSmoothCoefSlider.setVisible(false);

    mDataGraph.setVisible(false);
    mClickTimerButton.setVisible(false);
}

//==============================================================================
gris::DataGraph::DataGraph()
{
    mGUIBuffer.resize(100);
    std::fill(mGUIBuffer.begin(), mGUIBuffer.end(), 0.0);

    startTimer(50);
}

//==============================================================================
gris::DataGraph::~DataGraph()
{
}

//==============================================================================
void gris::DataGraph::paint(juce::Graphics & g)
{
    g.fillAll(juce::Colour(200, 200, 102));

    g.setOpacity(1.0f);
    g.setColour(juce::Colours::black);

    auto area = getLocalBounds().reduced(1);
    juce::RectangleList<float> rectList{};

    if (mParam) {
        auto & param{ mParam->get() };
        for (int i{}; i < mGUIBuffer.size(); ++i) {
            float initialX{}, initialY{}, width{}, height{};
            float valueToPaint{ static_cast<float>(mGUIBuffer.at(i)) };

            if (param.getParameterID() == ParameterID::elevation || param.getParameterID() == ParameterID::elevationspan
                || param.getParameterID() == ParameterID::z) {
                // parameter has an offset option, the graph can have negative values
                initialX = (static_cast<float>(area.getWidth()) / static_cast<float>(mGUIBuffer.size())) * (i + 1);
                width = static_cast<float>(area.getWidth()) / mGUIBuffer.size();
                height = static_cast<float>(area.getHeight() * std::abs(valueToPaint) / 2);
                if (valueToPaint < 0) {
                    // bottom half
                    initialY = static_cast<float>((area.getHeight() / 2) + 1.0f);
                    rectList.add(initialX, initialY, width, height);
                } else {
                    // top half
                    initialY = static_cast<float>((area.getHeight() / 2) - height + 1.0f);
                    rectList.add(initialX, initialY, width, height);
                }
            } else {
                // the graph uses only positive values
                initialX = (static_cast<float>(area.getWidth()) / static_cast<float>(mGUIBuffer.size())) * (i + 1);
                initialY = static_cast<float>(area.getHeight() - (area.getHeight() * std::abs(mGUIBuffer.at(i))) + 1);
                width = static_cast<float>(area.getWidth()) / mGUIBuffer.size();
                height = static_cast<float>(area.getHeight() * std::abs(mGUIBuffer.at(i)));
                rectList.add(initialX, initialY, width, height);
            }
        }
    }

    g.fillRectList(rectList);
}

//==============================================================================
void gris::DataGraph::resized()
{
}

//==============================================================================
void gris::DataGraph::timerCallback()
{
    if (isVisible()) {
        mGUIBuffer.push_back(readBufferMean());
        if (mGUIBuffer.size() > 100) {
            mGUIBuffer.pop_front();
        }
        repaint();
    }
}

//==============================================================================
void gris::DataGraph::addToBuffer(double value)
{
    mBuffer += value;
    mBufferCount++;
}

//==============================================================================
double gris::DataGraph::readBufferMean()
{
    double mean{};

    if (mBufferCount > 0) {
        mean = mBuffer / mBufferCount;
        mBuffer = 0.0;
        mBufferCount = 0;
    }

    return mean;
}

//==============================================================================
void gris::DataGraph::setSpatialParameter(std::optional<std::reference_wrapper<SpatialParameter>> param)
{
    mParam = param;
}
