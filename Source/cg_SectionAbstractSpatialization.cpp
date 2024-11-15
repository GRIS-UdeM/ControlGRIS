/**************************************************************************
 * Copyright 2018 UdeM - GRIS - Olivier Belanger                          *
 *                                                                        *
 * This file is part of ControlGris, a multi-source spatialization plugin *
 *                                                                        *
 * ControlGris is free software: you can redistribute it and/or modify    *
 * it under the terms of the GNU Lesser General Public License as         *
 * published by the Free Software Foundation, either version 3 of the     *
 * License, or (at your option) any later version.                        *
 *                                                                        *
 * ControlGris is distributed in the hope that it will be useful,         *
 * but WITHOUT ANY WARRANTY; without even the implied warranty of         *
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the          *
 * GNU Lesser General Public License for more details.                    *
 *                                                                        *
 * You should have received a copy of the GNU Lesser General Public       *
 * License along with ControlGris.  If not, see                           *
 * <http://www.gnu.org/licenses/>.                                        *
 *************************************************************************/

#include "cg_SectionAbstractSpatialization.hpp"

#include "cg_constants.hpp"

namespace gris
{
namespace
{
auto constexpr SPEED_SLIDER_MIN_VAL{ 0.0 };
auto constexpr SPEED_SLIDER_MAX_VAL{ 10.0 };
auto constexpr SPEED_SLIDER_MID_VAL{ 1.0 };
} // namespace

//==============================================================================
SectionAbstractSpatialization::SectionAbstractSpatialization(GrisLookAndFeel & grisLookAndFeel,
                                                             juce::AudioProcessorValueTreeState & apvts)
    : mGrisLookAndFeel(grisLookAndFeel)
    , mAPVTS(apvts)
    , mRandomProximityXYSlider(grisLookAndFeel)
    , mRandomTimeMinXYSlider(grisLookAndFeel)
    , mRandomTimeMaxXYSlider(grisLookAndFeel)
    , mRandomProximityZSlider(grisLookAndFeel)
    , mRandomTimeMinZSlider(grisLookAndFeel)
    , mRandomTimeMaxZSlider(grisLookAndFeel)
{
    setName("SectionAbstractSpatialization");

    mSpatMode = SpatMode::dome;

    mTrajectoryTypeLabel.setText("Trajectory Type:", juce::NotificationType::dontSendNotification);
    addAndMakeVisible(&mTrajectoryTypeLabel);

    mTrajectoryTypeXYLabel.setText("X-Y", juce::NotificationType::dontSendNotification);
    addAndMakeVisible(&mTrajectoryTypeXYLabel);

    mTrajectoryTypeZLabel.setText("Z", juce::NotificationType::dontSendNotification);
    addAndMakeVisible(&mTrajectoryTypeZLabel);

    mPositionTrajectoryTypeCombo.addItemList(POSITION_TRAJECTORY_TYPE_TYPES, 1);
    mPositionTrajectoryTypeCombo.setSelectedId(1);
    addAndMakeVisible(&mPositionTrajectoryTypeCombo);
    mPositionTrajectoryTypeCombo.onChange = [this] {
        mListeners.call([&](Listener & l) {
            l.positionTrajectoryTypeChangedCallback(
                static_cast<PositionTrajectoryType>(mPositionTrajectoryTypeCombo.getSelectedId()));
        });
    };

    mElevationTrajectoryTypeCombo.addItemList(ELEVATION_TRAJECTORY_TYPE_TYPES, 1);
    mElevationTrajectoryTypeCombo.setSelectedId(1);
    addChildComponent(&mElevationTrajectoryTypeCombo);
    mElevationTrajectoryTypeCombo.onChange = [this] {
        mListeners.call([&](Listener & l) {
            l.elevationTrajectoryTypeChangedCallback(
                static_cast<ElevationTrajectoryType>(mElevationTrajectoryTypeCombo.getSelectedId()));
        });
    };

    mDurationLabel.setText("Dur per cycle:", juce::NotificationType::dontSendNotification);
    addAndMakeVisible(&mDurationLabel);

    addAndMakeVisible(&mDurationEditor);
    mDurationEditor.setFont(mGrisLookAndFeel.getFont());
    mDurationEditor.setTextToShowWhenEmpty("1", juce::Colours::white);
    mDurationEditor.setText("5", false);
    mDurationEditor.setInputRestrictions(10, "0123456789.");
    mDurationEditor.onReturnKey = [this] { mDurationUnitCombo.grabKeyboardFocus(); };
    mDurationEditor.onFocusLost = [this] {
        mListeners.call([&](Listener & l) {
            l.trajectoryCycleDurationChangedCallback(mDurationEditor.getText().getDoubleValue(),
                                                     mDurationUnitCombo.getSelectedId());
        });
        mDurationUnitCombo.grabKeyboardFocus();
    };

    addAndMakeVisible(&mDurationUnitCombo);
    mDurationUnitCombo.addItem("Sec(s)", 1);
    mDurationUnitCombo.addItem("Beat(s)", 2);
    mDurationUnitCombo.setSelectedId(1);
    mDurationUnitCombo.onChange = [this] {
        mListeners.call([&](Listener & l) {
            l.trajectoryDurationUnitChangedCallback(mDurationEditor.getText().getDoubleValue(),
                                                    mDurationUnitCombo.getSelectedId());
        });
    };

    mPositionCycleSpeedSlider.setNormalisableRange(juce::NormalisableRange<double>(0.0, 1.0, 0.01));
    mPositionCycleSpeedSlider.setDoubleClickReturnValue(true, 0.5);
    auto posCycleSpeed{ mAPVTS.state.getProperty("posCycleSpeed") };
    if (posCycleSpeed.isVoid()) {
        posCycleSpeed = 0.5;
    }
    mPositionCycleSpeedSlider.setValue(posCycleSpeed);
    mPositionCycleSpeedSlider.setSliderSnapsToMousePosition(false);
    mPositionCycleSpeedSlider.setTextBoxStyle(juce::Slider::NoTextBox, false, 40, 20);
    mPositionCycleSpeedSlider.setColour(juce::Slider::textBoxOutlineColourId, juce::Colours::transparentBlack);
    addAndMakeVisible(&mPositionCycleSpeedSlider);
    mPositionCycleSpeedSlider.onValueChange = [this] {
        auto const sliderVal{ mPositionCycleSpeedSlider.getValue() };
        mAPVTS.state.setProperty("posCycleSpeed", sliderVal, nullptr);
        double speedMultToSend{};
        if (mSpeedLinked) {
            mElevationCycleSpeedSlider.setValue(sliderVal);
        }
        if (sliderVal <= 0.5) {
            speedMultToSend = juce::jmap(sliderVal, 0.0, 0.5, SPEED_SLIDER_MIN_VAL, SPEED_SLIDER_MID_VAL);
        } else {
            speedMultToSend = juce::jmap(sliderVal, 0.5, 1.0, SPEED_SLIDER_MID_VAL, SPEED_SLIDER_MAX_VAL);
        }
        mListeners.call([&](Listener & l) { l.positionTrajectoryCurrentSpeedChangedCallback(speedMultToSend); });
    };

    mElevationCycleSpeedSlider.setNormalisableRange(juce::NormalisableRange<double>(0.0, 1.0, 0.01));
    mElevationCycleSpeedSlider.setDoubleClickReturnValue(true, 0.5);
    auto eleCycleSpeed{ mAPVTS.state.getProperty("eleCycleSpeed") };
    if (eleCycleSpeed.isVoid()) {
        eleCycleSpeed = 0.5;
    }
    mElevationCycleSpeedSlider.setValue(eleCycleSpeed);
    mElevationCycleSpeedSlider.setSliderSnapsToMousePosition(false);
    mElevationCycleSpeedSlider.setTextBoxStyle(juce::Slider::NoTextBox, false, 40, 20);
    mElevationCycleSpeedSlider.setColour(juce::Slider::textBoxOutlineColourId, juce::Colours::transparentBlack);
    addAndMakeVisible(&mElevationCycleSpeedSlider);
    mElevationCycleSpeedSlider.onValueChange = [this] {
        auto const sliderVal{ mElevationCycleSpeedSlider.getValue() };
        mAPVTS.state.setProperty("eleCycleSpeed", sliderVal, nullptr);
        double speedMultToSend{};
        if (mSpeedLinked) {
            mPositionCycleSpeedSlider.setValue(sliderVal);
        }
        if (sliderVal <= 0.5) {
            speedMultToSend = juce::jmap(sliderVal, 0.0, 0.5, SPEED_SLIDER_MIN_VAL, SPEED_SLIDER_MID_VAL);
        } else {
            speedMultToSend = juce::jmap(sliderVal, 0.5, 1.0, SPEED_SLIDER_MID_VAL, SPEED_SLIDER_MAX_VAL);
        }
        mListeners.call([&](Listener & l) { l.elevationTrajectoryCurrentSpeedChangedCallback(speedMultToSend); });
    };

    // Removed because this interacted with DAWs
    // mPositionActivateButton.addShortcut(juce::KeyPress('a', 0, 0));

    addAndMakeVisible(&mPositionActivateButton);
    mPositionActivateButton.setButtonText("Activate");
    mPositionActivateButton.setClickingTogglesState(true);
    mPositionActivateButton.onClick = [this] {
        mListeners.call(
            [&](Listener & l) { l.positionTrajectoryStateChangedCallback(mPositionActivateButton.getToggleState()); });
        mDurationUnitCombo.grabKeyboardFocus();
    };

    mPositionBackAndForthToggle.setButtonText("Back & Forth");
    mPositionBackAndForthToggle.onClick = [this] {
        mListeners.call([&](Listener & l) {
            l.positionTrajectoryBackAndForthChangedCallback(mPositionBackAndForthToggle.getToggleState());
        });
        setPositionDampeningEnabled(mPositionBackAndForthToggle.getToggleState());
    };
    addAndMakeVisible(&mPositionBackAndForthToggle);

    mDampeningLabel.setText("Number of cycles", juce::NotificationType::dontSendNotification);
    addAndMakeVisible(&mDampeningLabel);
    mDampeningLabel2ndLine.setText("dampening:", juce::dontSendNotification);
    addAndMakeVisible(&mDampeningLabel2ndLine);

    addAndMakeVisible(&mPositionDampeningEditor);
    mPositionDampeningEditor.setFont(mGrisLookAndFeel.getFont());
    mPositionDampeningEditor.setTextToShowWhenEmpty("0", juce::Colours::white);
    mPositionDampeningEditor.setText("0", false);
    mPositionDampeningEditor.setInputRestrictions(10, "0123456789");
    mPositionDampeningEditor.onReturnKey = [this] { mDurationUnitCombo.grabKeyboardFocus(); };
    mPositionDampeningEditor.onFocusLost = [this] {
        mListeners.call([&](Listener & l) {
            l.positionTrajectoryDampeningCyclesChangedCallback(mPositionDampeningEditor.getText().getIntValue());
        });
        mDurationUnitCombo.grabKeyboardFocus();
    };

    mDeviationLabel.setText("Deviation degrees", juce::NotificationType::dontSendNotification);
    addAndMakeVisible(&mDeviationLabel);
    mDeviationLabel2ndLine.setText("per cycle:", juce::NotificationType::dontSendNotification);
    addAndMakeVisible(&mDeviationLabel2ndLine);

    addAndMakeVisible(&mDeviationEditor);
    mDeviationEditor.setFont(mGrisLookAndFeel.getFont());
    mDeviationEditor.setTextToShowWhenEmpty("0", juce::Colours::white);
    mDeviationEditor.setText("0", false);
    mDeviationEditor.setInputRestrictions(10, "-0123456789.");
    mDeviationEditor.onReturnKey = [this] { mDurationUnitCombo.grabKeyboardFocus(); };
    mDeviationEditor.onFocusLost = [this] {
        mListeners.call([&](Listener & l) {
            l.trajectoryDeviationPerCycleChangedCallback(std::fmod(mDeviationEditor.getText().getFloatValue(), 360.0f));
        });
        mDeviationEditor.setText(juce::String(std::fmod(mDeviationEditor.getText().getFloatValue(), 360.0)));
        mDurationUnitCombo.grabKeyboardFocus();
    };

    mElevationActivateButton.addShortcut(juce::KeyPress('a', juce::ModifierKeys::shiftModifier, 0));
    addChildComponent(&mElevationActivateButton);
    mElevationActivateButton.setButtonText("Activate");
    mElevationActivateButton.setClickingTogglesState(true);
    mElevationActivateButton.onClick = [this] {
        mListeners.call([&](Listener & l) {
            l.elevationTrajectoryStateChangedCallback(mElevationActivateButton.getToggleState());
        });
        mDurationUnitCombo.grabKeyboardFocus();
    };

    mElevationBackAndForthToggle.setButtonText("Back & Forth");
    mElevationBackAndForthToggle.onClick = [this] {
        mListeners.call([&](Listener & l) {
            l.elevationTrajectoryBackAndForthChangedCallback(mElevationBackAndForthToggle.getToggleState());
        });
        setElevationDampeningEnabled(mElevationBackAndForthToggle.getToggleState());
    };
    addAndMakeVisible(&mElevationBackAndForthToggle);

    addAndMakeVisible(&mElevationDampeningEditor);
    mElevationDampeningEditor.setFont(mGrisLookAndFeel.getFont());
    mElevationDampeningEditor.setTextToShowWhenEmpty("0", juce::Colours::white);
    mElevationDampeningEditor.setText("0", false);
    mElevationDampeningEditor.setInputRestrictions(10, "0123456789");
    mElevationDampeningEditor.onReturnKey = [this] { mDurationUnitCombo.grabKeyboardFocus(); };
    mElevationDampeningEditor.onFocusLost = [this] {
        mListeners.call([&](Listener & l) {
            l.elevationTrajectoryDampeningCyclesChangedCallback(mElevationDampeningEditor.getText().getIntValue());
        });
        mDurationUnitCombo.grabKeyboardFocus();
    };

    mRandomXYLabel.setText("Random", juce::dontSendNotification);
    addAndMakeVisible(&mRandomXYLabel);

    addAndMakeVisible(&mRandomXYToggle);
    auto posRandomToggle{ mAPVTS.state.getProperty("posRandomToggle") };
    if (posRandomToggle.isVoid()) {
        posRandomToggle = false;
    }
    mRandomXYToggle.setToggleState(posRandomToggle, juce::dontSendNotification);
    mRandomXYToggle.onClick = [this] {
        auto const toggleState{ mRandomXYToggle.getToggleState() };
        mAPVTS.state.setProperty("posRandomToggle", toggleState, nullptr);
        mListeners.call([&](Listener & l) { l.positionTrajectoryRandomEnableChangedCallback(toggleState); });
    };

    addAndMakeVisible(&mRandomXYLoopButton);
    auto posRandomLoop{ mAPVTS.state.getProperty("posRandomLoop") };
    if (posRandomLoop.isVoid()) {
        posRandomLoop = false;
    }
    mRandomXYLoopButton.setToggleState(posRandomLoop, juce::dontSendNotification);
    mRandomXYLoopButton.setButtonText("Loop");
    mRandomXYLoopButton.setClickingTogglesState(true);
    mRandomXYLoopButton.onClick = [this] {
        auto const toggleState{ mRandomXYLoopButton.getToggleState() };
        mAPVTS.state.setProperty("posRandomLoop", toggleState, nullptr);
        mListeners.call([&](Listener & l) { l.positionTrajectoryRandomLoopChangedCallback(toggleState); });
    };

    addAndMakeVisible(&mRandomTypeXYCombo);
    mRandomTypeXYCombo.addItem("Continuous", TrajectoryRandomTypeToInt(TrajectoryRandomType::continuous));
    mRandomTypeXYCombo.addItem("Discrete", TrajectoryRandomTypeToInt(TrajectoryRandomType::discrete));
    mRandomTypeXYCombo.onChange = [this] {
        auto const selectedId{ mRandomTypeXYCombo.getSelectedId() };
        mAPVTS.state.setProperty("posRandomType", selectedId, nullptr);
        mRandomXYLoopButton.setVisible(TrajectoryRandomTypeFromInt(selectedId) == TrajectoryRandomType::continuous);
        mListeners.call([&](Listener & l) {
            l.positionTrajectoryRandomTypeChangedCallback(TrajectoryRandomTypeFromInt(selectedId));
        });
    };
    auto posRandomType{ mAPVTS.state.getProperty("posRandomType") };
    if (posRandomType.isVoid()) {
        posRandomType = TrajectoryRandomTypeToInt(TrajectoryRandomType::continuous);
    }
    mRandomTypeXYCombo.setSelectedId(posRandomType);

    addAndMakeVisible(&mRandomProximityXYLabel);
    mRandomProximityXYLabel.setText("Proximity", juce::dontSendNotification);

    addAndMakeVisible(&mRandomTimeMinXYLabel);
    mRandomTimeMinXYLabel.setText("Time Min.", juce::dontSendNotification);

    addAndMakeVisible(&mRandomTimeMaxXYLabel);
    mRandomTimeMaxXYLabel.setText("Time Max.", juce::dontSendNotification);

    addAndMakeVisible(&mRandomProximityXYSlider);
    mRandomProximityXYSlider.setNumDecimalPlacesToDisplay(2);
    mRandomProximityXYSlider.setRange(0.0, 1.0);
    mRandomProximityXYSlider.onValueChange = [this] {
        auto proxVal{ mRandomProximityXYSlider.getValue() };
        mAPVTS.state.setProperty("posRandomProximity", proxVal, nullptr);
        mListeners.call([&](Listener & l) { l.positionTrajectoryRandomProximityChangedCallback(proxVal); });
    };
    auto posRandomProximity{ mAPVTS.state.getProperty("posRandomProximity") };
    if (posRandomProximity.isVoid()) {
        posRandomProximity = 0.0;
    }
    mRandomProximityXYSlider.setValue(posRandomProximity);

    addAndMakeVisible(&mRandomTimeMinXYSlider);
    mRandomTimeMinXYSlider.setNumDecimalPlacesToDisplay(2);
    mRandomTimeMinXYSlider.setRange(0.03, 5.0);
    mRandomTimeMinXYSlider.onValueChange = [this] {
        auto const timeMin{ mRandomTimeMinXYSlider.getValue() };
        auto const timeMax{ mRandomTimeMaxXYSlider.getValue() };
        if (timeMin > timeMax) {
            mRandomTimeMaxXYSlider.setValue(timeMin);
        }
        mListeners.call([&](Listener & l) { l.positionTrajectoryRandomTimeMinChangedCallback(timeMin); });
        mAPVTS.state.setProperty("posRandomTimeMin", timeMin, nullptr);
    };
    auto posRandomTimeMin{ mAPVTS.state.getProperty("posRandomTimeMin") };
    if (posRandomTimeMin.isVoid()) {
        posRandomTimeMin = 0.03;
    }
    mRandomTimeMinXYSlider.setValue(posRandomTimeMin, juce::dontSendNotification);

    addAndMakeVisible(&mRandomTimeMaxXYSlider);
    mRandomTimeMaxXYSlider.setNumDecimalPlacesToDisplay(2);
    mRandomTimeMaxXYSlider.setRange(0.03, 5.0);
    mRandomTimeMaxXYSlider.onValueChange = [this] {
        auto const timeMin{ mRandomTimeMinXYSlider.getValue() };
        auto const timeMax{ mRandomTimeMaxXYSlider.getValue() };
        if (timeMax < timeMin) {
            mRandomTimeMinXYSlider.setValue(timeMax);
        }
        mListeners.call([&](Listener & l) { l.positionTrajectoryRandomTimeMaxChangedCallback(timeMax); });
        mAPVTS.state.setProperty("posRandomTimeMax", timeMax, nullptr);
    };
    auto posRandomTimeMax{ mAPVTS.state.getProperty("posRandomTimeMax") };
    if (posRandomTimeMax.isVoid()) {
        posRandomTimeMax = 0.03;
    }
    mRandomTimeMaxXYSlider.setValue(posRandomTimeMax, juce::dontSendNotification);

    mRandomZLabel.setText("Random", juce::dontSendNotification);
    addAndMakeVisible(&mRandomZLabel);

    addAndMakeVisible(&mRandomZToggle);
    auto eleRandomToggle{ mAPVTS.state.getProperty("eleRandomToggle") };
    if (eleRandomToggle.isVoid()) {
        eleRandomToggle = false;
    }
    mRandomZToggle.setToggleState(eleRandomToggle, juce::dontSendNotification);
    mRandomZToggle.onClick = [this] {
        auto const toggleState{ mRandomZToggle.getToggleState() };
        mAPVTS.state.setProperty("eleRandomToggle", toggleState, nullptr);
        mListeners.call([&](Listener & l) { l.elevationTrajectoryRandomEnableChangedCallback(toggleState); });
    };

    addAndMakeVisible(&mRandomZLoopButton);
    auto eleRandomLoop{ mAPVTS.state.getProperty("eleRandomLoop") };
    if (eleRandomLoop.isVoid()) {
        eleRandomLoop = false;
    }
    mRandomZLoopButton.setToggleState(eleRandomLoop, juce::dontSendNotification);
    mRandomZLoopButton.setButtonText("Loop");
    mRandomZLoopButton.setClickingTogglesState(true);
    mRandomZLoopButton.onClick = [this] {
        auto const toggleState{ mRandomZLoopButton.getToggleState() };
        mAPVTS.state.setProperty("eleRandomLoop", toggleState, nullptr);
        mListeners.call([&](Listener & l) { l.elevationTrajectoryRandomLoopChangedCallback(toggleState); });
    };

    addAndMakeVisible(&mRandomTypeZCombo);
    mRandomTypeZCombo.addItem("Continuous", TrajectoryRandomTypeToInt(TrajectoryRandomType::continuous));
    mRandomTypeZCombo.addItem("Discrete", TrajectoryRandomTypeToInt(TrajectoryRandomType::discrete));
    mRandomTypeZCombo.onChange = [this] {
        auto const selectedId{ mRandomTypeZCombo.getSelectedId() };
        mAPVTS.state.setProperty("eleRandomType", selectedId, nullptr);
        mRandomZLoopButton.setVisible(TrajectoryRandomTypeFromInt(selectedId) == TrajectoryRandomType::continuous);
        mListeners.call([&](Listener & l) {
            l.elevationTrajectoryRandomTypeChangedCallback(TrajectoryRandomTypeFromInt(selectedId));
        });
    };
    auto eleRandomType{ mAPVTS.state.getProperty("eleRandomType") };
    if (eleRandomType.isVoid()) {
        eleRandomType = TrajectoryRandomTypeToInt(TrajectoryRandomType::continuous);
    }
    mRandomTypeZCombo.setSelectedId(eleRandomType);

    addAndMakeVisible(&mRandomProximityZLabel);
    mRandomProximityZLabel.setText("Proximity", juce::dontSendNotification);

    addAndMakeVisible(&mRandomTimeMinZLabel);
    mRandomTimeMinZLabel.setText("Time Min.", juce::dontSendNotification);

    addAndMakeVisible(&mRandomTimeMaxZLabel);
    mRandomTimeMaxZLabel.setText("Time Max.", juce::dontSendNotification);

    addAndMakeVisible(&mRandomProximityZSlider);
    mRandomProximityZSlider.setNumDecimalPlacesToDisplay(2);
    mRandomProximityZSlider.setRange(0.0, 1.0);
    mRandomProximityZSlider.onValueChange = [this] {
        auto proxVal{ mRandomProximityZSlider.getValue() };
        mAPVTS.state.setProperty("eleRandomProximity", proxVal, nullptr);
        mListeners.call([&](Listener & l) { l.elevationTrajectoryRandomProximityChangedCallback(proxVal); });
    };
    auto eleRandomProximity{ mAPVTS.state.getProperty("eleRandomProximity") };
    if (eleRandomProximity.isVoid()) {
        eleRandomProximity = 0.0;
    }
    mRandomProximityZSlider.setValue(eleRandomProximity);

    addAndMakeVisible(&mRandomTimeMinZSlider);
    mRandomTimeMinZSlider.setNumDecimalPlacesToDisplay(2);
    mRandomTimeMinZSlider.setRange(0.03, 5.0);
    mRandomTimeMinZSlider.onValueChange = [this] {
        auto const timeMin{ mRandomTimeMinZSlider.getValue() };
        auto const timeMax{ mRandomTimeMaxZSlider.getValue() };
        if (timeMin > timeMax) {
            mRandomTimeMaxZSlider.setValue(timeMin);
        }
        mListeners.call([&](Listener & l) { l.elevationTrajectoryRandomTimeMinChangedCallback(timeMin); });
        mAPVTS.state.setProperty("eleRandomTimeMin", timeMin, nullptr);
    };
    auto eleRandomTimeMin{ mAPVTS.state.getProperty("eleRandomTimeMin") };
    if (eleRandomTimeMin.isVoid()) {
        eleRandomTimeMin = 0.03;
    }
    mRandomTimeMinZSlider.setValue(eleRandomTimeMin, juce::dontSendNotification);

    addAndMakeVisible(&mRandomTimeMaxZSlider);
    mRandomTimeMaxZSlider.setNumDecimalPlacesToDisplay(2);
    mRandomTimeMaxZSlider.setRange(0.03, 5.0);
    mRandomTimeMaxZSlider.onValueChange = [this] {
        auto const timeMin{ mRandomTimeMinZSlider.getValue() };
        auto const timeMax{ mRandomTimeMaxZSlider.getValue() };
        if (timeMax < timeMin) {
            mRandomTimeMinZSlider.setValue(timeMax);
        }
        mListeners.call([&](Listener & l) { l.elevationTrajectoryRandomTimeMaxChangedCallback(timeMax); });
        mAPVTS.state.setProperty("eleRandomTimeMax", timeMax, nullptr);
    };
    auto eleRandomTimeMax{ mAPVTS.state.getProperty("eleRandomTimeMax") };
    if (eleRandomTimeMax.isVoid()) {
        eleRandomTimeMax = 0.03;
    }
    mRandomTimeMaxZSlider.setValue(eleRandomTimeMax, juce::dontSendNotification);
}

//==============================================================================
void SectionAbstractSpatialization::actualizeValueTreeState()
{
    mPositionCycleSpeedSlider.onValueChange();
    mRandomXYToggle.onClick();
    mRandomXYLoopButton.onClick();
    mRandomTypeXYCombo.onChange();
    mRandomProximityXYSlider.onValueChange();
    mRandomTimeMinXYSlider.onValueChange();
    mRandomTimeMaxXYSlider.onValueChange();

    mElevationCycleSpeedSlider.onValueChange();
    mRandomZToggle.onClick();
    mRandomZLoopButton.onClick();
    mRandomTypeZCombo.onChange();
    mRandomProximityZSlider.onValueChange();
    mRandomTimeMinZSlider.onValueChange();
    mRandomTimeMaxZSlider.onValueChange();
}

//==============================================================================
void SectionAbstractSpatialization::setSpatMode(SpatMode const spatMode)
{
    mSpatMode = spatMode;
    resized();
}

//==============================================================================
void SectionAbstractSpatialization::setTrajectoryType(int const type)
{
    mPositionTrajectoryTypeCombo.setSelectedId(type);
}

//==============================================================================
void SectionAbstractSpatialization::setElevationTrajectoryType(int const type)
{
    mElevationTrajectoryTypeCombo.setSelectedId(type);
}

//==============================================================================
void SectionAbstractSpatialization::setPositionBackAndForth(bool const state)
{
    mPositionBackAndForthToggle.setToggleState(state, juce::NotificationType::sendNotification);
    setPositionDampeningEnabled(state);
}

//==============================================================================
void SectionAbstractSpatialization::setElevationBackAndForth(bool const state)
{
    mElevationBackAndForthToggle.setToggleState(state, juce::NotificationType::sendNotification);
    setElevationDampeningEnabled(state);
}

//==============================================================================
void SectionAbstractSpatialization::setPositionDampeningEnabled(bool const state)
{
    mPositionDampeningEditor.setEnabled(state);
    juce::String text = mPositionDampeningEditor.getText();
    mPositionDampeningEditor.clear();
    if (state)
        mPositionDampeningEditor.setColour(juce::TextEditor::textColourId, juce::Colour::fromRGB(235, 245, 250));
    else
        mPositionDampeningEditor.setColour(juce::TextEditor::textColourId, juce::Colour::fromRGB(172, 172, 172));
    mPositionDampeningEditor.setText(text);
}

//==============================================================================
void SectionAbstractSpatialization::setElevationDampeningEnabled(bool const state)
{
    mElevationDampeningEditor.setEnabled(state);
    juce::String text = mElevationDampeningEditor.getText();
    mElevationDampeningEditor.clear();
    if (state)
        mElevationDampeningEditor.setColour(juce::TextEditor::textColourId, juce::Colour::fromRGB(235, 245, 250));
    else
        mElevationDampeningEditor.setColour(juce::TextEditor::textColourId, juce::Colour::fromRGB(172, 172, 172));
    mElevationDampeningEditor.setText(text);
}

//==============================================================================
void SectionAbstractSpatialization::setPositionDampeningCycles(int const value)
{
    mPositionDampeningEditor.setText(juce::String(value));
}

//==============================================================================
void SectionAbstractSpatialization::setElevationDampeningCycles(int const value)
{
    mElevationDampeningEditor.setText(juce::String(value));
}

//==============================================================================
void SectionAbstractSpatialization::setDeviationPerCycle(float const value)
{
    mDeviationEditor.setText(juce::String(value));
}

//==============================================================================
void SectionAbstractSpatialization::setPositionActivateState(bool const state)
{
    mPositionActivateButton.setToggleState(state, juce::NotificationType::dontSendNotification);
}

//==============================================================================
void SectionAbstractSpatialization::setElevationActivateState(bool const state)
{
    mElevationActivateButton.setToggleState(state, juce::NotificationType::dontSendNotification);
}

//==============================================================================
void SectionAbstractSpatialization::setSpeedLinkState(bool state)
{
    mSpeedLinked = state;
    repaint();
}

//==============================================================================
void SectionAbstractSpatialization::setCycleDuration(double const value)
{
    mDurationEditor.setText(juce::String(value));
    mListeners.call([&](Listener & l) {
        l.trajectoryCycleDurationChangedCallback(mDurationEditor.getText().getDoubleValue(),
                                                 mDurationUnitCombo.getSelectedId());
    });
}

//==============================================================================
void SectionAbstractSpatialization::setDurationUnit(int const value)
{
    mDurationUnitCombo.setSelectedId(value, juce::NotificationType::sendNotificationSync);
    mListeners.call([&](Listener & l) {
        l.trajectoryDurationUnitChangedCallback(mDurationEditor.getText().getDoubleValue(),
                                                mDurationUnitCombo.getSelectedId());
    });
}

//==============================================================================
void SectionAbstractSpatialization::mouseDown(juce::MouseEvent const & event)
{
    if (mSpatMode == SpatMode::cube) {
        // Area where the speedLinked arrow is shown.
        juce::Rectangle<float> const speedLinkedArrowArea{ 292.0f, 70.0f, 30.0f, 17.0f };
        if (speedLinkedArrowArea.contains(event.getMouseDownPosition().toFloat())) {
            mSpeedLinked = !mSpeedLinked;
            repaint();
        }
    }
}

//==============================================================================
void SectionAbstractSpatialization::textEditorFocusLost(juce::TextEditor & textEd)
{
    textEditorReturnKeyPressed(textEd);
}

//==============================================================================
void SectionAbstractSpatialization::paint(juce::Graphics & g)
{
    g.fillAll(mGrisLookAndFeel.findColour(juce::ResizableWindow::backgroundColourId));

    if (mSpatMode == SpatMode::cube) {
        if (mSpeedLinked)
            g.setColour(juce::Colours::orange);
        else
            g.setColour(juce::Colours::black);
        g.drawArrow(juce::Line<float>(302.0f, 78.0f, 292.0f, 78.0f), 4, 10, 7);
        g.drawArrow(juce::Line<float>(297.0f, 78.0f, 317.0f, 78.0f), 4, 10, 7);

        g.setColour(juce::Colours::orange);
        g.setFont(16.0f);
        auto eleCycleSpeedSliderBounds{ mElevationCycleSpeedSlider.getBounds() };
        g.drawText("-",
                   eleCycleSpeedSliderBounds.getTopLeft().getX() - 1,
                   eleCycleSpeedSliderBounds.getTopLeft().getY() - 11,
                   15,
                   15,
                   juce::Justification::centred);
        g.drawText("+",
                   eleCycleSpeedSliderBounds.getTopRight().getX() - 13,
                   eleCycleSpeedSliderBounds.getTopRight().getY() - 11,
                   15,
                   15,
                   juce::Justification::centred);
    }

    g.setColour(juce::Colours::orange);
    g.setFont(16.0f);
    auto posCycleSpeedSliderBounds{ mPositionCycleSpeedSlider.getBounds() };
    g.drawText("-",
               posCycleSpeedSliderBounds.getTopLeft().getX() - 1,
               posCycleSpeedSliderBounds.getTopLeft().getY() - 11,
               15,
               15,
               juce::Justification::centred);
    g.drawText("+",
               posCycleSpeedSliderBounds.getTopRight().getX() - 13,
               posCycleSpeedSliderBounds.getTopRight().getY() - 11,
               15,
               15,
               juce::Justification::centred);
}

//==============================================================================
void SectionAbstractSpatialization::resized()
{
    mTrajectoryTypeLabel.setBounds(5, 7, 150, 10);
    mTrajectoryTypeXYLabel.setBounds(90, 7, 150, 10);
    mPositionTrajectoryTypeCombo.setBounds(115, 5, 175, 15);
    mPositionBackAndForthToggle.setBounds(196, 27, 94, 15);
    mDampeningLabel.setBounds(5, 18, 150, 22);
    mDampeningLabel2ndLine.setBounds(5, 26, 150, 22);
    mPositionDampeningEditor.setBounds(115, 27, 75, 15);
    mDeviationLabel.setBounds(110, 40, 150, 22);
    mDeviationLabel2ndLine.setBounds(110, 48, 150, 22);
    mDeviationEditor.setBounds(211, 49, 78, 15);
    mPositionCycleSpeedSlider.setBounds(113, 72, 180, 16);

    mRandomXYToggle.setBounds(112, 92, 60, 15);
    mRandomXYLabel.setBounds(124, 95, 150, 10);
    mRandomXYLoopButton.setBounds(177, 92, 32, 15);
    mRandomTypeXYCombo.setBounds(211, 92, 78, 15);

    mRandomProximityXYLabel.setBounds(110, 110, 60, 10);
    mRandomTimeMinXYLabel.setBounds(175, 110, 60, 10);
    mRandomTimeMaxXYLabel.setBounds(235, 110, 60, 10);

    mRandomProximityXYSlider.setBounds(120, 122, 35, 12);
    mRandomTimeMinXYSlider.setBounds(186, 122, 35, 12);
    mRandomTimeMaxXYSlider.setBounds(249, 122, 35, 12);

    mPositionActivateButton.setBounds(114, 138, 176, 20);

    mTrajectoryTypeZLabel.setBounds(303, 7, 150, 10);
    mElevationTrajectoryTypeCombo.setBounds(320, 5, 175, 15);
    mElevationDampeningEditor.setBounds(320, 27, 75, 15);
    mElevationBackAndForthToggle.setBounds(401, 27, 94, 15);

    mElevationCycleSpeedSlider.setBounds(317, 72, 180, 16);

//    mRandomZLabel.setBounds(315, 113, 150, 10);
//    mRandomZToggle.setBounds(386, 112, 88, 15);
//    mRandomTypeZCombo.setBounds(416, 112, 78, 15);
    auto const xOffsetForZRandom{ 205 };
    mRandomZToggle.setBounds(112 + xOffsetForZRandom, 92, 60, 15);
    mRandomZLabel.setBounds(124 + xOffsetForZRandom, 95, 150, 10);
    mRandomZLoopButton.setBounds(177 + xOffsetForZRandom, 92, 32, 15);
    mRandomTypeZCombo.setBounds(211 + xOffsetForZRandom, 92, 78, 15);

    mRandomProximityZLabel.setBounds(110 + xOffsetForZRandom, 110, 60, 10);
    mRandomTimeMinZLabel.setBounds(175 + xOffsetForZRandom, 110, 60, 10);
    mRandomTimeMaxZLabel.setBounds(235 + xOffsetForZRandom, 110, 60, 10);

    mRandomProximityZSlider.setBounds(120 + xOffsetForZRandom, 122, 35, 12);
    mRandomTimeMinZSlider.setBounds(186 + xOffsetForZRandom, 122, 35, 12);
    mRandomTimeMaxZSlider.setBounds(249 + xOffsetForZRandom, 122, 35, 12);

    mElevationActivateButton.setBounds(319, 138, 176, 20);

    if (mSpatMode == SpatMode::cube) {
        mTrajectoryTypeXYLabel.setVisible(true);
        mTrajectoryTypeZLabel.setVisible(true);
        mElevationTrajectoryTypeCombo.setVisible(true);
        mElevationActivateButton.setVisible(true);
        mElevationBackAndForthToggle.setVisible(true);
        mElevationDampeningEditor.setVisible(true);
        mRandomZLabel.setVisible(true);
        mRandomZToggle.setVisible(true);
        mRandomZLoopButton.setVisible(true);
        mRandomTypeZCombo.setVisible(true);
        mRandomProximityZLabel.setVisible(true);
        mRandomTimeMinZLabel.setVisible(true);
        mRandomTimeMaxZLabel.setVisible(true);
        mRandomProximityZSlider.setVisible(true);
        mRandomTimeMinZSlider.setVisible(true);
        mRandomTimeMaxZSlider.setVisible(true);
        mElevationCycleSpeedSlider.setVisible(true);
    } else {
        mTrajectoryTypeXYLabel.setVisible(false);
        mTrajectoryTypeZLabel.setVisible(false);
        mElevationTrajectoryTypeCombo.setVisible(false);
        mElevationActivateButton.setVisible(false);
        mElevationBackAndForthToggle.setVisible(false);
        mElevationDampeningEditor.setVisible(false);
        mRandomZLabel.setVisible(false);
        mRandomZToggle.setVisible(false);
        mRandomZLoopButton.setVisible(false);
        mRandomTypeZCombo.setVisible(false);
        mRandomProximityZLabel.setVisible(false);
        mRandomTimeMinZLabel.setVisible(false);
        mRandomTimeMaxZLabel.setVisible(false);
        mRandomProximityZSlider.setVisible(false);
        mRandomTimeMinZSlider.setVisible(false);
        mRandomTimeMaxZSlider.setVisible(false);
        mElevationCycleSpeedSlider.setVisible(false);
    }

    mDurationLabel.setBounds(495, 5, 90, 20);
    mDurationEditor.setBounds(500, 30, 90, 20);
    mDurationUnitCombo.setBounds(500, 60, 90, 20);
}

} // namespace gris
