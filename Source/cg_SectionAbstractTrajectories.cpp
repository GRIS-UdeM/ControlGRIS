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

#include "cg_SectionAbstractTrajectories.hpp"

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
SectionAbstractTrajectories::SectionAbstractTrajectories(GrisLookAndFeel & grisLookAndFeel,
                                                         juce::AudioProcessorValueTreeState & apvts,
                                                         ControlGrisAudioProcessor & audioProcessor)
    : mGrisLookAndFeel(grisLookAndFeel)
    , mAPVTS(apvts)
    , mProcessor(audioProcessor)
    , mRandomProximityXYSlider(grisLookAndFeel)
    , mRandomTimeMinXYSlider(grisLookAndFeel)
    , mRandomTimeMaxXYSlider(grisLookAndFeel)
    , mRandomProximityZSlider(grisLookAndFeel)
    , mRandomTimeMinZSlider(grisLookAndFeel)
    , mRandomTimeMaxZSlider(grisLookAndFeel)
{
    setName("SectionAbstractTrajectories");

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
    mDurationEditor.setLookAndFeel(&mGrisLookAndFeel);
    mDurationEditor.setFont(mGrisLookAndFeel.getFont());
    mDurationEditor.setTextToShowWhenEmpty("1", juce::Colours::white);
    mDurationEditor.setText("5", false);
    mDurationEditor.setInputRestrictions(10, "0123456789.");
    mDurationEditor.onFocusLost = [this] {
        mDurationEditor.moveCaretToEnd();
        mListeners.call([&](Listener & l) {
            l.trajectoryCycleDurationChangedCallback(mDurationEditor.getText().getDoubleValue(),
                                                     mDurationUnitCombo.getSelectedId());
        });
        unfocusAllComponents();
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

    mCycleSpeedLabel.setText("Speed:", juce::dontSendNotification);
    addAndMakeVisible(&mCycleSpeedLabel);

    mPositionCycleSpeedSlider.setNormalisableRange(juce::NormalisableRange<double>(0.0, 1.0, 0.01));
    mPositionCycleSpeedSlider.setDoubleClickReturnValue(true, 0.5);
    auto posCycleSpeed{ mAPVTS.state.getProperty(Automation::Ids::POSITION_SPEED_SLIDER) };
    if (posCycleSpeed.isVoid()) {
        posCycleSpeed = 0.5;
    }
    mPositionCycleSpeedSlider.setValue(posCycleSpeed, juce::sendNotification);
    mPositionCycleSpeedSlider.setSliderSnapsToMousePosition(false);
    mPositionCycleSpeedSlider.setTextBoxStyle(juce::Slider::NoTextBox, false, 40, 20);
    mPositionCycleSpeedSlider.setColour(juce::Slider::textBoxOutlineColourId, juce::Colours::transparentBlack);
    addAndMakeVisible(&mPositionCycleSpeedSlider);
    mPositionCycleSpeedSlider.onValueChange = [this] {
        positionSpeedSliderChangedStartedCallback();
        auto const sliderVal{ mPositionCycleSpeedSlider.getValue() };
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
        auto * parameter{ mAPVTS.getParameter(Automation::Ids::POSITION_SPEED_SLIDER) };
        auto const gestureLock{ mProcessor.getChangeGestureManager().getScopedLock(
            Automation::Ids::POSITION_SPEED_SLIDER) };
        parameter->setValueNotifyingHost(static_cast<float>(sliderVal));
        positionSpeedSliderChangedEndedCallback();
        repaint();
    };

    mElevationCycleSpeedSlider.setNormalisableRange(juce::NormalisableRange<double>(0.0, 1.0, 0.01));
    mElevationCycleSpeedSlider.setDoubleClickReturnValue(true, 0.5);
    auto eleCycleSpeed{ mAPVTS.state.getProperty(Automation::Ids::ELEVATION_SPEED_SLIDER) };
    if (eleCycleSpeed.isVoid()) {
        eleCycleSpeed = 0.5;
    }
    mElevationCycleSpeedSlider.setValue(eleCycleSpeed, juce::sendNotification);
    mElevationCycleSpeedSlider.setSliderSnapsToMousePosition(false);
    mElevationCycleSpeedSlider.setTextBoxStyle(juce::Slider::NoTextBox, false, 40, 20);
    mElevationCycleSpeedSlider.setColour(juce::Slider::textBoxOutlineColourId, juce::Colours::transparentBlack);
    addAndMakeVisible(&mElevationCycleSpeedSlider);
    mElevationCycleSpeedSlider.onValueChange = [this] {
        elevationSpeedSliderChangedStartedCallback();
        auto const sliderVal{ mElevationCycleSpeedSlider.getValue() };
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
        auto * parameter{ mAPVTS.getParameter(Automation::Ids::ELEVATION_SPEED_SLIDER) };
        auto const gestureLock{ mProcessor.getChangeGestureManager().getScopedLock(
            Automation::Ids::ELEVATION_SPEED_SLIDER) };
        parameter->setValueNotifyingHost(static_cast<float>(sliderVal));
        elevationSpeedSliderChangedEndedCallback();
        repaint();
    };

    // Removed because this interacted with DAWs
    // mPositionActivateButton.addShortcut(juce::KeyPress('a', 0, 0));

    addAndMakeVisible(&mPositionActivateButton);
    mPositionActivateButton.setButtonText("Activate");
    mPositionActivateButton.setClickingTogglesState(true);
    mPositionActivateButton.onClick = [this] {
        positionActivateButtonChangedStartedCallback();
        auto state{ mPositionActivateButton.getToggleState() };
        if (juce::ModifierKeys::getCurrentModifiers().isShiftDown() && state) {
            mAPVTS.state.setProperty("positionActivateButtonAlwaysOn", true, nullptr);
            mPositionActivateButton.setName("ActivateButton_ON");
            startTimer(100);
        } else {
            mAPVTS.state.setProperty("positionActivateButtonAlwaysOn", false, nullptr);
            mPositionActivateButton.setName("");
            stopTimer();
        }
        if (mActivateLinked) {
            mElevationActivateButton.setToggleState(mPositionActivateButton.getToggleState(), juce::sendNotification);
        }
        mListeners.call(
            [&](Listener & l) { l.positionTrajectoryStateChangedCallback(mPositionActivateButton.getToggleState()); });
        // automation
        auto * parameter{ mAPVTS.getParameter(Automation::Ids::ABSTRACT_TRAJECTORIES_POSITION_ACTIVATE) };
        auto const gestureLock{ mProcessor.getChangeGestureManager().getScopedLock(
            Automation::Ids::ABSTRACT_TRAJECTORIES_POSITION_ACTIVATE) };
        parameter->setValueNotifyingHost(state ? 1.0f : 0.0f);
        positionActivateButtonChangedEndedCallback();
    };

    mPositionBackAndForthToggle.setButtonText("Back & Forth");
    mPositionBackAndForthToggle.onClick = [this] {
        mListeners.call([&](Listener & l) {
            l.positionTrajectoryBackAndForthChangedCallback(mPositionBackAndForthToggle.getToggleState());
        });
        if (!mRandomXYToggle.getToggleState()) {
            setPositionDampeningEnabled(mPositionBackAndForthToggle.getToggleState());
        }
    };
    addAndMakeVisible(&mPositionBackAndForthToggle);

    mDampeningLabel.setText("Number of cycles", juce::NotificationType::dontSendNotification);
    addAndMakeVisible(&mDampeningLabel);
    mDampeningLabel2ndLine.setText("dampening:", juce::dontSendNotification);
    addAndMakeVisible(&mDampeningLabel2ndLine);

    addAndMakeVisible(&mPositionDampeningEditor);
    mPositionDampeningEditor.setLookAndFeel(&mGrisLookAndFeel);
    mPositionDampeningEditor.setFont(mGrisLookAndFeel.getFont());
    mPositionDampeningEditor.setTextToShowWhenEmpty("0", juce::Colours::white);
    mPositionDampeningEditor.setText("0", false);
    mPositionDampeningEditor.setInputRestrictions(10, "0123456789");
    mPositionDampeningEditor.onFocusLost = [this] {
        mPositionDampeningEditor.moveCaretToEnd();
        mListeners.call([&](Listener & l) {
            l.positionTrajectoryDampeningCyclesChangedCallback(mPositionDampeningEditor.getText().getIntValue());
        });
        unfocusAllComponents();
    };

    mDeviationLabel.setText("Deviation degrees", juce::NotificationType::dontSendNotification);
    addAndMakeVisible(&mDeviationLabel);
    mDeviationLabel2ndLine.setText("per cycle:", juce::NotificationType::dontSendNotification);
    addAndMakeVisible(&mDeviationLabel2ndLine);

    addAndMakeVisible(&mDeviationEditor);
    mDeviationEditor.setLookAndFeel(&mGrisLookAndFeel);
    mDeviationEditor.setFont(mGrisLookAndFeel.getFont());
    mDeviationEditor.setTextToShowWhenEmpty("0", juce::Colours::white);
    mDeviationEditor.setText("0", false);
    mDeviationEditor.setInputRestrictions(10, "-0123456789.");
    mDeviationEditor.onFocusLost = [this] {
        mDeviationEditor.moveCaretToEnd();
        mListeners.call([&](Listener & l) {
            l.trajectoryDeviationPerCycleChangedCallback(std::fmod(mDeviationEditor.getText().getFloatValue(), 360.0f));
        });
        mDeviationEditor.setText(juce::String(std::fmod(mDeviationEditor.getText().getFloatValue(), 360.0)));
        unfocusAllComponents();
    };

    mElevationActivateButton.addShortcut(juce::KeyPress('a', juce::ModifierKeys::shiftModifier, 0));
    addChildComponent(&mElevationActivateButton);
    mElevationActivateButton.setButtonText("Activate");
    mElevationActivateButton.setClickingTogglesState(true);
    mElevationActivateButton.onClick = [this] {
        elevationActivateButtonChangedStartedCallback();
        auto state{ mElevationActivateButton.getToggleState() };
        if (juce::ModifierKeys::getCurrentModifiers().isShiftDown() && state) {
            mAPVTS.state.setProperty("elevationActivateButtonAlwaysOn", true, nullptr);
            mElevationActivateButton.setName("ActivateButton_ON");
            startTimer(100);
        } else {
            mAPVTS.state.setProperty("elevationActivateButtonAlwaysOn", false, nullptr);
            mElevationActivateButton.setName("");
            stopTimer();
        }
        if (mActivateLinked) {
            mPositionActivateButton.setToggleState(mElevationActivateButton.getToggleState(), juce::sendNotification);
        }
        mListeners.call([&](Listener & l) {
            l.elevationTrajectoryStateChangedCallback(mElevationActivateButton.getToggleState());
        });
        // automation
        auto * parameter{ mAPVTS.getParameter(Automation::Ids::ABSTRACT_TRAJECTORIES_ELEVATION_ACTIVATE) };
        auto const gestureLock{ mProcessor.getChangeGestureManager().getScopedLock(
            Automation::Ids::ABSTRACT_TRAJECTORIES_ELEVATION_ACTIVATE) };
        parameter->setValueNotifyingHost(state ? 1.0f : 0.0f);
        elevationActivateButtonChangedEndedCallback();
    };

    mElevationBackAndForthToggle.setButtonText("Back & Forth");
    mElevationBackAndForthToggle.onClick = [this] {
        mListeners.call([&](Listener & l) {
            l.elevationTrajectoryBackAndForthChangedCallback(mElevationBackAndForthToggle.getToggleState());
        });
        if (!mRandomZToggle.getToggleState()) {
            setElevationDampeningEnabled(mElevationBackAndForthToggle.getToggleState());
        }
    };
    addAndMakeVisible(&mElevationBackAndForthToggle);

    addAndMakeVisible(&mElevationDampeningEditor);
    mElevationDampeningEditor.setLookAndFeel(&mGrisLookAndFeel);
    mElevationDampeningEditor.setFont(mGrisLookAndFeel.getFont());
    mElevationDampeningEditor.setTextToShowWhenEmpty("0", juce::Colours::white);
    mElevationDampeningEditor.setText("0", false);
    mElevationDampeningEditor.setInputRestrictions(10, "0123456789");
    mElevationDampeningEditor.onFocusLost = [this] {
        mElevationDampeningEditor.moveCaretToEnd();
        mListeners.call([&](Listener & l) {
            l.elevationTrajectoryDampeningCyclesChangedCallback(mElevationDampeningEditor.getText().getIntValue());
        });
        unfocusAllComponents();
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

        setPositionDampeningEnabled(toggleState ? false : mPositionBackAndForthToggle.getToggleState());
        mRandomXYLoopButton.setEnabled(toggleState);
        mRandomStartXYButton.setEnabled(toggleState);
        mRandomTypeXYCombo.setEnabled(toggleState);
        mRandomProximityXYLabel.setEnabled(toggleState);
        mRandomTimeMinXYLabel.setEnabled(toggleState);
        mRandomTimeMaxXYLabel.setEnabled(toggleState);
        mRandomStartXYLabel.setEnabled(toggleState);
        mRandomProximityXYSlider.setEnabled(toggleState);
        mRandomTimeMinXYSlider.setEnabled(toggleState);
        mRandomTimeMaxXYSlider.setEnabled(toggleState);
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

    addAndMakeVisible(&mRandomStartXYButton);
    auto posRandomStart{ mAPVTS.state.getProperty("posRandomStart") };
    if (posRandomStart.isVoid()) {
        posRandomStart = false;
    }
    if (posRandomStart) {
        mRandomStartXYButton.setButtonText("Middle");
    } else {
        mRandomStartXYButton.setButtonText("Start");
    }
    mRandomStartXYButton.setClickingTogglesState(false);
    mRandomStartXYButton.onClick = [this] {
        bool toggleState{ mRandomStartXYButton.getButtonText().compare("Start")
                          != 0 }; // false for start, true for middle
        if (!mRandomStartXYButtonSendStateInit) {
            if (toggleState) {
                mRandomStartXYButton.setButtonText("Start");
            } else {
                mRandomStartXYButton.setButtonText("Middle");
            }
            mAPVTS.state.setProperty("posRandomStart", !toggleState, nullptr);
            mListeners.call([&](Listener & l) { l.positionTrajectoryRandomStartChangedCallback(!toggleState); });
        } else {
            mListeners.call([&](Listener & l) { l.positionTrajectoryRandomStartChangedCallback(toggleState); });
        }
        mRandomStartXYButtonSendStateInit = false;
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
    mRandomProximityXYLabel.setText("Prox.", juce::dontSendNotification);

    addAndMakeVisible(&mRandomTimeMinXYLabel);
    mRandomTimeMinXYLabel.setText("T. Min.", juce::dontSendNotification);

    addAndMakeVisible(&mRandomTimeMaxXYLabel);
    mRandomTimeMaxXYLabel.setText("T. Max.", juce::dontSendNotification);

    addAndMakeVisible(&mRandomStartXYLabel);
    mRandomStartXYLabel.setText("Begin.", juce::dontSendNotification);

    addAndMakeVisible(&mRandomProximityXYSlider);
    mRandomProximityXYSlider.setDefaultNumDecimalPlacesToDisplay(2);
    mRandomProximityXYSlider.setRange(0.0, 1.0, 0.01);
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
    mRandomTimeMinXYSlider.setDefaultNumDecimalPlacesToDisplay(2);
    mRandomTimeMinXYSlider.setRange(0.03, 5.0, 0.01);
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
    mRandomTimeMaxXYSlider.setDefaultNumDecimalPlacesToDisplay(2);
    mRandomTimeMaxXYSlider.setRange(0.03, 5.0, 0.01);
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

        setElevationDampeningEnabled(toggleState ? false : mElevationBackAndForthToggle.getToggleState());
        mRandomZLoopButton.setEnabled(toggleState);
        mRandomStartZButton.setEnabled(toggleState);
        mRandomTypeZCombo.setEnabled(toggleState);
        mRandomProximityZLabel.setEnabled(toggleState);
        mRandomTimeMinZLabel.setEnabled(toggleState);
        mRandomTimeMaxZLabel.setEnabled(toggleState);
        mRandomStartZLabel.setEnabled(toggleState);
        mRandomProximityZSlider.setEnabled(toggleState);
        mRandomTimeMinZSlider.setEnabled(toggleState);
        mRandomTimeMaxZSlider.setEnabled(toggleState);
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

    addAndMakeVisible(&mRandomStartZButton);
    auto eleRandomStart{ mAPVTS.state.getProperty("eleRandomStart") };
    if (eleRandomStart.isVoid()) {
        eleRandomStart = false;
    }
    if (eleRandomStart) {
        mRandomStartZButton.setButtonText("Middle");
    } else {
        mRandomStartZButton.setButtonText("Start");
    }
    mRandomStartZButton.setClickingTogglesState(false);
    mRandomStartZButton.onClick = [this] {
        bool toggleState{ mRandomStartZButton.getButtonText().compare("Start")
                          != 0 }; // false for start, true for middle
        if (!mRandomStartZButtonSendStateInit) {
            if (toggleState) {
                mRandomStartZButton.setButtonText("Start");
            } else {
                mRandomStartZButton.setButtonText("Middle");
            }
            mAPVTS.state.setProperty("eleRandomStart", !toggleState, nullptr);
            mListeners.call([&](Listener & l) { l.elevationTrajectoryRandomStartChangedCallback(!toggleState); });
        } else {
            mListeners.call([&](Listener & l) { l.elevationTrajectoryRandomStartChangedCallback(toggleState); });
        }
        mRandomStartZButtonSendStateInit = false;
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
    mRandomProximityZLabel.setText("Proxi.", juce::dontSendNotification);

    addAndMakeVisible(&mRandomTimeMinZLabel);
    mRandomTimeMinZLabel.setText("T. Min.", juce::dontSendNotification);

    addAndMakeVisible(&mRandomTimeMaxZLabel);
    mRandomTimeMaxZLabel.setText("T. Max.", juce::dontSendNotification);

    addAndMakeVisible(&mRandomStartZLabel);
    mRandomStartZLabel.setText("Begin.", juce::dontSendNotification);

    addAndMakeVisible(&mRandomProximityZSlider);
    mRandomProximityZSlider.setDefaultNumDecimalPlacesToDisplay(2);
    mRandomProximityZSlider.setRange(0.0, 1.0, 0.01);
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
    mRandomTimeMinZSlider.setDefaultNumDecimalPlacesToDisplay(2);
    mRandomTimeMinZSlider.setRange(0.03, 5.0, 0.01);
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
    mRandomTimeMaxZSlider.setDefaultNumDecimalPlacesToDisplay(2);
    mRandomTimeMaxZSlider.setRange(0.03, 5.0, 0.01);
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
void SectionAbstractTrajectories::actualizeValueTreeState()
{
    mPositionCycleSpeedSlider.onValueChange();
    mRandomXYToggle.onClick();
    mRandomXYLoopButton.onClick();
    mRandomStartXYButton.triggerClick();
    mRandomTypeXYCombo.onChange();
    mRandomProximityXYSlider.onValueChange();
    mRandomTimeMinXYSlider.onValueChange();
    mRandomTimeMaxXYSlider.onValueChange();

    mElevationCycleSpeedSlider.onValueChange();
    mRandomZToggle.onClick();
    mRandomZLoopButton.onClick();
    mRandomStartZButton.triggerClick();
    mRandomTypeZCombo.onChange();
    mRandomProximityZSlider.onValueChange();
    mRandomTimeMinZSlider.onValueChange();
    mRandomTimeMaxZSlider.onValueChange();
}

//==============================================================================
void SectionAbstractTrajectories::setSpatMode(SpatMode const spatMode)
{
    mSpatMode = spatMode;
    resized();
}

//==============================================================================
void SectionAbstractTrajectories::setTrajectoryType(int const type)
{
    mPositionTrajectoryTypeCombo.setSelectedId(type);
}

//==============================================================================
void SectionAbstractTrajectories::setElevationTrajectoryType(int const type)
{
    mElevationTrajectoryTypeCombo.setSelectedId(type);
}

//==============================================================================
void SectionAbstractTrajectories::setPositionBackAndForth(bool const state)
{
    mPositionBackAndForthToggle.setToggleState(state, juce::NotificationType::sendNotification);
    setPositionDampeningEnabled(state);
}

//==============================================================================
void SectionAbstractTrajectories::setElevationBackAndForth(bool const state)
{
    mElevationBackAndForthToggle.setToggleState(state, juce::NotificationType::sendNotification);
    setElevationDampeningEnabled(state);
}

//==============================================================================
void SectionAbstractTrajectories::setPositionDampeningEnabled(bool const state)
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
void SectionAbstractTrajectories::setElevationDampeningEnabled(bool const state)
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
void SectionAbstractTrajectories::setPositionDampeningCycles(int const value)
{
    mPositionDampeningEditor.setText(juce::String(value));
}

//==============================================================================
void SectionAbstractTrajectories::setElevationDampeningCycles(int const value)
{
    mElevationDampeningEditor.setText(juce::String(value));
}

//==============================================================================
void SectionAbstractTrajectories::setDeviationPerCycle(float const value)
{
    mDeviationEditor.setText(juce::String(value));
}

//==============================================================================
void SectionAbstractTrajectories::setPositionActivateState(bool const state)
{
    mPositionActivateButton.setToggleState(state, juce::NotificationType::sendNotification);
}

//==============================================================================
void SectionAbstractTrajectories::setElevationActivateState(bool const state)
{
    mElevationActivateButton.setToggleState(state, juce::NotificationType::sendNotification);
}

//==============================================================================
void SectionAbstractTrajectories::setSpeedLinkState(bool state)
{
    mSpeedLinked = state;
    repaint();
}

//==============================================================================
void SectionAbstractTrajectories::updatePositionSpeedSliderVal(float value)
{
    mPositionCycleSpeedSlider.setValue(value, juce::sendNotification);
}

//==============================================================================
void SectionAbstractTrajectories::updateElevationSpeedSliderVal(float value)
{
    mElevationCycleSpeedSlider.setValue(value, juce::sendNotification);
}

//==============================================================================
void SectionAbstractTrajectories::positionSpeedSliderChangedStartedCallback()
{
    mProcessor.getChangeGestureManager().beginGesture(Automation::Ids::POSITION_SPEED_SLIDER);
}

//==============================================================================
void SectionAbstractTrajectories::positionSpeedSliderChangedEndedCallback()
{
    mProcessor.getChangeGestureManager().endGesture(Automation::Ids::POSITION_SPEED_SLIDER);
}

//==============================================================================
void SectionAbstractTrajectories::elevationSpeedSliderChangedStartedCallback()
{
    mProcessor.getChangeGestureManager().beginGesture(Automation::Ids::ELEVATION_SPEED_SLIDER);
}

//==============================================================================
void SectionAbstractTrajectories::elevationSpeedSliderChangedEndedCallback()
{
    mProcessor.getChangeGestureManager().endGesture(Automation::Ids::ELEVATION_SPEED_SLIDER);
}

//==============================================================================
void SectionAbstractTrajectories::updateAbstractTrajectoriesPositionActivate(float value)
{
    auto const state{ value != 0.0f };
    if (mPositionActivateButton.isShowing()) {
        mPositionActivateButton.setToggleState(state, juce::sendNotification);
    }
}

//==============================================================================
void SectionAbstractTrajectories::updateAbstractTrajectoriesElevationActivate(float value)
{
    auto const state{ value != 0.0f };
    if (mElevationActivateButton.isShowing()) {
        mElevationActivateButton.setToggleState(state, juce::sendNotification);
    }
}

//==============================================================================
void SectionAbstractTrajectories::positionActivateButtonChangedStartedCallback()
{
    mProcessor.getChangeGestureManager().beginGesture(Automation::Ids::ABSTRACT_TRAJECTORIES_POSITION_ACTIVATE);
}

//==============================================================================
void SectionAbstractTrajectories::positionActivateButtonChangedEndedCallback()
{
    mProcessor.getChangeGestureManager().endGesture(Automation::Ids::ABSTRACT_TRAJECTORIES_POSITION_ACTIVATE);
}

//==============================================================================
void SectionAbstractTrajectories::elevationActivateButtonChangedStartedCallback()
{
    mProcessor.getChangeGestureManager().beginGesture(Automation::Ids::ABSTRACT_TRAJECTORIES_ELEVATION_ACTIVATE);
}

//==============================================================================
void SectionAbstractTrajectories::elevationActivateButtonChangedEndedCallback()
{
    mProcessor.getChangeGestureManager().endGesture(Automation::Ids::ABSTRACT_TRAJECTORIES_ELEVATION_ACTIVATE);
}

//==============================================================================
void SectionAbstractTrajectories::setCycleDuration(double const value)
{
    mDurationEditor.setText(juce::String(value));
    mListeners.call([&](Listener & l) {
        l.trajectoryCycleDurationChangedCallback(mDurationEditor.getText().getDoubleValue(),
                                                 mDurationUnitCombo.getSelectedId());
    });
}

//==============================================================================
void SectionAbstractTrajectories::setDurationUnit(int const value)
{
    mDurationUnitCombo.setSelectedId(value, juce::NotificationType::sendNotificationSync);
    mListeners.call([&](Listener & l) {
        l.trajectoryDurationUnitChangedCallback(mDurationEditor.getText().getDoubleValue(),
                                                mDurationUnitCombo.getSelectedId());
    });
}

//==============================================================================
void SectionAbstractTrajectories::mouseDown(juce::MouseEvent const & event)
{
    if (mSpatMode == SpatMode::cube) {
        // Area where the speedLinked arrow is shown.
        juce::Rectangle<float> const speedLinkedArrowArea{ 292.0f, 70.0f, 30.0f, 17.0f };
        if (speedLinkedArrowArea.contains(event.getMouseDownPosition().toFloat())) {
            mSpeedLinked = !mSpeedLinked;
            repaint();
        }

        // Area of the Activate arrow
        juce::Rectangle<float> const activateLinkedArea{ 292.0f,
                                                         static_cast<float>(mPositionActivateButton.getBounds().getY()),
                                                         30.0f,
                                                         17.0f };
        if (activateLinkedArea.contains(event.getMouseDownPosition().toFloat())) {
            mActivateLinked = !mActivateLinked;
            repaint();
        }

        // resetting elevation cycle speed slider
        auto eleCycleSpeedSliderBounds{ mElevationCycleSpeedSlider.getBounds() };
        juce::Path pathEle;
        pathEle.addRectangle(static_cast<float>(eleCycleSpeedSliderBounds.getCentreX() - 5),
                             static_cast<float>(eleCycleSpeedSliderBounds.getY() - 7),
                             10,
                             7);
        if (pathEle.contains(event.getMouseDownPosition().toFloat())) {
            mElevationCycleSpeedSlider.setValue(0.5, juce::sendNotification);
        }
    }
    // resetting position cycle speed slider
    auto posCycleSpeedSliderBounds{ mPositionCycleSpeedSlider.getBounds() };
    juce::Path pathPos;
    pathPos.addRectangle(static_cast<float>(posCycleSpeedSliderBounds.getCentreX() - 5),
                         static_cast<float>(posCycleSpeedSliderBounds.getY() - 7),
                         10,
                         7);
    if (pathPos.contains(event.getMouseDownPosition().toFloat())) {
        mPositionCycleSpeedSlider.setValue(0.5, juce::sendNotification);
    }
}

//==============================================================================
void SectionAbstractTrajectories::paint(juce::Graphics & g)
{
    if (mSpatMode == SpatMode::cube) {
        if (mSpeedLinked)
            g.setColour(juce::Colours::orange);
        else
            g.setColour(juce::Colours::black);
        g.drawArrow(juce::Line<float>(302.0f, 78.0f, 292.0f, 78.0f), 4, 10, 7);
        g.drawArrow(juce::Line<float>(297.0f, 78.0f, 317.0f, 78.0f), 4, 10, 7);

        if (mActivateLinked)
            g.setColour(juce::Colours::orange);
        else
            g.setColour(juce::Colours::black);
        auto activateArrowY{ mPositionActivateButton.getBounds().getY() + mPositionActivateButton.getHeight() / 2 };
        g.drawArrow(
            juce::Line<float>(302.0f, static_cast<float>(activateArrowY), 292.0f, static_cast<float>(activateArrowY)),
            4.0f,
            10.0f,
            7.0f);
        g.drawArrow(
            juce::Line<float>(297.0f, static_cast<float>(activateArrowY), 317.0f, static_cast<float>(activateArrowY)),
            4.0f,
            10.0f,
            7.0f);

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

        juce::Path pathEle;
        pathEle.addTriangle({ static_cast<float>(eleCycleSpeedSliderBounds.getCentreX() - 3),
                              static_cast<float>(eleCycleSpeedSliderBounds.getY() - 5) },
                            { static_cast<float>(eleCycleSpeedSliderBounds.getCentreX() + 3),
                              static_cast<float>(eleCycleSpeedSliderBounds.getY() - 5) },
                            { static_cast<float>(eleCycleSpeedSliderBounds.getCentreX()),
                              static_cast<float>(eleCycleSpeedSliderBounds.getY() - 1) });
        if (mElevationCycleSpeedSlider.getValue() == mElevationCycleSpeedSlider.getRange().getLength() / 2) {
            g.setColour(juce::Colours::lightgreen);
        } else {
            g.setColour(mGrisLookAndFeel.getDarkColor());
        }
        g.fillPath(pathEle);
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

    juce::Path pathPos;
    pathPos.addTriangle({ static_cast<float>(posCycleSpeedSliderBounds.getCentreX() - 3),
                    static_cast<float>(posCycleSpeedSliderBounds.getY() - 5) },
                  { static_cast<float>(posCycleSpeedSliderBounds.getCentreX() + 3),
                    static_cast<float>(posCycleSpeedSliderBounds.getY() - 5) },
                  { static_cast<float>(posCycleSpeedSliderBounds.getCentreX()),
                    static_cast<float>(posCycleSpeedSliderBounds.getY() - 1) });
    if (mPositionCycleSpeedSlider.getValue() == mPositionCycleSpeedSlider.getRange().getLength() / 2) {
        g.setColour(juce::Colours::lightgreen);
    } else {
        g.setColour(mGrisLookAndFeel.getDarkColor());
    }
    g.fillPath(pathPos);
}

//==============================================================================
void SectionAbstractTrajectories::resized()
{
    mTrajectoryTypeLabel.setBounds(5, 7, 150, 10);
    mTrajectoryTypeXYLabel.setBounds(90, 7, 150, 10);
    mPositionTrajectoryTypeCombo.setBounds(115, 5, 175, 15);
    mPositionBackAndForthToggle.setBounds(196, 27, 94, 15);
    mDampeningLabel.setBounds(5, 18, 150, 22);
    mDampeningLabel2ndLine.setBounds(5, 26, 150, 22);
    mPositionDampeningEditor.setBounds(115, 27, 75, 15);
    mDeviationLabel.setBounds(110, 40, 90, 22);
    mDeviationLabel2ndLine.setBounds(110, 48, 90, 22);
    mDeviationEditor.setBounds(211, 49, 78, 15);
    mCycleSpeedLabel.setBounds(5, 71, 100, 22);
    mPositionCycleSpeedSlider.setBounds(113, 72, 180, 16);

    mRandomXYToggle.setBounds(112, 92, 60, 15);
    mRandomXYLabel.setBounds(124, 95, 150, 10);
    mRandomTypeXYCombo.setBounds(175, 92, 78, 15);

    mRandomXYLoopButton.setBounds(257, 92, 32, 15);
    mRandomStartXYButton.setBounds(249, 121, 40, 14);

    mRandomProximityXYLabel.setBounds(117, 110, 60, 10);
    mRandomTimeMinXYLabel.setBounds(158, 110, 60, 10);
    mRandomTimeMaxXYLabel.setBounds(201, 110, 60, 10);
    mRandomStartXYLabel.setBounds(248, 110, 60, 10);

    mRandomProximityXYSlider.setBounds(116, 122, 35, 12);
    mRandomTimeMinXYSlider.setBounds(161, 122, 35, 12);
    mRandomTimeMaxXYSlider.setBounds(206, 122, 35, 12);

    mPositionActivateButton.setBounds(114, 138, 176, 20);

    mTrajectoryTypeZLabel.setBounds(303, 7, 150, 10);
    mElevationTrajectoryTypeCombo.setBounds(320, 5, 175, 15);
    mElevationDampeningEditor.setBounds(320, 27, 75, 15);
    mElevationBackAndForthToggle.setBounds(401, 27, 94, 15);

    mElevationCycleSpeedSlider.setBounds(317, 72, 180, 16);

    auto const xOffsetForZRandom{ 205 };
    mRandomZToggle.setBounds(112 + xOffsetForZRandom, 92, 60, 15);
    mRandomZLabel.setBounds(124 + xOffsetForZRandom, 95, 150, 10);
    mRandomTypeZCombo.setBounds(175 + xOffsetForZRandom, 92, 78, 15);

    mRandomZLoopButton.setBounds(257 + xOffsetForZRandom, 92, 32, 15);
    mRandomStartZButton.setBounds(249 + xOffsetForZRandom, 121, 40, 14);

    mRandomProximityZLabel.setBounds(116 + xOffsetForZRandom, 110, 60, 10);
    mRandomTimeMinZLabel.setBounds(158 + xOffsetForZRandom, 110, 60, 10);
    mRandomTimeMaxZLabel.setBounds(201 + xOffsetForZRandom, 110, 60, 10);
    mRandomStartZLabel.setBounds(248 + xOffsetForZRandom, 110, 60, 10);

    mRandomProximityZSlider.setBounds(116 + xOffsetForZRandom, 122, 35, 12);
    mRandomTimeMinZSlider.setBounds(161 + xOffsetForZRandom, 122, 35, 12);
    mRandomTimeMaxZSlider.setBounds(206 + xOffsetForZRandom, 122, 35, 12);

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
        mRandomStartZLabel.setVisible(true);
        mRandomProximityZSlider.setVisible(true);
        mRandomTimeMinZSlider.setVisible(true);
        mRandomTimeMaxZSlider.setVisible(true);
        mRandomStartZButton.setVisible(true);
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
        mRandomStartZLabel.setVisible(false);
        mRandomProximityZSlider.setVisible(false);
        mRandomTimeMinZSlider.setVisible(false);
        mRandomTimeMaxZSlider.setVisible(false);
        mRandomStartZButton.setVisible(false);
        mElevationCycleSpeedSlider.setVisible(false);
    }

    mDurationLabel.setBounds(495, 5, 90, 20);
    mDurationEditor.setBounds(500, 30, 90, 20);
    mDurationUnitCombo.setBounds(500, 60, 90, 20);
}

//==============================================================================
void SectionAbstractTrajectories::timerCallback()
{
    if (!(mProcessor.isPlaying())) {
        bool positionActivateButtonAlwaysOn{ mAPVTS.state.getProperty("positionActivateButtonAlwaysOn") };
        bool elevationActivateButtonAlwaysOn{ mAPVTS.state.getProperty("elevavtionActivateButtonAlwaysOn") };

        if (positionActivateButtonAlwaysOn) {
            mListeners.call([&](Listener & l) {
                l.positionTrajectoryStateChangedCallback(mPositionActivateButton.getToggleState());
            });
        }
        if (elevationActivateButtonAlwaysOn) {
            mListeners.call([&](Listener & l) {
                l.elevationTrajectoryStateChangedCallback(mElevationActivateButton.getToggleState());
            });
        }
    }
}

} // namespace gris
