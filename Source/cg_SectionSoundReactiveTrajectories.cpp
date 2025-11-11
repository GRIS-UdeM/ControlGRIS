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

#include "cg_SectionSoundReactiveTrajectories.h"

//==============================================================================
gris::SectionSoundReactiveTrajectories::SectionSoundReactiveTrajectories(GrisLookAndFeel & grisLookAndFeel,
                                                                         ControlGrisAudioProcessor & audioProcessor)
    : mGrisLookAndFeel(grisLookAndFeel)
    , mAudioProcessor(audioProcessor)
    , mAPVTS(mAudioProcessor.getValueTreeState())
    , mGainSlider(grisLookAndFeel)
    , mParameterButtonDomeRefs{ &mParameterAzimuthButton,
                                &mParameterElevationButton,
                                &mParameterAzimuthOrXYSpanButton,
                                &mParameterElevationOrZSpanButton }
    , mParameterButtonCubeRefs{ &mParameterXButton,
                                &mParameterYButton,
                                &mParameterZButton,
                                &mParameterAzimuthOrXYSpanButton,
                                &mParameterElevationOrZSpanButton }
    , mParameterAzimuthRangeSlider(grisLookAndFeel)
    , mParameterElevationRangeSlider(grisLookAndFeel)
    , mParameterXRangeSlider(grisLookAndFeel)
    , mParameterYRangeSlider(grisLookAndFeel)
    , mParameterZRangeSlider(grisLookAndFeel)
    , mParameterAzimuthOrXYSpanRangeSlider(grisLookAndFeel)
    , mParameterElevationOrZSpanRangeSlider(grisLookAndFeel)
    , mParameterAzimuthXOffsetSlider(grisLookAndFeel)
    , mParameterYOffsetSlider(grisLookAndFeel)
    , mParameterElevationZOffsetSlider(grisLookAndFeel)
    , mParameterAziXYSpanOffsetSlider(grisLookAndFeel)
    , mParameterEleZSpanOffsetSlider(grisLookAndFeel)
    , mDataGraph(grisLookAndFeel)
    , mDescriptorExpanderSlider(grisLookAndFeel)
    , mDescriptorThresholdSlider(grisLookAndFeel)
    , mDescriptorMinFreqSlider(grisLookAndFeel)
    , mDescriptorMaxFreqSlider(grisLookAndFeel)
    , mDescriptorMinTimeSlider(grisLookAndFeel)
    , mDescriptorMaxTimeSlider(grisLookAndFeel)
    , mDescriptorSmoothSlider(grisLookAndFeel)
    , mDescriptorSmoothCoefSlider(grisLookAndFeel)
{
    auto const initRangeSlider = [&](NumSlider & slider) {
        slider.setNormalisableRange(juce::NormalisableRange<double>{ -500.0, 500.0, 0.1 });
        // slider.setValue(100.0, juce::dontSendNotification);
        slider.setNumDecimalPlacesToDisplay(1);
        addAndMakeVisible(slider);
    };

    auto const initParameterDescCombo = [&](juce::ComboBox & combo) {
        combo.addItemList(AUDIO_DESCRIPTOR_TYPES, 1);
        // combo.setSelectedId(1, juce::dontSendNotification);
        addAndMakeVisible(combo);
    };

    auto const rangeSliderOnValueChange
        = [&](juce::ComboBox & descriptorCombo, NumSlider & rangeSlider, SpatialParameter & param) {
              switch (Descriptor::fromInt(descriptorCombo.getSelectedId())) {
              case DescriptorID::loudness:
                  param.setParamRangeLoudness(rangeSlider.getValue());
                  break;
              case DescriptorID::pitch:
                  param.setParamRangePitch(rangeSlider.getValue());
                  break;
              case DescriptorID::centroid:
                  param.setParamRangeCentroid(rangeSlider.getValue());
                  break;
              case DescriptorID::spread:
                  param.setParamRangeSpread(rangeSlider.getValue());
                  break;
              case DescriptorID::noise:
                  param.setParamRangeNoise(rangeSlider.getValue());
                  break;
              case DescriptorID::iterationsSpeed:
                  param.setParamRangeOnsetDetection(rangeSlider.getValue());
                  break;
              case DescriptorID::invalid:
              default:
                  break;
              }
          };

    auto const offsetSliderOnValueChange
        = [&](juce::ComboBox & descriptorCombo, NumSlider & offsetSlider, SpatialParameter & param) {
              switch (Descriptor::fromInt(descriptorCombo.getSelectedId())) {
              case DescriptorID::loudness:
                  param.setParamOffsetLoudness(offsetSlider.getValue());
                  break;
              case DescriptorID::pitch:
                  param.setParamOffsetPitch(offsetSlider.getValue());
                  break;
              case DescriptorID::centroid:
                  param.setParamOffsetCentroid(offsetSlider.getValue());
                  break;
              case DescriptorID::spread:
                  param.setParamOffsetSpread(offsetSlider.getValue());
                  break;
              case DescriptorID::noise:
                  param.setParamOffsetNoise(offsetSlider.getValue());
                  break;
              case DescriptorID::iterationsSpeed:
                  param.setParamOffsetOnsetDetection(offsetSlider.getValue());
                  break;
              case DescriptorID::invalid:
              default:
                  break;
              }
          };

    setName("Sound Reactive");

    mXYParamLinked = mAudioProcessor.getXYParamLink();

    addAndMakeVisible(&mSpatialParameterLabel);
    mSpatialParameterLabel.setText("Spatial Parameters", juce::dontSendNotification);
    addAndMakeVisible(&mAudioAnalysisLabel);
    mAudioAnalysisLabel.setText("Audio Analysis", juce::dontSendNotification);

    //==============================================================================
    // Spatial Parameters

    addAndMakeVisible(&mChannelMixLabel);
    mChannelMixLabel.setText("Ch. Mix", juce::dontSendNotification);

    addAndMakeVisible(&mChannelMixCombo);
    updateChannelMixCombo();
    mChannelMixCombo.onChange = [this] {
        auto numChannels{ mChannelMixCombo.getSelectedId() };
        mAPVTS.state.setProperty("numInputChannelsForAnalysis", numChannels, nullptr);
        mAudioProcessor.setNumChannelsForAudioAnalysis(numChannels);
    };

    addAndMakeVisible(&mGainLabel);
    mGainLabel.setText("Gain", juce::dontSendNotification);

    addAndMakeVisible(&mGainSlider);
    mGainSlider.setNumDecimalPlacesToDisplay(1);
    mGainSlider.setRange(0.0, 2.0, 0.1);
    mGainSlider.setDefaultReturnValue(1.0);
    auto gain{ mAPVTS.state.getProperty("audioGainForAnalysis") };
    if (gain.isVoid()) {
        mGainSlider.setValue(1.0);
    } else {
        mGainSlider.setValue(gain, juce::sendNotification);
    }
    mGainSlider.onValueChange = [this] {
        auto gainSliderVal{ mGainSlider.getValue() };
        mAPVTS.state.setProperty("audioGainForAnalysis", gainSliderVal, nullptr);
        mAudioProcessor.setGainMultiplierForAudioAnalysis(gainSliderVal);
    };

    // default values
    mParameterAzimuthRangeSlider.setDefaultReturnValue(100.0);
    mParameterElevationRangeSlider.setDefaultReturnValue(100.0);
    mParameterXRangeSlider.setDefaultReturnValue(100.0);
    mParameterYRangeSlider.setDefaultReturnValue(100.0);
    mParameterZRangeSlider.setDefaultReturnValue(100.0);
    mParameterAzimuthOrXYSpanRangeSlider.setDefaultReturnValue(100.0);
    mParameterElevationOrZSpanRangeSlider.setDefaultReturnValue(100.0);
    mParameterAzimuthXOffsetSlider.setDefaultReturnValue(0.0);
    mParameterYOffsetSlider.setDefaultReturnValue(0.0);
    mParameterElevationZOffsetSlider.setDefaultReturnValue(0.0);
    mParameterAziXYSpanOffsetSlider.setDefaultReturnValue(0.0);
    mParameterEleZSpanOffsetSlider.setDefaultReturnValue(0.0);

    mParameterAzimuthRangeSlider.setNumDecimalPlacesToDisplay(3);
    mParameterElevationRangeSlider.setNumDecimalPlacesToDisplay(3);
    mParameterXRangeSlider.setNumDecimalPlacesToDisplay(3);
    mParameterYRangeSlider.setNumDecimalPlacesToDisplay(3);
    mParameterZRangeSlider.setNumDecimalPlacesToDisplay(3);
    mParameterAzimuthOrXYSpanRangeSlider.setNumDecimalPlacesToDisplay(3);
    mParameterElevationOrZSpanRangeSlider.setNumDecimalPlacesToDisplay(3);
    mParameterAzimuthXOffsetSlider.setNumDecimalPlacesToDisplay(2);
    mParameterYOffsetSlider.setNumDecimalPlacesToDisplay(2);
    mParameterElevationZOffsetSlider.setNumDecimalPlacesToDisplay(2);
    mParameterAziXYSpanOffsetSlider.setNumDecimalPlacesToDisplay(2);
    mParameterEleZSpanOffsetSlider.setNumDecimalPlacesToDisplay(2);

    // range sliders
    mParameterAzimuthRangeSlider.onValueChange = [this, rangeSliderOnValueChange] {
        rangeSliderOnValueChange(mParameterAzimuthDescriptorCombo,
                                 mParameterAzimuthRangeSlider,
                                 mAudioProcessor.getAzimuthDome());
    };

    mParameterElevationRangeSlider.onValueChange = [this, rangeSliderOnValueChange] {
        rangeSliderOnValueChange(mParameterElevationDescriptorCombo,
                                 mParameterElevationRangeSlider,
                                 mAudioProcessor.getElevationDome());
    };

    mParameterXRangeSlider.onValueChange = [this, rangeSliderOnValueChange] {
        rangeSliderOnValueChange(mParameterXDescriptorCombo, mParameterXRangeSlider, mAudioProcessor.getXCube());
    };

    mParameterYRangeSlider.onValueChange = [this, rangeSliderOnValueChange] {
        rangeSliderOnValueChange(mParameterYDescriptorCombo, mParameterYRangeSlider, mAudioProcessor.getYCube());
    };

    mParameterZRangeSlider.onValueChange = [this, rangeSliderOnValueChange] {
        rangeSliderOnValueChange(mParameterZDescriptorCombo, mParameterZRangeSlider, mAudioProcessor.getZCube());
    };

    mParameterAzimuthOrXYSpanRangeSlider.onValueChange = [this, rangeSliderOnValueChange] {
        if (mSpatMode == SpatMode::dome) {
            rangeSliderOnValueChange(mParameterAzimuthOrXYSpanDescriptorCombo,
                                     mParameterAzimuthOrXYSpanRangeSlider,
                                     mAudioProcessor.getHSpanDome());
        } else {
            rangeSliderOnValueChange(mParameterAzimuthOrXYSpanDescriptorCombo,
                                     mParameterAzimuthOrXYSpanRangeSlider,
                                     mAudioProcessor.getHSpanCube());
        }
    };

    mParameterElevationOrZSpanRangeSlider.onValueChange = [this, rangeSliderOnValueChange] {
        if (mSpatMode == SpatMode::dome) {
            rangeSliderOnValueChange(mParameterElevationOrZSpanDescriptorCombo,
                                     mParameterElevationOrZSpanRangeSlider,
                                     mAudioProcessor.getVSpanDome());
        } else {
            rangeSliderOnValueChange(mParameterElevationOrZSpanDescriptorCombo,
                                     mParameterElevationOrZSpanRangeSlider,
                                     mAudioProcessor.getVSpanCube());
        }
    };

    // offset sliders
    mParameterAzimuthXOffsetSlider.onValueChange = [this, offsetSliderOnValueChange] {
        if (mSpatMode == SpatMode::dome) {
            offsetSliderOnValueChange(mParameterAzimuthDescriptorCombo,
                                      mParameterAzimuthXOffsetSlider,
                                      mAudioProcessor.getAzimuthDome());
        } else {
            offsetSliderOnValueChange(mParameterXDescriptorCombo,
                                      mParameterAzimuthXOffsetSlider,
                                      mAudioProcessor.getXCube());
        }
    };

    mParameterYOffsetSlider.onValueChange = [this, offsetSliderOnValueChange] {
        offsetSliderOnValueChange(mParameterYDescriptorCombo, mParameterYOffsetSlider, mAudioProcessor.getYCube());
    };

    mParameterElevationZOffsetSlider.onValueChange = [this, offsetSliderOnValueChange] {
        if (mSpatMode == SpatMode::dome) {
            offsetSliderOnValueChange(mParameterElevationDescriptorCombo,
                                      mParameterElevationZOffsetSlider,
                                      mAudioProcessor.getElevationDome());
        } else {
            offsetSliderOnValueChange(mParameterZDescriptorCombo,
                                      mParameterElevationZOffsetSlider,
                                      mAudioProcessor.getZCube());
        }
    };

    mParameterEleZSpanOffsetSlider.onValueChange = [this, offsetSliderOnValueChange] {
        if (mSpatMode == SpatMode::dome) {
            offsetSliderOnValueChange(mParameterElevationOrZSpanDescriptorCombo,
                                      mParameterEleZSpanOffsetSlider,
                                      mAudioProcessor.getVSpanDome());
        } else {
            offsetSliderOnValueChange(mParameterElevationOrZSpanDescriptorCombo,
                                      mParameterEleZSpanOffsetSlider,
                                      mAudioProcessor.getVSpanCube());
        }
    };

    mParameterAziXYSpanOffsetSlider.onValueChange = [this, offsetSliderOnValueChange] {
        if (mSpatMode == SpatMode::dome) {
            offsetSliderOnValueChange(mParameterAzimuthOrXYSpanDescriptorCombo,
                                      mParameterAziXYSpanOffsetSlider,
                                      mAudioProcessor.getHSpanDome());
        } else {
            offsetSliderOnValueChange(mParameterAzimuthOrXYSpanDescriptorCombo,
                                      mParameterAziXYSpanOffsetSlider,
                                      mAudioProcessor.getHSpanCube());
        }
    };

    //addAndMakeVisible(&mParameterLapLabel);
    mParameterLapLabel.setText("Lap", juce::dontSendNotification);

    // lap textEditor
    //addAndMakeVisible(&mParameterLapEditor);
    mParameterLapEditor.setLookAndFeel(&mGrisLookAndFeel);
    mParameterLapEditor.setFont(grisLookAndFeel.getFont());
    mParameterLapEditor.setInputRestrictions(2, "1234567890");
    mParameterLapEditor.setText("1");
    mParameterLapEditor.setJustification(juce::Justification::centred);
    mParameterLapEditor.onEscapeKey = [this] { mParameterLapEditor.resetCurrentText(); };
    mParameterLapEditor.onFocusLost = [this] {
        mParameterLapEditor.stopEditing();
        mParameterLapEditor.moveCaretToEnd();
        auto descriptor = DescriptorID::invalid;
        std::optional<std::reference_wrapper<SpatialParameter>> usedParam;

        if (mSpatMode == SpatMode::dome) {
            descriptor = Descriptor::fromInt(mParameterAzimuthDescriptorCombo.getSelectedId());
            usedParam = mAudioProcessor.getAzimuthDome();
        } else {
            // XYParamLinked should be true...
            descriptor = Descriptor::fromInt(mParameterXDescriptorCombo.getSelectedId());
            usedParam = mAudioProcessor.getXCube();
        }

        auto & param{ usedParam->get() };
        auto value{ mParameterLapEditor.getText().getDoubleValue() };
        if (value == 0.0) {
            value = 1.0;
            mParameterLapEditor.setText("1");
        }
        switch (descriptor) {
        case DescriptorID::loudness:
            param.setParamLapLoudness(value);
            break;
        case DescriptorID::pitch:
            param.setParamLapPitch(value);
            break;
        case DescriptorID::centroid:
            param.setParamLapCentroid(value);
            break;
        case DescriptorID::spread:
            param.setParamLapSpread(value);
            break;
        case DescriptorID::noise:
            param.setParamLapNoise(value);
            break;
        case DescriptorID::iterationsSpeed:
            param.setParamLapOnsetDetection(value);
            break;
        case DescriptorID::invalid:
        default:
            break;
        }
        unfocusAllComponents();
    };
    mParameterLapEditor.onReturnKey = [this] { mParameterLapEditor.onFocusLost(); };

    addAndMakeVisible(&mParameterAzimuthButton);
    mParameterAzimuthButton.setButtonText("Azimuth");
    mParameterAzimuthButton.setClickingTogglesState(true);
    mParameterAzimuthButton.onClick = [this] {
        if (mParameterAzimuthButton.getToggleState()) {
            mLastUsedParameterDomeButton = mParameterAzimuthButton;
            mParameterAzimuthDescriptorCombo.onChange();
            mAPVTS.state.setProperty("LastUsedParameterDomeButtonRefIdx", 0, nullptr);
        } else {
            mParameterToShow.reset();
            mDescriptorIdToUse = DescriptorID::invalid;
            mLastUsedParameterDomeButton.reset();
            setAudioAnalysisComponentsInvisible();
            mAPVTS.state.setProperty("LastUsedParameterDomeButtonRefIdx", "", nullptr);
        }
        repaint();
    };

    addAndMakeVisible(&mParameterElevationButton);
    mParameterElevationButton.setButtonText("Elevation");
    mParameterElevationButton.setClickingTogglesState(true);
    mParameterElevationButton.onClick = [this] {
        if (mParameterElevationButton.getToggleState()) {
            mLastUsedParameterDomeButton = mParameterElevationButton;
            mParameterElevationDescriptorCombo.onChange();
            mAPVTS.state.setProperty("LastUsedParameterDomeButtonRefIdx", 1, nullptr);
        } else {
            mParameterToShow.reset();
            mDescriptorIdToUse = DescriptorID::invalid;
            mLastUsedParameterDomeButton.reset();
            setAudioAnalysisComponentsInvisible();
            mAPVTS.state.setProperty("LastUsedParameterDomeButtonRefIdx", "", nullptr);
        }
        repaint();
    };

    addAndMakeVisible(&mParameterXButton);
    mParameterXButton.setButtonText("X");
    mParameterXButton.setClickingTogglesState(true);
    mParameterXButton.onClick = [this] {
        if (mParameterXButton.getToggleState()) {
            mLastUsedParameterCubeButton = mParameterXButton;
            mParameterXDescriptorCombo.onChange();
            mAPVTS.state.setProperty("LastUsedParameterCubeButtonRefIdx", 0, nullptr);
        } else {
            mParameterToShow.reset();
            mDescriptorIdToUse = DescriptorID::invalid;
            mLastUsedParameterCubeButton.reset();
            setAudioAnalysisComponentsInvisible();
            mAPVTS.state.setProperty("LastUsedParameterCubeButtonRefIdx", "", nullptr);
        }
        repaint();
    };

    addAndMakeVisible(&mParameterYButton);
    mParameterYButton.setButtonText("Y");
    mParameterYButton.setClickingTogglesState(true);
    mParameterYButton.onClick = [this] {
        if (mParameterYButton.getToggleState()) {
            mLastUsedParameterCubeButton = mParameterYButton;
            mParameterYDescriptorCombo.onChange();
            mAPVTS.state.setProperty("LastUsedParameterCubeButtonRefIdx", 1, nullptr);
        } else {
            mParameterToShow.reset();
            mDescriptorIdToUse = DescriptorID::invalid;
            mLastUsedParameterCubeButton.reset();
            setAudioAnalysisComponentsInvisible();
            mAPVTS.state.setProperty("LastUsedParameterCubeButtonRefIdx", "", nullptr);
        }
        repaint();
    };

    addAndMakeVisible(&mParameterZButton);
    mParameterZButton.setButtonText("Z");
    mParameterZButton.setClickingTogglesState(true);
    mParameterZButton.onClick = [this] {
        if (mParameterZButton.getToggleState()) {
            mLastUsedParameterCubeButton = mParameterZButton;
            mParameterZDescriptorCombo.onChange();
            mAPVTS.state.setProperty("LastUsedParameterCubeButtonRefIdx", 2, nullptr);
        } else {
            mParameterToShow.reset();
            mDescriptorIdToUse = DescriptorID::invalid;
            mLastUsedParameterCubeButton.reset();
            setAudioAnalysisComponentsInvisible();
            mAPVTS.state.setProperty("LastUsedParameterCubeButtonRefIdx", "", nullptr);
        }
        repaint();
    };

    addAndMakeVisible(&mParameterAzimuthOrXYSpanButton);
    mParameterAzimuthOrXYSpanButton.setButtonText("Azimuth Span");
    mParameterAzimuthOrXYSpanButton.setClickingTogglesState(true);
    mParameterAzimuthOrXYSpanButton.onClick = [this] {
        if (mParameterAzimuthOrXYSpanButton.getToggleState()) {
            if (mSpatMode == SpatMode::dome) {
                mLastUsedParameterDomeButton = mParameterAzimuthOrXYSpanButton;
                mAPVTS.state.setProperty("LastUsedParameterDomeButtonRefIdx", 2, nullptr);
            } else {
                mLastUsedParameterCubeButton = mParameterAzimuthOrXYSpanButton;
                mAPVTS.state.setProperty("LastUsedParameterCubeButtonRefIdx", 3, nullptr);
            }
            mParameterAzimuthOrXYSpanDescriptorCombo.onChange();
        } else {
            mParameterToShow.reset();
            mDescriptorIdToUse = DescriptorID::invalid;
            setAudioAnalysisComponentsInvisible();
            if (mSpatMode == SpatMode::dome) {
                mLastUsedParameterDomeButton.reset();
                mAPVTS.state.setProperty("LastUsedParameterDomeButtonRefIdx", "", nullptr);
            } else {
                mLastUsedParameterCubeButton.reset();
                mAPVTS.state.setProperty("LastUsedParameterCubeButtonRefIdx", "", nullptr);
            }
        }
        repaint();
    };

    addAndMakeVisible(&mParameterElevationOrZSpanButton);
    mParameterElevationOrZSpanButton.setButtonText("Elevation Span");
    mParameterElevationOrZSpanButton.setClickingTogglesState(true);
    mParameterElevationOrZSpanButton.onClick = [this] {
        if (mParameterElevationOrZSpanButton.getToggleState()) {
            if (mSpatMode == SpatMode::dome) {
                mLastUsedParameterDomeButton = mParameterElevationOrZSpanButton;
                mAPVTS.state.setProperty("LastUsedParameterDomeButtonRefIdx", 3, nullptr);
            } else {
                mLastUsedParameterCubeButton = mParameterElevationOrZSpanButton;
                mAPVTS.state.setProperty("LastUsedParameterCubeButtonRefIdx", 4, nullptr);
            }
            mParameterElevationOrZSpanDescriptorCombo.onChange();
        } else {
            mParameterToShow.reset();
            mDescriptorIdToUse = DescriptorID::invalid;
            setAudioAnalysisComponentsInvisible();
            if (mSpatMode == SpatMode::dome) {
                mLastUsedParameterDomeButton.reset();
                mAPVTS.state.setProperty("LastUsedParameterDomeButtonRefIdx", "", nullptr);
            } else {
                mLastUsedParameterCubeButton.reset();
                mAPVTS.state.setProperty("LastUsedParameterCubeButtonRefIdx", "", nullptr);
            }
        }
        repaint();
    };

    initParameterDescCombo(mParameterAzimuthDescriptorCombo);
    mParameterAzimuthDescriptorCombo.onChange = [this] {
        unselectAllParamButtons();
        if (mParameterAzimuthDescriptorCombo.getSelectedId() == 1) {
            mParameterToShow.reset();
            mDescriptorIdToUse = DescriptorID::invalid;
        } else {
            mParameterToShow = mAudioProcessor.getAzimuthDome();
            mDescriptorIdToUse = Descriptor::fromInt(mParameterAzimuthDescriptorCombo.getSelectedId());
            mLastUsedParameterDomeButton = mParameterAzimuthButton;
        }
        if (mLastUsedParameterDomeButton != std::nullopt) {
            mParameterAzimuthButton.setToggleState(true, juce::dontSendNotification);
            mLastUsedParameterDomeButton = mParameterAzimuthButton;
            mAPVTS.state.setProperty("LastUsedParameterDomeButtonRefIdx", 0, nullptr);
        }
        mAPVTS.state.setProperty("LastUsedAzimuthDescriptor",
                                 mParameterAzimuthDescriptorCombo.getSelectedId(),
                                 nullptr);
        auto & param{ mAudioProcessor.getAzimuthDome() };
        param.setDescriptorToUse(mDescriptorIdToUse);

        switch (mDescriptorIdToUse) {
        case DescriptorID::loudness:
            mParameterAzimuthRangeSlider.setValue(param.getParamRangeLoudness());
            mParameterAzimuthXOffsetSlider.setVisible(true);
            mParameterAzimuthXOffsetSlider.setValue(param.getParamOffsetLoudness());
            mParameterLapEditor.setText(juce::String(param.getParamLapLoudness()));
            break;
        case DescriptorID::pitch:
            mParameterAzimuthRangeSlider.setValue(param.getParamRangePitch());
            mParameterAzimuthXOffsetSlider.setVisible(false);
            mParameterLapEditor.setText(juce::String(param.getParamLapPitch()));
            changeMinMaxSlidersRange(20, 5000);
            break;
        case DescriptorID::centroid:
            mParameterAzimuthRangeSlider.setValue(param.getParamRangeCentroid());
            mParameterAzimuthXOffsetSlider.setVisible(false);
            mParameterLapEditor.setText(juce::String(param.getParamLapCentroid()));
            changeMinMaxSlidersRange(20, 20000);
            break;
        case DescriptorID::spread:
            mParameterAzimuthRangeSlider.setValue(param.getParamRangeSpread());
            mParameterAzimuthXOffsetSlider.setVisible(true);
            mParameterAzimuthXOffsetSlider.setValue(param.getParamOffsetSpread());
            mParameterLapEditor.setText(juce::String(param.getParamLapSpread()));
            break;
        case DescriptorID::noise:
            mParameterAzimuthRangeSlider.setValue(param.getParamRangeNoise());
            mParameterAzimuthXOffsetSlider.setVisible(true);
            mParameterAzimuthXOffsetSlider.setValue(param.getParamOffsetNoise());
            mParameterLapEditor.setText(juce::String(param.getParamLapNoise()));
            break;
        case DescriptorID::iterationsSpeed:
            mParameterAzimuthRangeSlider.setValue(param.getParamRangeOnsetDetection());
            mParameterAzimuthXOffsetSlider.setVisible(false);
            mParameterLapEditor.setText(juce::String(param.getParamLapOnsetDetection()));
            break;
        case DescriptorID::invalid:
            mParameterAzimuthXOffsetSlider.setVisible(false);
            break;
        default:
            break;
        }
        if (mParameterLapEditor.getText().getIntValue() == 0) {
            mParameterLapEditor.setText("1");
        }

        refreshDescriptorPanel();
        mAudioProcessor.setShouldProcessAudioAnalysis();
    };

    initParameterDescCombo(mParameterElevationDescriptorCombo);
    mParameterElevationDescriptorCombo.onChange = [this] {
        unselectAllParamButtons();
        if (mParameterElevationDescriptorCombo.getSelectedId() == 1) {
            mParameterToShow.reset();
            mDescriptorIdToUse = DescriptorID::invalid;
        } else {
            mParameterToShow = mAudioProcessor.getElevationDome();
            mDescriptorIdToUse = Descriptor::fromInt(mParameterElevationDescriptorCombo.getSelectedId());
            mLastUsedParameterDomeButton = mParameterElevationButton;
        }
        if (mLastUsedParameterDomeButton != std::nullopt) {
            mParameterElevationButton.setToggleState(true, juce::dontSendNotification);
            mLastUsedParameterDomeButton = mParameterElevationButton;
            mAPVTS.state.setProperty("LastUsedParameterDomeButtonRefIdx", 1, nullptr);
        }
        mAPVTS.state.setProperty("LastUsedElevationDescriptor",
                                 mParameterElevationDescriptorCombo.getSelectedId(),
                                 nullptr);
        auto & param{ mAudioProcessor.getElevationDome() };
        param.setDescriptorToUse(mDescriptorIdToUse);

        switch (mDescriptorIdToUse) {
        case DescriptorID::loudness:
            mParameterElevationRangeSlider.setValue(param.getParamRangeLoudness());
            mParameterElevationZOffsetSlider.setVisible(true);
            mParameterElevationZOffsetSlider.setValue(param.getParamOffsetLoudness());
            break;
        case DescriptorID::pitch:
            mParameterElevationRangeSlider.setValue(param.getParamRangePitch());
            mParameterElevationZOffsetSlider.setVisible(false);
            changeMinMaxSlidersRange(20, 5000);
            break;
        case DescriptorID::centroid:
            mParameterElevationRangeSlider.setValue(param.getParamRangeCentroid());
            mParameterElevationZOffsetSlider.setVisible(false);
            changeMinMaxSlidersRange(20, 20000);
            break;
        case DescriptorID::spread:
            mParameterElevationRangeSlider.setValue(param.getParamRangeSpread());
            mParameterElevationZOffsetSlider.setVisible(true);
            mParameterElevationZOffsetSlider.setValue(param.getParamOffsetSpread());
            break;
        case DescriptorID::noise:
            mParameterElevationRangeSlider.setValue(param.getParamRangeNoise());
            mParameterElevationZOffsetSlider.setVisible(true);
            mParameterElevationZOffsetSlider.setValue(param.getParamOffsetNoise());
            break;
        case DescriptorID::iterationsSpeed:
            mParameterElevationRangeSlider.setValue(param.getParamRangeOnsetDetection());
            mParameterElevationZOffsetSlider.setVisible(false);
            break;
        case DescriptorID::invalid:
            mParameterElevationZOffsetSlider.setVisible(false);
            break;
        default:
            break;
        }

        refreshDescriptorPanel();
        mAudioProcessor.setShouldProcessAudioAnalysis();
    };

    initParameterDescCombo(mParameterXDescriptorCombo);
    mParameterXDescriptorCombo.onChange = [this] {
        unselectAllParamButtons();
        if (mParameterXDescriptorCombo.getSelectedId() == 1) {
            mParameterToShow.reset();
            mDescriptorIdToUse = DescriptorID::invalid;
        } else {
            mParameterToShow = mAudioProcessor.getXCube();
            mDescriptorIdToUse = Descriptor::fromInt(mParameterXDescriptorCombo.getSelectedId());
            mLastUsedParameterCubeButton = mParameterXButton;
        }
        if (mLastUsedParameterCubeButton != std::nullopt) {
            mParameterXButton.setToggleState(true, juce::dontSendNotification);
            mLastUsedParameterCubeButton = mParameterXButton;
            mAPVTS.state.setProperty("LastUsedParameterCubeButtonRefIdx", 0, nullptr);
        }
        mAPVTS.state.setProperty("LastUsedXDescriptor", mParameterXDescriptorCombo.getSelectedId(), nullptr);
        auto & param{ mAudioProcessor.getXCube() };
        param.setDescriptorToUse(mDescriptorIdToUse);

        switch (mDescriptorIdToUse) {
        // mParameterLapCombo is visible only if mXYParamLinked is true
        case DescriptorID::loudness:
            mParameterXRangeSlider.setValue(param.getParamRangeLoudness());
            mParameterAzimuthXOffsetSlider.setVisible(true);
            mParameterAzimuthXOffsetSlider.setValue(param.getParamOffsetLoudness());
            mParameterLapEditor.setText(juce::String(param.getParamLapLoudness()));
            break;
        case DescriptorID::pitch:
            mParameterXRangeSlider.setValue(param.getParamRangePitch());
            mParameterAzimuthXOffsetSlider.setVisible(false);
            mParameterLapEditor.setText(juce::String(param.getParamLapPitch()));
            changeMinMaxSlidersRange(20, 5000);
            break;
        case DescriptorID::centroid:
            mParameterXRangeSlider.setValue(param.getParamRangeCentroid());
            mParameterAzimuthXOffsetSlider.setVisible(false);
            mParameterLapEditor.setText(juce::String(param.getParamLapCentroid()));
            changeMinMaxSlidersRange(20, 20000);
            break;
        case DescriptorID::spread:
            mParameterXRangeSlider.setValue(param.getParamRangeSpread());
            mParameterAzimuthXOffsetSlider.setVisible(true);
            mParameterAzimuthXOffsetSlider.setValue(param.getParamOffsetSpread());
            mParameterLapEditor.setText(juce::String(param.getParamLapSpread()));
            break;
        case DescriptorID::noise:
            mParameterXRangeSlider.setValue(param.getParamRangeNoise());
            mParameterAzimuthXOffsetSlider.setVisible(true);
            mParameterAzimuthXOffsetSlider.setValue(param.getParamOffsetNoise());
            mParameterLapEditor.setText(juce::String(param.getParamLapNoise()));
            break;
        case DescriptorID::iterationsSpeed:
            mParameterXRangeSlider.setValue(param.getParamRangeOnsetDetection());
            mParameterAzimuthXOffsetSlider.setVisible(false);
            mParameterLapEditor.setText(juce::String(param.getParamLapOnsetDetection()));
            break;
        case DescriptorID::invalid:
            mParameterAzimuthXOffsetSlider.setVisible(false);
            break;
        default:
            break;
        }

        refreshDescriptorPanel();
        mAudioProcessor.setShouldProcessAudioAnalysis();
    };

    initParameterDescCombo(mParameterYDescriptorCombo);
    mParameterYDescriptorCombo.onChange = [this] {
        unselectAllParamButtons();
        if (mParameterYDescriptorCombo.getSelectedId() == 1) {
            mParameterToShow.reset();
            mDescriptorIdToUse = DescriptorID::invalid;
        } else {
            mParameterToShow = mAudioProcessor.getYCube();
            mDescriptorIdToUse = Descriptor::fromInt(mParameterYDescriptorCombo.getSelectedId());
            mLastUsedParameterCubeButton = mParameterYButton;
        }
        if (mLastUsedParameterCubeButton != std::nullopt) {
            mParameterYButton.setToggleState(true, juce::dontSendNotification);
            mLastUsedParameterCubeButton = mParameterYButton;
            mAPVTS.state.setProperty("LastUsedParameterCubeButtonRefIdx", 1, nullptr);
        }
        mAPVTS.state.setProperty("LastUsedYDescriptor", mParameterYDescriptorCombo.getSelectedId(), nullptr);
        auto & param{ mAudioProcessor.getYCube() };
        param.setDescriptorToUse(mDescriptorIdToUse);

        switch (mDescriptorIdToUse) {
        case DescriptorID::loudness:
            mParameterYRangeSlider.setValue(param.getParamRangeLoudness());
            mParameterYOffsetSlider.setVisible(true);
            mParameterYOffsetSlider.setValue(param.getParamOffsetLoudness());
            break;
        case DescriptorID::pitch:
            mParameterYRangeSlider.setValue(param.getParamRangePitch());
            mParameterYOffsetSlider.setVisible(false);
            changeMinMaxSlidersRange(20, 5000);
            break;
        case DescriptorID::centroid:
            mParameterYRangeSlider.setValue(param.getParamRangeCentroid());
            mParameterYOffsetSlider.setVisible(false);
            changeMinMaxSlidersRange(20, 20000);
            break;
        case DescriptorID::spread:
            mParameterYRangeSlider.setValue(param.getParamRangeSpread());
            mParameterYOffsetSlider.setVisible(true);
            mParameterYOffsetSlider.setValue(param.getParamOffsetSpread());
            break;
        case DescriptorID::noise:
            mParameterYRangeSlider.setValue(param.getParamRangeNoise());
            mParameterYOffsetSlider.setVisible(true);
            mParameterYOffsetSlider.setValue(param.getParamOffsetNoise());
            break;
        case DescriptorID::iterationsSpeed:
            mParameterYRangeSlider.setValue(param.getParamRangeOnsetDetection());
            mParameterYOffsetSlider.setVisible(false);
            break;
        case DescriptorID::invalid:
            mParameterYOffsetSlider.setVisible(false);
            break;
        default:
            break;
        }

        refreshDescriptorPanel();
        mAudioProcessor.setShouldProcessAudioAnalysis();
    };

    initParameterDescCombo(mParameterZDescriptorCombo);
    mParameterZDescriptorCombo.onChange = [this] {
        unselectAllParamButtons();
        if (mParameterZDescriptorCombo.getSelectedId() == 1) {
            mParameterToShow.reset();
            mDescriptorIdToUse = DescriptorID::invalid;
        } else {
            mParameterToShow = mAudioProcessor.getZCube();
            mDescriptorIdToUse = Descriptor::fromInt(mParameterZDescriptorCombo.getSelectedId());
            mLastUsedParameterCubeButton = mParameterZButton;
        }
        if (mLastUsedParameterCubeButton != std::nullopt) {
            mParameterZButton.setToggleState(true, juce::dontSendNotification);
            mLastUsedParameterCubeButton = mParameterZButton;
            mAPVTS.state.setProperty("LastUsedParameterCubeButtonRefIdx", 2, nullptr);
        }
        mAPVTS.state.setProperty("LastUsedZDescriptor", mParameterZDescriptorCombo.getSelectedId(), nullptr);
        auto & param{ mAudioProcessor.getZCube() };
        param.setDescriptorToUse(mDescriptorIdToUse);

        switch (mDescriptorIdToUse) {
        case DescriptorID::loudness:
            mParameterZRangeSlider.setValue(param.getParamRangeLoudness());
            mParameterElevationZOffsetSlider.setVisible(true);
            mParameterElevationZOffsetSlider.setValue(param.getParamOffsetLoudness());
            break;
        case DescriptorID::pitch:
            mParameterZRangeSlider.setValue(param.getParamRangePitch());
            mParameterElevationZOffsetSlider.setVisible(false);
            changeMinMaxSlidersRange(20, 5000);
            break;
        case DescriptorID::centroid:
            mParameterZRangeSlider.setValue(param.getParamRangeCentroid());
            mParameterElevationZOffsetSlider.setVisible(false);
            changeMinMaxSlidersRange(20, 20000);
            break;
        case DescriptorID::spread:
            mParameterZRangeSlider.setValue(param.getParamRangeSpread());
            mParameterElevationZOffsetSlider.setVisible(true);
            mParameterElevationZOffsetSlider.setValue(param.getParamOffsetSpread());
            break;
        case DescriptorID::noise:
            mParameterZRangeSlider.setValue(param.getParamRangeNoise());
            mParameterElevationZOffsetSlider.setVisible(true);
            mParameterElevationZOffsetSlider.setValue(param.getParamOffsetNoise());
            break;
        case DescriptorID::iterationsSpeed:
            mParameterZRangeSlider.setValue(param.getParamRangeOnsetDetection());
            mParameterElevationZOffsetSlider.setVisible(false);
            break;
        case DescriptorID::invalid:
            mParameterElevationZOffsetSlider.setVisible(false);
            break;
        default:
            break;
        }

        refreshDescriptorPanel();
        mAudioProcessor.setShouldProcessAudioAnalysis();
    };

    initParameterDescCombo(mParameterAzimuthOrXYSpanDescriptorCombo);
    mParameterAzimuthOrXYSpanDescriptorCombo.onChange = [this] {
        unselectAllParamButtons();
        mDescriptorIdToUse = Descriptor::fromInt(mParameterAzimuthOrXYSpanDescriptorCombo.getSelectedId());
        if (mDescriptorIdToUse == DescriptorID::invalid) {
            mParameterToShow.reset();
            mParameterAziXYSpanOffsetSlider.setVisible(false);
        } else {
            if (mSpatMode == SpatMode::dome) {
                mParameterToShow = mAudioProcessor.getHSpanDome();
                mLastUsedParameterDomeButton = mParameterAzimuthOrXYSpanButton;
            } else {
                mParameterToShow = mAudioProcessor.getHSpanCube();
                mLastUsedParameterCubeButton = mParameterAzimuthOrXYSpanButton;
            }
        }
        if (mSpatMode == SpatMode::dome) {
            if (mLastUsedParameterDomeButton != std::nullopt) {
                mParameterAzimuthOrXYSpanButton.setToggleState(true, juce::dontSendNotification);
                mLastUsedParameterDomeButton = mParameterAzimuthOrXYSpanButton;
                mAPVTS.state.setProperty("LastUsedParameterDomeButtonRefIdx", 2, nullptr);
            }
            mAPVTS.state.setProperty("LastUsedHSpanDomeDescriptor",
                                     mParameterAzimuthOrXYSpanDescriptorCombo.getSelectedId(),
                                     nullptr);
            auto & param{ mAudioProcessor.getHSpanDome() };
            param.setDescriptorToUse(mDescriptorIdToUse);
        } else {
            if (mLastUsedParameterCubeButton != std::nullopt) {
                mParameterAzimuthOrXYSpanButton.setToggleState(true, juce::dontSendNotification);
                mLastUsedParameterCubeButton = mParameterAzimuthOrXYSpanButton;
                mAPVTS.state.setProperty("LastUsedParameterCubeButtonRefIdx", 3, nullptr);
            }
            mAPVTS.state.setProperty("LastUsedHSpanCubeDescriptor",
                                     mParameterAzimuthOrXYSpanDescriptorCombo.getSelectedId(),
                                     nullptr);
            auto & param{ mAudioProcessor.getHSpanCube() };
            param.setDescriptorToUse(mDescriptorIdToUse);
        }
        if (mParameterToShow) {
            auto & param{ mParameterToShow->get() };

            switch (mDescriptorIdToUse) {
            case DescriptorID::loudness:
                mParameterAzimuthOrXYSpanRangeSlider.setValue(param.getParamRangeLoudness());
                mParameterAziXYSpanOffsetSlider.setVisible(true);
                mParameterAziXYSpanOffsetSlider.setValue(param.getParamOffsetLoudness());
                break;
            case DescriptorID::pitch:
                mParameterAzimuthOrXYSpanRangeSlider.setValue(param.getParamRangePitch());
                mParameterAziXYSpanOffsetSlider.setVisible(false);
                changeMinMaxSlidersRange(20, 5000);
                break;
            case DescriptorID::centroid:
                mParameterAzimuthOrXYSpanRangeSlider.setValue(param.getParamRangeCentroid());
                mParameterAziXYSpanOffsetSlider.setVisible(false);
                changeMinMaxSlidersRange(20, 20000);
                break;
            case DescriptorID::spread:
                mParameterAzimuthOrXYSpanRangeSlider.setValue(param.getParamRangeSpread());
                mParameterAziXYSpanOffsetSlider.setVisible(true);
                mParameterAziXYSpanOffsetSlider.setValue(param.getParamOffsetSpread());
                break;
            case DescriptorID::noise:
                mParameterAzimuthOrXYSpanRangeSlider.setValue(param.getParamRangeNoise());
                mParameterAziXYSpanOffsetSlider.setVisible(true);
                mParameterAziXYSpanOffsetSlider.setValue(param.getParamOffsetNoise());
                break;
            case DescriptorID::iterationsSpeed:
                mParameterAzimuthOrXYSpanRangeSlider.setValue(param.getParamRangeOnsetDetection());
                mParameterAziXYSpanOffsetSlider.setVisible(false);
                break;
            case DescriptorID::invalid:
                mParameterAziXYSpanOffsetSlider.setVisible(false);
                break;
            default:
                break;
            }
        }

        mAudioProcessor.setAudioAnalysisAzimuthSpanFlag(mDescriptorIdToUse != DescriptorID::invalid);
        refreshDescriptorPanel();
        mAudioProcessor.setShouldProcessAudioAnalysis();
    };

    initParameterDescCombo(mParameterElevationOrZSpanDescriptorCombo);
    mParameterElevationOrZSpanDescriptorCombo.onChange = [this] {
        unselectAllParamButtons();
        mDescriptorIdToUse = Descriptor::fromInt(mParameterElevationOrZSpanDescriptorCombo.getSelectedId());
        if (mDescriptorIdToUse == DescriptorID::invalid) {
            mParameterToShow.reset();
            mParameterEleZSpanOffsetSlider.setVisible(false);
        } else {
            if (mSpatMode == SpatMode::dome) {
                mParameterToShow = mAudioProcessor.getVSpanDome();
                mLastUsedParameterDomeButton = mParameterElevationOrZSpanButton;
            } else {
                mParameterToShow = mAudioProcessor.getVSpanCube();
                mLastUsedParameterCubeButton = mParameterElevationOrZSpanButton;
            }
        }
        if (mSpatMode == SpatMode::dome) {
            if (mLastUsedParameterDomeButton != std::nullopt) {
                mParameterElevationOrZSpanButton.setToggleState(true, juce::dontSendNotification);
                mLastUsedParameterDomeButton = mParameterElevationOrZSpanButton;
                mAPVTS.state.setProperty("LastUsedParameterDomeButtonRefIdx", 3, nullptr);
            }
            mAPVTS.state.setProperty("LastUsedVSpanDomeDescriptor",
                                     mParameterElevationOrZSpanDescriptorCombo.getSelectedId(),
                                     nullptr);
            auto & param{ mAudioProcessor.getVSpanDome() };
            param.setDescriptorToUse(mDescriptorIdToUse);
        } else {
            if (mLastUsedParameterCubeButton != std::nullopt) {
                mParameterElevationOrZSpanButton.setToggleState(true, juce::dontSendNotification);
                mLastUsedParameterCubeButton = mParameterElevationOrZSpanButton;
                mAPVTS.state.setProperty("LastUsedParameterCubeButtonRefIdx", 4, nullptr);
            }
            mAPVTS.state.setProperty("LastUsedVSpanCubeDescriptor",
                                     mParameterElevationOrZSpanDescriptorCombo.getSelectedId(),
                                     nullptr);
            auto & param{ mAudioProcessor.getVSpanCube() };
            param.setDescriptorToUse(mDescriptorIdToUse);
        }
        if (mParameterToShow) {
            auto & param{ mParameterToShow->get() };

            switch (mDescriptorIdToUse) {
            case DescriptorID::loudness:
                mParameterElevationOrZSpanRangeSlider.setValue(param.getParamRangeLoudness());
                mParameterEleZSpanOffsetSlider.setVisible(true);
                mParameterEleZSpanOffsetSlider.setValue(param.getParamOffsetLoudness());
                break;
            case DescriptorID::pitch:
                mParameterElevationOrZSpanRangeSlider.setValue(param.getParamRangePitch());
                mParameterEleZSpanOffsetSlider.setVisible(false);
                changeMinMaxSlidersRange(20, 5000);
                break;
            case DescriptorID::centroid:
                mParameterElevationOrZSpanRangeSlider.setValue(param.getParamRangeCentroid());
                mParameterEleZSpanOffsetSlider.setVisible(false);
                changeMinMaxSlidersRange(20, 20000);
                break;
            case DescriptorID::spread:
                mParameterElevationOrZSpanRangeSlider.setValue(param.getParamRangeSpread());
                mParameterEleZSpanOffsetSlider.setVisible(true);
                mParameterEleZSpanOffsetSlider.setValue(param.getParamOffsetSpread());
                break;
            case DescriptorID::noise:
                mParameterElevationOrZSpanRangeSlider.setValue(param.getParamRangeNoise());
                mParameterEleZSpanOffsetSlider.setVisible(true);
                mParameterEleZSpanOffsetSlider.setValue(param.getParamOffsetNoise());
                break;
            case DescriptorID::iterationsSpeed:
                mParameterElevationOrZSpanRangeSlider.setValue(param.getParamRangeOnsetDetection());
                mParameterEleZSpanOffsetSlider.setVisible(false);
                break;
            case DescriptorID::invalid:
                mParameterEleZSpanOffsetSlider.setVisible(false);
                break;
            default:
                break;
            }
        }

        mAudioProcessor.setAudioAnalysisElevationSpanFlag(mDescriptorIdToUse != DescriptorID::invalid);
        refreshDescriptorPanel();
        mAudioProcessor.setShouldProcessAudioAnalysis();
    };

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

    addAndMakeVisible(&mParameterAzimuthXOffsetSlider);
    mParameterAzimuthXOffsetSlider.setNormalisableRange(juce::NormalisableRange<double>{ -360.0, 360.0, 0.01 });
    mParameterAzimuthXOffsetSlider.setValue(0.0, juce::dontSendNotification);

    addAndMakeVisible(&mParameterElevationZOffsetSlider);
    mParameterElevationZOffsetSlider.setNormalisableRange(juce::NormalisableRange<double>{ -1.0, 1.0, 0.01 });
    mParameterElevationZOffsetSlider.setValue(0.0, juce::dontSendNotification);

    addAndMakeVisible(&mParameterYOffsetSlider);
    mParameterYOffsetSlider.setNormalisableRange(juce::NormalisableRange<double>{ -1.0, 1.0, 0.01 });
    mParameterYOffsetSlider.setValue(0.0, juce::dontSendNotification);

    addAndMakeVisible(&mParameterAziXYSpanOffsetSlider);
    mParameterAziXYSpanOffsetSlider.setNormalisableRange(juce::NormalisableRange<double>{ -1.0, 1.0, 0.01 });
    mParameterAziXYSpanOffsetSlider.setValue(0.0, juce::dontSendNotification);

    addAndMakeVisible(&mParameterEleZSpanOffsetSlider);
    mParameterEleZSpanOffsetSlider.setNormalisableRange(juce::NormalisableRange<double>{ -1.0, 1.0, 0.01 });
    mParameterEleZSpanOffsetSlider.setValue(0.0, juce::dontSendNotification);

    addAndMakeVisible(&mAudioAnalysisActivateButton);
    mAudioAnalysisActivateButton.setButtonText("Activate");
    mAudioAnalysisActivateButton.setClickingTogglesState(true);
    mAudioAnalysisActivateButton.onClick
        = [this] { mAudioProcessor.setAudioAnalysisState(mAudioAnalysisActivateButton.getToggleState()); };

    //==============================================================================
    // Audio Analysis

    mDescriptorMetricLabel.setText("Metric", juce::dontSendNotification);
    mDescriptorExpanderLabel.setText("Expander", juce::dontSendNotification);
    mDescriptorThresholdLabel.setText("Threshold", juce::dontSendNotification);
    mDescriptorMinFreqLabel.setText("Min. Freq", juce::dontSendNotification);
    mDescriptorMaxFreqLabel.setText("Max. Freq", juce::dontSendNotification);
    mDescriptorMinTimeLabel.setText("Min. Time", juce::dontSendNotification);
    mDescriptorMaxTimeLabel.setText("Max. Time", juce::dontSendNotification);
    mDescriptorSmoothLabel.setText("Smooth", juce::dontSendNotification);
    mDescriptorSmoothCoefLabel.setText("Smooth Coef.", juce::dontSendNotification);

    // default values
    mDescriptorExpanderSlider.setDefaultReturnValue(100.0);
    mDescriptorSmoothSlider.setDefaultReturnValue(5.0);
    mDescriptorSmoothCoefSlider.setDefaultReturnValue(0.0);
    mDescriptorThresholdSlider.setDefaultReturnValue(0.1);
    mDescriptorMinTimeSlider.setDefaultReturnValue(0.1);
    mDescriptorMaxTimeSlider.setDefaultReturnValue(10.0);

    mDescriptorExpanderSlider.setDefaultNumDecimalPlacesToDisplay(1);
    mDescriptorSmoothSlider.setDefaultNumDecimalPlacesToDisplay(2);
    mDescriptorSmoothCoefSlider.setDefaultNumDecimalPlacesToDisplay(2);
    mDescriptorMinFreqSlider.setDefaultNumDecimalPlacesToDisplay(1);
    mDescriptorMaxFreqSlider.setDefaultNumDecimalPlacesToDisplay(1);
    mDescriptorThresholdSlider.setDefaultNumDecimalPlacesToDisplay(3);
    mDescriptorMinTimeSlider.setDefaultNumDecimalPlacesToDisplay(3);
    mDescriptorMaxTimeSlider.setDefaultNumDecimalPlacesToDisplay(3);

    mDescriptorExpanderSlider.setRange(0, 500);
    mDescriptorSmoothSlider.setRange(0, 100);
    mDescriptorSmoothCoefSlider.setRange(0, 100);
    mDescriptorThresholdSlider.setRange(0.0, 1.0);
    mDescriptorMinTimeSlider.setRange(0.0, 30.0 + ALMOST_ZERO);
    mDescriptorMaxTimeSlider.setRange(ALMOST_ZERO, 30.0);

    mDescriptorExpanderSlider.onValueChange = [this] {
        if (mParameterToShow) {
            auto & param = mParameterToShow->get();
            auto value = mDescriptorExpanderSlider.getValue();
            switch (mDescriptorIdToUse) {
            case DescriptorID::loudness:
                param.setParamExpanderLoudness(value);
                break;
            case DescriptorID::spread:
                param.setParamExpanderSpread(value);
                break;
            case DescriptorID::noise:
                param.setParamExpanderNoise(value);
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
            auto minVal{ mDescriptorMinFreqSlider.getValue() };
            auto maxVal{ mDescriptorMaxFreqSlider.getValue() };
            if (maxVal <= 1000 && maxVal - minVal < MIN_FREQ_RANGE_ANALYSIS_BELOW_1000) {
                minVal = maxVal - MIN_FREQ_RANGE_ANALYSIS_BELOW_1000;
                mDescriptorMinFreqSlider.setValue(minVal);
            } else if (maxVal > 1000 && maxVal - minVal < MIN_FREQ_RANGE_ANALYSIS_ABOVE_1000) {
                minVal = maxVal - MIN_FREQ_RANGE_ANALYSIS_ABOVE_1000;
                mDescriptorMinFreqSlider.setValue(minVal);
            }
            auto & param = mParameterToShow->get();
            switch (mDescriptorIdToUse) {
            case DescriptorID::pitch:
                param.setParamMinFreqPitch(minVal);
                break;
            case DescriptorID::centroid:
                param.setParamMinFreqCentroid(minVal);
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
            auto minVal{ mDescriptorMinFreqSlider.getValue() };
            auto maxVal{ mDescriptorMaxFreqSlider.getValue() };
            if (minVal < 1000 && maxVal - minVal < MIN_FREQ_RANGE_ANALYSIS_BELOW_1000) {
                maxVal = minVal + MIN_FREQ_RANGE_ANALYSIS_BELOW_1000;
                mDescriptorMaxFreqSlider.setValue(maxVal);
            } else if (minVal >= 1000 && maxVal - minVal < MIN_FREQ_RANGE_ANALYSIS_ABOVE_1000) {
                maxVal = minVal + MIN_FREQ_RANGE_ANALYSIS_ABOVE_1000;
                mDescriptorMaxFreqSlider.setValue(maxVal);
            }
            auto & param = mParameterToShow->get();
            switch (mDescriptorIdToUse) {
            case DescriptorID::pitch:
                param.setParamMaxFreqPitch(maxVal);
                break;
            case DescriptorID::centroid:
                param.setParamMaxFreqCentroid(maxVal);
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
            auto minVal{ mDescriptorMinTimeSlider.getValue() };
            auto maxVal{ mDescriptorMaxTimeSlider.getValue() };
            if (minVal > maxVal) {
                minVal = maxVal - ALMOST_ZERO;
                mDescriptorMinTimeSlider.setValue(minVal);
            }
            auto & param = mParameterToShow->get();
            param.setParamMinTime(minVal);
            mAudioProcessor.setOnsetDetectionMinTime(param.getParameterID(), minVal);
        }
    };

    mDescriptorMaxTimeSlider.onValueChange = [this] {
        if (mParameterToShow) {
            auto minVal{ mDescriptorMinTimeSlider.getValue() };
            auto maxVal{ mDescriptorMaxTimeSlider.getValue() };
            if (maxVal < minVal) {
                maxVal = minVal + ALMOST_ZERO;
                mDescriptorMaxTimeSlider.setValue(maxVal);
            }
            auto & param = mParameterToShow->get();
            param.setParamMaxTime(maxVal);
            mAudioProcessor.setOnsetDetectionMaxTime(param.getParameterID(), maxVal);
        }
    };

    addAndMakeVisible(&mDescriptorExpanderSlider);
    addAndMakeVisible(&mDescriptorSmoothSlider);
    //addAndMakeVisible(&mDescriptorSmoothCoefSlider);
    addAndMakeVisible(&mDescriptorMinFreqSlider);
    addAndMakeVisible(&mDescriptorMaxFreqSlider);
    addAndMakeVisible(&mDescriptorThresholdSlider);
    addAndMakeVisible(&mDescriptorMinTimeSlider);
    addAndMakeVisible(&mDescriptorMaxTimeSlider);

    addAndMakeVisible(&mDescriptorMetricLabel);
    addAndMakeVisible(&mDescriptorExpanderLabel);
    addAndMakeVisible(&mDescriptorThresholdLabel);
    addAndMakeVisible(&mDescriptorMinFreqLabel);
    addAndMakeVisible(&mDescriptorMaxFreqLabel);
    addAndMakeVisible(&mDescriptorMinTimeLabel);
    addAndMakeVisible(&mDescriptorMaxTimeLabel);
    addAndMakeVisible(&mDescriptorSmoothLabel);
    //addAndMakeVisible(&mDescriptorSmoothCoefLabel);

    addAndMakeVisible(&mDescriptorMetricCombo);
    mDescriptorMetricCombo.addItemList(ONSET_DETECTION_METRIC_TYPES, 1);
    mDescriptorMetricCombo.onChange = [this] {
        if (mParameterToShow) {
            auto & param = mParameterToShow->get();
            param.setParamMetricComboboxIndex(mDescriptorMetricCombo.getSelectedId());
            mAudioProcessor.setOnsetDetectionMetric(param.getParameterID(), mDescriptorMetricCombo.getSelectedId());
        }
    };

    // update the datagraph at 60fps
    startTimer(timerParamID::datagraphUpdate, 17);

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

    auto domeButtonIdx = mAPVTS.state.getProperty("LastUsedParameterDomeButtonRefIdx");
    auto cubeButtonIdx = mAPVTS.state.getProperty("LastUsedParameterCubeButtonRefIdx");
    if (domeButtonIdx.isVoid() || domeButtonIdx == juce::String("")) {
        mLastUsedParameterDomeButton.reset();
    } else {
        mLastUsedParameterDomeButton = *mParameterButtonDomeRefs[static_cast<int>(domeButtonIdx)];
    }
    if (cubeButtonIdx.isVoid() || cubeButtonIdx == juce::String("")) {
        mLastUsedParameterCubeButton.reset();
    } else {
        mLastUsedParameterCubeButton = *mParameterButtonCubeRefs[static_cast<int>(cubeButtonIdx)];
    }

    setSpatMode(mAudioProcessor.getSpatMode());
}

//==============================================================================
void gris::SectionSoundReactiveTrajectories::paint(juce::Graphics & g)
{
    // g.setColour(mGrisLookAndFeel.getDarkColor());
    g.setColour(juce::Colour(100, 100, 100));
    // g.drawLine(juce::Line<float>(0.0f, 20.0f, (float)getWidth(), 20.0f));
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

    // draw arrow for selected parameter
    g.setColour(juce::Colours::orange);
    if (mSpatMode == SpatMode::dome) {
        if (mLastUsedParameterDomeButton) {
            auto & button = mLastUsedParameterDomeButton->get();
            g.drawArrow(juce::Line<float>(static_cast<float>(button.getBounds().getRight()),
                                          static_cast<float>(button.getBounds().getCentreY() + 1.0f),
                                          355.0f,
                                          static_cast<float>(button.getBounds().getCentreY() + 1.0f)),
                        2.0f,
                        7.0f,
                        7.0f);
        }
    } else {
        if (mLastUsedParameterCubeButton) {
            auto & button = mLastUsedParameterCubeButton->get();
            g.drawArrow(juce::Line<float>(static_cast<float>(button.getBounds().getRight()),
                                          static_cast<float>(button.getBounds().getCentreY() + 1),
                                          355.0f,
                                          static_cast<float>(button.getBounds().getCentreY() + 1)),
                        2.0f,
                        7.0f,
                        7.0f);
        }
    }
}

//==============================================================================
void gris::SectionSoundReactiveTrajectories::resized()
{
    auto area{ getLocalBounds() };
    auto bannerArea{ area.removeFromTop(20) };
    bannerArea.removeFromLeft(350);
    auto bannerAudioAnalysis{ bannerArea };
    auto areaSpatParams{ area.removeFromLeft(350) };
    mAreaAudioAnalysis = area; // we're using it in the audio analysis ui section
    auto const showAziXYSpanRangeSlider{ Descriptor::fromInt(mParameterAzimuthOrXYSpanDescriptorCombo.getSelectedId())
                                         != DescriptorID::invalid };
    auto const showEleZSpanRangeSlider{ Descriptor::fromInt(mParameterElevationOrZSpanDescriptorCombo.getSelectedId())
                                        != DescriptorID::invalid };

    mSpatialParameterLabel.setBounds(5, 3, 140, 15);
    mAudioAnalysisLabel.setBounds(bannerAudioAnalysis.getTopLeft().getX() + 5, 3, 140, 15);

    mChannelMixLabel.setBounds(mSpatialParameterLabel.getRight() + 55, 3, 50, 15);
    mChannelMixCombo.setBounds(mChannelMixLabel.getRight() - 5, 2, 35, 15);
    mGainLabel.setBounds(350 - 35 - 3 - 30, 3, 33, 15);
    mGainSlider.setBounds(350 - 35 - 3, 4, 35, 12);

    mParameterYButton.setEnabled(true);
    mParameterYDescriptorCombo.setEnabled(true);
    mParameterYRangeSlider.setEnabled(true);
    mParameterYOffsetSlider.setEnabled(true);

    mParameterAzimuthOrXYSpanRangeSlider.setVisible(showAziXYSpanRangeSlider);
    mParameterElevationOrZSpanRangeSlider.setVisible(showEleZSpanRangeSlider);

    if (mSpatMode == SpatMode::dome) {
        auto const showAziRangeSlider{ Descriptor::fromInt(mParameterAzimuthDescriptorCombo.getSelectedId())
                                       != DescriptorID::invalid };
        auto const showEleRangeSlider{ Descriptor::fromInt(mParameterElevationDescriptorCombo.getSelectedId())
                                       != DescriptorID::invalid };
        auto const showLapEd{ Descriptor::fromInt(mParameterAzimuthDescriptorCombo.getSelectedId())
                              != DescriptorID::invalid };

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
        mParameterAzimuthRangeSlider.setVisible(showAziRangeSlider);
        mParameterElevationRangeSlider.setVisible(showEleRangeSlider);
        mParameterXRangeSlider.setVisible(false);
        mParameterYRangeSlider.setVisible(false);
        mParameterZRangeSlider.setVisible(false);
        mParameterLapLabel.setVisible(true);
        mParameterLapEditor.setVisible(showLapEd);

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
                                                           mParameterElevationDescriptorCombo.getBounds().getBottom()
                                                               + 5,
                                                           100,
                                                           15);
        mParameterElevationOrZSpanDescriptorCombo.setBounds(
            mParameterAzimuthButton.getRight() + 5,
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

        mParameterAzimuthXOffsetSlider.setBounds(mParameterAzimuthRangeSlider.getRight() + 5,
                                                 mParameterAzimuthRangeSlider.getBounds().getTopLeft().getY(),
                                                 35,
                                                 12);
        mParameterElevationZOffsetSlider.setBounds(mParameterElevationRangeSlider.getRight() + 5,
                                                   mParameterElevationRangeSlider.getBounds().getTopLeft().getY(),
                                                   35,
                                                   12);
        mParameterAziXYSpanOffsetSlider.setBounds(mParameterAzimuthOrXYSpanRangeSlider.getRight() + 5,
                                                  mParameterAzimuthOrXYSpanRangeSlider.getBounds().getTopLeft().getY(),
                                                  35,
                                                  12);
        mParameterEleZSpanOffsetSlider.setBounds(mParameterElevationOrZSpanRangeSlider.getRight() + 5,
                                                 mParameterElevationOrZSpanRangeSlider.getBounds().getTopLeft().getY(),
                                                 35,
                                                 12);

        mParameterLapLabel.setBounds(mParameterOffsetLabel.getRight() + 7,
                                     areaSpatParams.getTopLeft().getY() + 2,
                                     40,
                                     15);

        mParameterLapEditor.setBounds(mParameterOffsetLabel.getRight() + 5,
                                      mParameterAzimuthDescriptorCombo.getBounds().getTopLeft().getY() + 1,
                                      30,
                                      13);

        mAudioAnalysisActivateButton.setBounds(mParameterElevationOrZSpanButton.getBounds().getBottomLeft().getX() + 70,
                                               mParameterElevationOrZSpanButton.getBounds().getBottomLeft().getY() + 17,
                                               176,
                                               20);
    } else {
        auto const showXRangeSlider{ Descriptor::fromInt(mParameterXDescriptorCombo.getSelectedId())
                                     != DescriptorID::invalid };
        auto const showYRangeSlider{ Descriptor::fromInt(mParameterYDescriptorCombo.getSelectedId())
                                     != DescriptorID::invalid };
        auto const showZRangeSlider{ Descriptor::fromInt(mParameterZDescriptorCombo.getSelectedId())
                                     != DescriptorID::invalid };

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
        mParameterXRangeSlider.setVisible(showXRangeSlider);
        mParameterYRangeSlider.setVisible(showYRangeSlider);
        mParameterZRangeSlider.setVisible(showZRangeSlider);
        mParameterLapLabel.setVisible(false);
        mParameterLapEditor.setVisible(false);

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

        mParameterXDescriptorCombo.setBounds(mParameterXButton.getRight() + 5,
                                             areaSpatParams.getTopLeft().getY() + 15,
                                             100,
                                             15);
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
        mParameterElevationOrZSpanDescriptorCombo.setBounds(
            mParameterXButton.getRight() + 5,
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

        mParameterAzimuthXOffsetSlider.setBounds(mParameterXRangeSlider.getRight() + 5,
                                                 mParameterXRangeSlider.getBounds().getTopLeft().getY(),
                                                 35,
                                                 12);
        mParameterYOffsetSlider.setBounds(mParameterYRangeSlider.getRight() + 5,
                                          mParameterYRangeSlider.getBounds().getTopLeft().getY(),
                                          35,
                                          12);
        mParameterElevationZOffsetSlider.setBounds(mParameterZRangeSlider.getRight() + 5,
                                                   mParameterZRangeSlider.getBounds().getTopLeft().getY(),
                                                   35,
                                                   12);
        mParameterAziXYSpanOffsetSlider.setBounds(mParameterAzimuthOrXYSpanRangeSlider.getRight() + 5,
                                                  mParameterAzimuthOrXYSpanRangeSlider.getBounds().getTopLeft().getY(),
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

        mParameterLapEditor.setBounds(mParameterOffsetLabel.getRight() + 5,
                                      mParameterXDescriptorCombo.getBounds().getTopLeft().getY() + 1,
                                      30,
                                      13);

        mAudioAnalysisActivateButton.setBounds(mParameterElevationOrZSpanButton.getBounds().getBottomLeft().getX() + 70,
                                               mParameterElevationOrZSpanButton.getBounds().getBottomLeft().getY() + 7,
                                               176,
                                               20);

        if (mXYParamLinked) {
            auto const showLapEd{ Descriptor::fromInt(mParameterXDescriptorCombo.getSelectedId())
                                  != DescriptorID::invalid };

            mParameterLapLabel.setVisible(true);
            mParameterLapEditor.setVisible(showLapEd);

            mParameterYButton.setEnabled(false);
            mParameterYDescriptorCombo.setEnabled(false);
            mParameterYRangeSlider.setEnabled(false);
            mParameterYOffsetSlider.setEnabled(false);
        }
    }
}

//==============================================================================
void gris::SectionSoundReactiveTrajectories::mouseDown(juce::MouseEvent const & event)
{
    // Area where the XYLinked line is shown.
    juce::Rectangle<float> const xyLinkedLineArea{ 1.0f, 41.0f, 15.0f, 22.0f };
    if (xyLinkedLineArea.contains(event.getMouseDownPosition().toFloat()) && mSpatMode == SpatMode::cube) {
        mXYParamLinked = !mXYParamLinked;
        mAudioProcessor.setXYParamLink(mXYParamLinked);

        if (mXYParamLinked) {
            mParameterAzimuthXOffsetSlider.setNormalisableRange(juce::NormalisableRange<double>{ -360.0, 360.0, 0.01 });
        } else {
            mParameterAzimuthXOffsetSlider.setNormalisableRange(juce::NormalisableRange<double>{ -1.0, 1.0, 0.01 });
            mParameterAzimuthXOffsetSlider.setValue(0.0);
        }
        mParameterAzimuthXOffsetSlider.onValueChange();

        if (mLastUsedParameterCubeButton) {
            auto & param = mLastUsedParameterCubeButton->get();
            if (&param == &mParameterYButton && mParameterYButton.getToggleState()) {
                mParameterXButton.triggerClick();
            }
        }

        resized();
        repaint();
    }
}

//==============================================================================
void gris::SectionSoundReactiveTrajectories::timerCallback(int timerID)
{
    if (timerID == timerParamID::datagraphUpdate) {
        addNewParamValueToDataGraph();
        return;
    }

    auto const checkTimer = [&](int timerID, int & counter) {
        if (counter >= 1000 * 120) {
            stopTimer(timerID);
            counter = 0;
        }
    };

    auto const updateButtonText = [&](int & counter) {
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
            updateButtonText(mOnsetDetectiontimerCounterAzimuth);
            break;
        case ParameterID::elevation:
            selectedTimeToShow = mOnsetDetectiontimerCounterElevation;
            updateButtonText(mOnsetDetectiontimerCounterElevation);
            break;
        case ParameterID::x:
            selectedTimeToShow = mOnsetDetectiontimerCounterX;
            updateButtonText(mOnsetDetectiontimerCounterX);
            break;
        case ParameterID::y:
            selectedTimeToShow = mOnsetDetectiontimerCounterY;
            updateButtonText(mOnsetDetectiontimerCounterY);
            break;
        case ParameterID::z:
            selectedTimeToShow = mOnsetDetectiontimerCounterZ;
            updateButtonText(mOnsetDetectiontimerCounterZ);
            break;
        case ParameterID::azimuthspan:
            selectedTimeToShow = mOnsetDetectiontimerCounterAzimuthSpan;
            updateButtonText(mOnsetDetectiontimerCounterAzimuthSpan);
            break;
        case ParameterID::elevationspan:
            selectedTimeToShow = mOnsetDetectiontimerCounterElevationSpan;
            updateButtonText(mOnsetDetectiontimerCounterElevationSpan);
            break;
        case ParameterID::invalid:
        default:
            break;
        }
    }
}

//==============================================================================
void gris::SectionSoundReactiveTrajectories::setSpatMode(SpatMode spatMode)
{
    auto const updateParameterCombo = [&](juce::ComboBox & combo, juce::String APVTSProperty) {
        combo.setSelectedId(mAPVTS.state.getProperty(APVTSProperty), juce::dontSendNotification);
        if (combo.getSelectedId() == 0)
            combo.setSelectedId(1, juce::dontSendNotification);
    };

    auto const updateOffsetSlider = [&](NumSlider & slider, SpatialParameter & param, DescriptorID descID) {
        switch (descID) {
        // should be only for loudness, spread and noise
        case DescriptorID::loudness:
            slider.setValue(param.getParamOffsetLoudness());
            break;
        case DescriptorID::pitch:
            slider.setValue(param.getParamOffsetPitch());
            break;
        case DescriptorID::centroid:
            slider.setValue(param.getParamOffsetCentroid());
            break;
        case DescriptorID::spread:
            slider.setValue(param.getParamOffsetSpread());
            break;
        case DescriptorID::noise:
            slider.setValue(param.getParamOffsetNoise());
            break;
        case DescriptorID::iterationsSpeed:
            slider.setValue(param.getParamOffsetOnsetDetection());
            break;
        case DescriptorID::invalid:
        default:
            break;
        }
    };

    auto const updateRangeSlider = [&](NumSlider & slider, SpatialParameter & param, DescriptorID descID) {
        switch (descID) {
        case DescriptorID::loudness:
            slider.setValue(param.getParamRangeLoudness());
            break;
        case DescriptorID::pitch:
            slider.setValue(param.getParamRangePitch());
            break;
        case DescriptorID::centroid:
            slider.setValue(param.getParamRangeCentroid());
            break;
        case DescriptorID::spread:
            slider.setValue(param.getParamRangeSpread());
            break;
        case DescriptorID::noise:
            slider.setValue(param.getParamRangeNoise());
            break;
        case DescriptorID::iterationsSpeed:
            slider.setValue(param.getParamRangeOnsetDetection());
            break;
        case DescriptorID::invalid:
        default:
            break;
        }
    };

    auto const updateLapEditor = [&](juce::TextEditor & ed, SpatialParameter & param, DescriptorID descID) {
        switch (descID) {
        case DescriptorID::loudness:
            ed.setText(juce::String(static_cast<int>(param.getParamLapLoudness())));
            break;
        case DescriptorID::pitch:
            ed.setText(juce::String(static_cast<int>(param.getParamLapPitch())));
            break;
        case DescriptorID::centroid:
            ed.setText(juce::String(static_cast<int>(param.getParamLapCentroid())));
            break;
        case DescriptorID::spread:
            ed.setText(juce::String(static_cast<int>(param.getParamLapSpread())));
            break;
        case DescriptorID::noise:
            ed.setText(juce::String(static_cast<int>(param.getParamLapNoise())));
            break;
        case DescriptorID::iterationsSpeed:
            ed.setText(juce::String(static_cast<int>(param.getParamLapOnsetDetection())));
            break;
        case DescriptorID::invalid:
        default:
            break;
        }
    };

    mSpatMode = spatMode;

    unselectAllParamButtons();

    if (mSpatMode == SpatMode::dome) {
        mParameterAzimuthOrXYSpanButton.setButtonText("Azimuth Span");
        mParameterElevationOrZSpanButton.setButtonText("Elevation Span");
        mParameterElevationZOffsetSlider.setNormalisableRange(juce::NormalisableRange<double>{ -1.0, 1.0, 0.01 });
        mParameterAzimuthXOffsetSlider.setNormalisableRange(juce::NormalisableRange<double>{ -360.0, 360.0, 0.01 });
        mParameterYOffsetSlider.setVisible(false);

        updateParameterCombo(mParameterAzimuthDescriptorCombo, "LastUsedAzimuthDescriptor");
        updateParameterCombo(mParameterElevationDescriptorCombo, "LastUsedElevationDescriptor");
        updateParameterCombo(mParameterAzimuthOrXYSpanDescriptorCombo, "LastUsedHSpanDomeDescriptor");
        updateParameterCombo(mParameterElevationOrZSpanDescriptorCombo, "LastUsedVSpanDomeDescriptor");

        // reassociate each parameter to its desctiptor
        auto domeButtonIdx = mAPVTS.state.getProperty("LastUsedParameterDomeButtonRefIdx");
        for (auto & button : mParameterButtonDomeRefs) {
            button->setToggleState(true, juce::sendNotification);
        }
        if (domeButtonIdx.isVoid() || domeButtonIdx == juce::String("")) {
            mLastUsedParameterDomeButton.reset();
            mAPVTS.state.setProperty("LastUsedParameterDomeButtonRefIdx", "", nullptr);
            unselectAllParamButtons();
        } else {
            mLastUsedParameterDomeButton = *mParameterButtonDomeRefs[static_cast<int>(domeButtonIdx)];
        }

        if (mLastUsedParameterDomeButton) {
            auto & button = mLastUsedParameterDomeButton->get();
            button.setToggleState(true, juce::sendNotification);
        }

        updateRangeSlider(mParameterAzimuthRangeSlider,
                          mAudioProcessor.getAzimuthDome(),
                          Descriptor::fromInt(mParameterAzimuthDescriptorCombo.getSelectedId()));
        updateRangeSlider(mParameterElevationRangeSlider,
                          mAudioProcessor.getElevationDome(),
                          Descriptor::fromInt(mParameterElevationDescriptorCombo.getSelectedId()));
        updateRangeSlider(mParameterAzimuthOrXYSpanRangeSlider,
                          mAudioProcessor.getHSpanDome(),
                          Descriptor::fromInt(mParameterAzimuthOrXYSpanDescriptorCombo.getSelectedId()));
        updateRangeSlider(mParameterElevationOrZSpanRangeSlider,
                          mAudioProcessor.getVSpanDome(),
                          Descriptor::fromInt(mParameterElevationOrZSpanDescriptorCombo.getSelectedId()));
        updateOffsetSlider(mParameterAzimuthXOffsetSlider,
                           mAudioProcessor.getAzimuthDome(),
                           Descriptor::fromInt(mParameterAzimuthDescriptorCombo.getSelectedId()));
        updateOffsetSlider(mParameterElevationZOffsetSlider,
                           mAudioProcessor.getElevationDome(),
                           Descriptor::fromInt(mParameterElevationDescriptorCombo.getSelectedId()));
        updateOffsetSlider(mParameterAziXYSpanOffsetSlider,
                           mAudioProcessor.getHSpanDome(),
                           Descriptor::fromInt(mParameterAzimuthOrXYSpanDescriptorCombo.getSelectedId()));
        updateOffsetSlider(mParameterEleZSpanOffsetSlider,
                           mAudioProcessor.getVSpanDome(),
                           Descriptor::fromInt(mParameterElevationOrZSpanDescriptorCombo.getSelectedId()));
        updateLapEditor(mParameterLapEditor,
                        mAudioProcessor.getAzimuthDome(),
                        Descriptor::fromInt(mParameterAzimuthDescriptorCombo.getSelectedId()));
    } else {
        mParameterAzimuthOrXYSpanButton.setButtonText("X-Y Span");
        mParameterElevationOrZSpanButton.setButtonText("Z Span");
        mParameterElevationZOffsetSlider.setNormalisableRange(juce::NormalisableRange<double>{ -1.0, 1.0, 0.01 });
        mParameterAzimuthXOffsetSlider.setNormalisableRange(juce::NormalisableRange<double>{ -360.0, 360.0, 0.01 });
        mXYParamLinked
            ? mParameterAzimuthXOffsetSlider.setNormalisableRange(
                  juce::NormalisableRange<double>{ -360.0, 360.0, 0.01 })
            : mParameterAzimuthXOffsetSlider.setNormalisableRange(juce::NormalisableRange<double>{ -1.0, 1.0, 0.01 });

        updateParameterCombo(mParameterXDescriptorCombo, "LastUsedXDescriptor");
        updateParameterCombo(mParameterYDescriptorCombo, "LastUsedYDescriptor");
        updateParameterCombo(mParameterZDescriptorCombo, "LastUsedZDescriptor");
        updateParameterCombo(mParameterAzimuthOrXYSpanDescriptorCombo, "LastUsedHSpanCubeDescriptor");
        updateParameterCombo(mParameterElevationOrZSpanDescriptorCombo, "LastUsedVSpanCubeDescriptor");

        // reassociate each parameter to its desctiptor
        auto cubeButtonIdx = mAPVTS.state.getProperty("LastUsedParameterCubeButtonRefIdx");
        for (auto & button : mParameterButtonCubeRefs) {
            button->setToggleState(true, juce::sendNotification);
        }
        if (cubeButtonIdx.isVoid() || cubeButtonIdx == juce::String("")) {
            mLastUsedParameterCubeButton.reset();
            mAPVTS.state.setProperty("LastUsedParameterCubeButtonRefIdx", "", nullptr);
            unselectAllParamButtons();
        } else {
            mLastUsedParameterCubeButton = *mParameterButtonCubeRefs[static_cast<int>(cubeButtonIdx)];
        }

        if (mLastUsedParameterCubeButton) {
            auto & button = mLastUsedParameterCubeButton->get();
            button.setToggleState(true, juce::sendNotification);
        }

        updateRangeSlider(mParameterXRangeSlider,
                          mAudioProcessor.getXCube(),
                          Descriptor::fromInt(mParameterXDescriptorCombo.getSelectedId()));
        updateRangeSlider(mParameterYRangeSlider,
                          mAudioProcessor.getYCube(),
                          Descriptor::fromInt(mParameterYDescriptorCombo.getSelectedId()));
        updateRangeSlider(mParameterZRangeSlider,
                          mAudioProcessor.getZCube(),
                          Descriptor::fromInt(mParameterZDescriptorCombo.getSelectedId()));
        updateRangeSlider(mParameterAzimuthOrXYSpanRangeSlider,
                          mAudioProcessor.getHSpanCube(),
                          Descriptor::fromInt(mParameterAzimuthOrXYSpanDescriptorCombo.getSelectedId()));
        updateRangeSlider(mParameterElevationOrZSpanRangeSlider,
                          mAudioProcessor.getVSpanCube(),
                          Descriptor::fromInt(mParameterElevationOrZSpanDescriptorCombo.getSelectedId()));
        updateOffsetSlider(mParameterAzimuthXOffsetSlider,
                           mAudioProcessor.getXCube(),
                           Descriptor::fromInt(mParameterZDescriptorCombo.getSelectedId()));
        updateOffsetSlider(mParameterYOffsetSlider,
                           mAudioProcessor.getYCube(),
                           Descriptor::fromInt(mParameterYDescriptorCombo.getSelectedId()));
        updateOffsetSlider(mParameterElevationZOffsetSlider,
                           mAudioProcessor.getZCube(),
                           Descriptor::fromInt(mParameterZDescriptorCombo.getSelectedId()));
        updateOffsetSlider(mParameterAziXYSpanOffsetSlider,
                           mAudioProcessor.getHSpanCube(),
                           Descriptor::fromInt(mParameterAzimuthOrXYSpanDescriptorCombo.getSelectedId()));
        updateOffsetSlider(mParameterEleZSpanOffsetSlider,
                           mAudioProcessor.getVSpanCube(),
                           Descriptor::fromInt(mParameterElevationOrZSpanDescriptorCombo.getSelectedId()));
        updateLapEditor(mParameterLapEditor,
                        mAudioProcessor.getXCube(),
                        Descriptor::fromInt(mParameterXDescriptorCombo.getSelectedId()));
    }

    refreshDescriptorPanel();
}

//==============================================================================
void gris::SectionSoundReactiveTrajectories::addNewParamValueToDataGraph()
{
    if (mParameterToShow) {
        auto & param{ mParameterToShow->get() };
        auto value{ param.getValue() };
        auto lap{ mParameterLapEditor.getText().getIntValue() };
        if (lap == 0) {
            lap = 1;
        }

        switch (param.getParameterID()) {
        case ParameterID::azimuth:
            value = juce::jmap(value, -360.0 * lap, 360.0 * lap, -1.0, 1.0);
            break;
        case ParameterID::elevation:
            value = juce::jmap(value, -90.0, 90.0, -1.0, 1.0);
            break;
        case ParameterID::x:
            if (mAudioProcessor.getXYParamLink()) {
                value = juce::jmap(value, -360.0 * lap, 360.0 * lap, -1.0, 1.0);
            } else {
                value *= 0.5;
            }
            break;
        case ParameterID::y:
            value *= 0.5;
            break;
        case ParameterID::azimuthspan:
            value = juce::jmap(value, -100.0, 100.0, -1.0, 1.0);
            break;
        case ParameterID::elevationspan:
            value = juce::jmap(value, -100.0, 100.0, -1.0, 1.0);
            break;
        case ParameterID::z:
            break;
        case ParameterID::invalid:
        default:
            break;
        }
        value = juce::jlimit(-1.0, 1.0, value);

        mDataGraph.addToBuffer(value);
    }
}

//==============================================================================
bool gris::SectionSoundReactiveTrajectories::getAudioAnalysisActivateState()
{
    return mAudioAnalysisActivateButton.getToggleState();
}

//==============================================================================
void gris::SectionSoundReactiveTrajectories::setAudioAnalysisActivateState(bool state)
{
    mAudioAnalysisActivateButton.setToggleState(state, juce::dontSendNotification);
}

//==============================================================================
void gris::SectionSoundReactiveTrajectories::updateChannelMixCombo()
{
    auto numInputChannels{ mAudioProcessor.getTotalNumInputChannels() };
    juce::StringArray chanArray;

    for (int i{}; i < numInputChannels; ++i) {
        chanArray.add(juce::String(i + 1));
    }
    mChannelMixCombo.clear();
    mChannelMixCombo.addItemList(chanArray, 1);

    int selectedId{ mAPVTS.state.getProperty("numInputChannelsForAnalysis") };
    if (selectedId == 0 || selectedId > numInputChannels) {
        selectedId = numInputChannels;
        mAPVTS.state.setProperty("numInputChannelsForAnalysis", selectedId, nullptr);
    }
    mChannelMixCombo.setSelectedId(selectedId, juce::dontSendNotification);
    mAudioProcessor.setNumChannelsForAudioAnalysis(selectedId);
}

//==============================================================================
void gris::SectionSoundReactiveTrajectories::unselectAllParamButtons()
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
void gris::SectionSoundReactiveTrajectories::refreshDescriptorPanel()
{
    bool shouldShowDescriptorPanel{};
    if (mSpatMode == SpatMode::dome) {
        shouldShowDescriptorPanel = mLastUsedParameterDomeButton != std::nullopt;
    } else {
        shouldShowDescriptorPanel = mLastUsedParameterCubeButton != std::nullopt;
    }

    mDataGraph.setDescriptor(mDescriptorIdToUse);

    if (shouldShowDescriptorPanel) {
        switch (mDescriptorIdToUse) {
        case DescriptorID::loudness:
            if (mParameterToShow) {
                auto & param = mParameterToShow->get();
                mDescriptorExpanderSlider.setValue(param.getParamExpanderLoudness());
                mDescriptorSmoothSlider.setValue(param.getParamSmoothLoudness());
                mDescriptorSmoothCoefSlider.setValue(param.getParamSmoothCoefLoudness());
            }
            loudnessSpreadNoiseDescriptorLayout();
            break;
        case DescriptorID::spread:
            if (mParameterToShow) {
                auto & param = mParameterToShow->get();
                mDescriptorExpanderSlider.setValue(param.getParamExpanderSpread());
                mDescriptorSmoothSlider.setValue(param.getParamSmoothSpread());
                mDescriptorSmoothCoefSlider.setValue(param.getParamSmoothCoefSpread());
            }
            loudnessSpreadNoiseDescriptorLayout();
            break;
        case DescriptorID::noise:
            if (mParameterToShow) {
                auto & param = mParameterToShow->get();
                mDescriptorExpanderSlider.setValue(param.getParamExpanderNoise());
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
    } else {
        setAudioAnalysisComponentsInvisible();
    }
    resized();
    repaint();
}

//==============================================================================
void gris::SectionSoundReactiveTrajectories::loudnessSpreadNoiseDescriptorLayout()
{
    setAudioAnalysisComponentsInvisible();

    mDescriptorExpanderLabel.setVisible(true);
    mDescriptorSmoothLabel.setVisible(true);
    mDescriptorSmoothCoefLabel.setVisible(true);

    mDescriptorExpanderSlider.setVisible(true);
    mDescriptorSmoothSlider.setVisible(true);
    mDescriptorSmoothCoefSlider.setVisible(true);

    mDataGraph.setVisible(true);

    auto area = mAreaAudioAnalysis;

    mDescriptorExpanderLabel.setBounds(area.getTopLeft().getX() + 5, area.getTopLeft().getY() + 15, 75, 15);
    mDescriptorSmoothLabel.setBounds(mDescriptorExpanderLabel.getBounds().getTopLeft().getX(),
                                     mDescriptorExpanderLabel.getBounds().getBottom() + 5,
                                     75,
                                     15);
    mDescriptorSmoothCoefLabel.setBounds(mDescriptorSmoothLabel.getBounds().getTopLeft().getX(),
                                         mDescriptorSmoothLabel.getBounds().getBottom() + 5,
                                         75,
                                         15);
    mDescriptorExpanderSlider.setBounds(mDescriptorExpanderLabel.getBounds().getRight(),
                                        mDescriptorExpanderLabel.getBounds().getY(),
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
void gris::SectionSoundReactiveTrajectories::pitchCentroidDescriptorLayout()
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
void gris::SectionSoundReactiveTrajectories::iterSpeedDescriptorLayout()
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
void gris::SectionSoundReactiveTrajectories::setAudioAnalysisComponentsInvisible()
{
    mDescriptorMetricLabel.setVisible(false);
    mDescriptorExpanderLabel.setVisible(false);
    mDescriptorThresholdLabel.setVisible(false);
    mDescriptorMinFreqLabel.setVisible(false);
    mDescriptorMaxFreqLabel.setVisible(false);
    mDescriptorMinTimeLabel.setVisible(false);
    mDescriptorMaxTimeLabel.setVisible(false);
    mDescriptorSmoothLabel.setVisible(false);
    mDescriptorSmoothCoefLabel.setVisible(false);

    mDescriptorMetricCombo.setVisible(false);

    mDescriptorExpanderSlider.setVisible(false);
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
void gris::SectionSoundReactiveTrajectories::changeMinMaxSlidersRange(int min, int max)
{
    mDescriptorMinFreqSlider.setRange(min, max);
    mDescriptorMaxFreqSlider.setRange(min, max);
    mDescriptorMinFreqSlider.setDefaultReturnValue(min);
    mDescriptorMaxFreqSlider.setDefaultReturnValue(max);
    mDescriptorMaxFreqSlider.setValue(mDescriptorMaxFreqSlider.getValue());
    mDescriptorMinFreqSlider.setValue(mDescriptorMinFreqSlider.getValue());
}

//==============================================================================
gris::DataGraph::DataGraph(GrisLookAndFeel & grisLookAndFeel) : mGrisLookAndFeel(grisLookAndFeel)
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
    g.fillAll(mGrisLookAndFeel.getWinBackgroundColor());

    g.setOpacity(1.0f);
    g.setColour(mGrisLookAndFeel.getOnColor());

    auto area = getLocalBounds().reduced(2);
    juce::RectangleList<float> rectList{};

    if (mDescId != DescriptorID::invalid) {
        for (int i{}; i < mGUIBuffer.size(); ++i) {
            float initialX{}, initialY{}, width{}, height{};
            float valueToPaint{ static_cast<float>(mGUIBuffer.at(i)) };

            if (mDescId == DescriptorID::loudness || mDescId == DescriptorID::spread
                || mDescId == DescriptorID::noise) {
                // parameter has an offset option, the graph can have negative values
                initialX
                    = ((static_cast<float>(area.getWidth()) / static_cast<float>(mGUIBuffer.size())) * i) + area.getX();
                width = static_cast<float>(area.getWidth()) / mGUIBuffer.size();
                height = static_cast<float>(area.getHeight() * std::abs(valueToPaint) / 2);
                if (valueToPaint < 0) {
                    // bottom half
                    initialY = static_cast<float>(area.getHeight() / 2) + area.getY();
                    rectList.add(initialX, initialY, width, height);
                } else {
                    // top half
                    initialY = static_cast<float>((area.getHeight() / 2) - height) + area.getY();
                    rectList.add(initialX, initialY, width, height);
                }
            } else {
                // the graph uses only positive values
                initialX
                    = (static_cast<float>(area.getWidth()) / static_cast<float>(mGUIBuffer.size()) * i) + area.getX();
                initialY = (static_cast<float>(area.getHeight() - (area.getHeight() * std::abs(mGUIBuffer.at(i)))))
                           + area.getY();
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
    mBuffer = mBuffer.get() + value;
    ++mBufferCount;
}

//==============================================================================
double gris::DataGraph::readBufferMean()
{
    // this is called by the timer thread.
    double mean{};

    if (mBufferCount.get() > 0) {
        mean = mBuffer.get() / mBufferCount.get();
        mBuffer = 0.0;
        mBufferCount = 0;
    }

    return mean;
}

//==============================================================================
void gris::DataGraph::setDescriptor(DescriptorID descId)
{
    mDescId = descId;
}
