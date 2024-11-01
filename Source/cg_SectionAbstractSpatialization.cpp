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
namespace {
    auto constexpr SPEED_SLIDER_MIN_VAL{ 0.1 };
    auto constexpr SPEED_SLIDER_MAX_VAL{ 10.0 };
    auto constexpr SPEED_SLIDER_MID_VAL{ 1.0 };
}

//==============================================================================
SectionAbstractSpatialization::SectionAbstractSpatialization(GrisLookAndFeel & grisLookAndFeel) : mGrisLookAndFeel(grisLookAndFeel)
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

    mPositionCycleSpeedLabel.setText("Cycle Speed:", juce::NotificationType::dontSendNotification);
    addAndMakeVisible(&mPositionCycleSpeedLabel);

    mPositionCycleSpeedSlider.setNormalisableRange(juce::NormalisableRange<double>(0.0, 1.0, 0.01));
    mPositionCycleSpeedSlider.setDoubleClickReturnValue(true, 0.5);
    mPositionCycleSpeedSlider.setValue(0.5, juce::NotificationType::sendNotificationAsync);
    mPositionCycleSpeedSlider.setSliderSnapsToMousePosition(false);
    mPositionCycleSpeedSlider.setTextBoxStyle(juce::Slider::TextBoxRight, false, 40, 20);
    mPositionCycleSpeedSlider.setColour(juce::Slider::textBoxOutlineColourId, juce::Colours::transparentBlack);
    addAndMakeVisible(&mPositionCycleSpeedSlider);
    mPositionCycleSpeedSlider.onValueChange = [this] {
        auto const sliderVal{ mPositionCycleSpeedSlider.getValue() };
        double speedMultToSend{};
        if (sliderVal <= 0.5) {
            speedMultToSend = juce::jmap(sliderVal, 0.0, 0.5, SPEED_SLIDER_MIN_VAL, SPEED_SLIDER_MID_VAL);
        } else {
            speedMultToSend = juce::jmap(sliderVal, 0.5, 1.0, SPEED_SLIDER_MID_VAL, SPEED_SLIDER_MAX_VAL);
        }
        mListeners.call(
                        [&](Listener & l) { l.positionTrajectoryCurrentSpeedChangedCallback(speedMultToSend); });
    };
    
    mElevationCycleSpeedSlider.setNormalisableRange(juce::NormalisableRange<double>(0.0, 1.0, 0.01));
    mElevationCycleSpeedSlider.setDoubleClickReturnValue(true, 0.5);
    mElevationCycleSpeedSlider.setValue(0.5, juce::NotificationType::sendNotificationAsync);
    mElevationCycleSpeedSlider.setSliderSnapsToMousePosition(false);
    mElevationCycleSpeedSlider.setTextBoxStyle(juce::Slider::TextBoxRight, false, 40, 20);
    mElevationCycleSpeedSlider.setColour(juce::Slider::textBoxOutlineColourId, juce::Colours::transparentBlack);
    addAndMakeVisible(&mElevationCycleSpeedSlider);
    mElevationCycleSpeedSlider.onValueChange = [this] {
        auto const sliderVal{ mElevationCycleSpeedSlider.getValue() };
        double speedMultToSend{};
        if (sliderVal <= 0.5) {
            speedMultToSend = juce::jmap(sliderVal, 0.0, 0.5, SPEED_SLIDER_MIN_VAL, SPEED_SLIDER_MID_VAL);
        } else {
            speedMultToSend = juce::jmap(sliderVal, 0.5, 1.0, SPEED_SLIDER_MID_VAL, SPEED_SLIDER_MAX_VAL);
        }
        mListeners.call(
                        [&](Listener & l) { l.elevationTrajectoryCurrentSpeedChangedCallback(speedMultToSend); });
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

    // temporary GUI elements
    mSpeedXYLabel.setText("Speed", juce::dontSendNotification);
    addAndMakeVisible(&mSpeedXYLabel);

    addAndMakeVisible(&mSpeedXYEditor);
    mSpeedXYEditor.addListener(this);
    mSpeedXYEditor.setFont(mGrisLookAndFeel.getFont());
    mSpeedXYEditor.setTextToShowWhenEmpty("1.0", juce::Colours::white);
    mSpeedXYEditor.setText("1.0", false);
    mSpeedXYEditor.setInputRestrictions(4, "0123456789.");

    mRandomXYLabel.setText("Random", juce::dontSendNotification);
    addAndMakeVisible(&mRandomXYLabel);

    addAndMakeVisible(&mRandomXYToggle);

    addAndMakeVisible(&mRandomXYCombo);
    mRandomXYCombo.addItem("Sine", 1);
    mRandomXYCombo.addItem("Square", 2);
    mRandomXYCombo.setSelectedId(1);

    mSpeedZLabel.setText("Speed", juce::dontSendNotification);
    addAndMakeVisible(&mSpeedZLabel);

    addAndMakeVisible(&mSpeedZEditor);
    mSpeedZEditor.addListener(this);
    mSpeedZEditor.setFont(mGrisLookAndFeel.getFont());
    mSpeedZEditor.setTextToShowWhenEmpty("1.0", juce::Colours::white);
    mSpeedZEditor.setText("1.0", false);
    mSpeedZEditor.setInputRestrictions(4, "0123456789.");

    mRandomZLabel.setText("Random", juce::dontSendNotification);
    addAndMakeVisible(&mRandomZLabel);

    addAndMakeVisible(&mRandomZToggle);

    addAndMakeVisible(&mRandomZCombo);
    mRandomZCombo.addItem("Sine", 1);
    mRandomZCombo.addItem("Square", 2);
    mRandomZCombo.setSelectedId(1);

    mSpeedZLabel.setEnabled(false);
    mSpeedZEditor.setEnabled(false);
    mRandomZLabel.setEnabled(false);
    mRandomZToggle.setEnabled(false);
    mRandomZCombo.setEnabled(false);

    mSpeedXYLabel.setEnabled(false);
    mSpeedXYEditor.setEnabled(false);
    mRandomXYLabel.setEnabled(false);
    mRandomXYToggle.setEnabled(false);
    mRandomXYCombo.setEnabled(false);
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
void SectionAbstractSpatialization::textEditorReturnKeyPressed(juce::TextEditor & textEd)
{
//    const auto value{ textEd.getText().getDoubleValue() };
    const auto paramName{ textEd.getName() };

    //mListeners.call([&](Listener & l) { l.speedStateChangedCallback(value); }); // TODO: add a parameter to modify
    // AudioProcessorValueTreeState

    if (mSpeedLinked) {
        if (&textEd == &mSpeedXYEditor) {
            mSpeedZEditor.setText(textEd.getText());
            // mListeners.call([&](Listener & l) { l.speedStateChangedCallback(value); }); // TODO: call parameter
            // Z AudioProcessorValueTreeState
        } else if (&textEd == &mSpeedZEditor) {
            mSpeedXYEditor.setText(textEd.getText());
            // mListeners.call([&](Listener & l) { l.speedStateChangedCallback(value); }); // TODO: call parameter
            // XY AudioProcessorValueTreeState
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
    }
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

    mSpeedXYLabel.setBounds(110, 72, 150, 10);
    mSpeedXYEditor.setBounds(211, 70, 78, 15);
    mRandomXYLabel.setBounds(110, 93, 150, 10);
    mRandomXYToggle.setBounds(181, 92, 88, 15);
    mRandomXYCombo.setBounds(211, 92, 78, 15);

    mPositionActivateButton.setBounds(114, 138, 176, 20);

    mTrajectoryTypeZLabel.setBounds(303, 7, 150, 10);
    mElevationTrajectoryTypeCombo.setBounds(320, 5, 175, 15);
    mElevationDampeningEditor.setBounds(320, 27, 75, 15);
    mElevationBackAndForthToggle.setBounds(401, 27, 94, 15);

    mSpeedZLabel.setBounds(315, 72, 150, 10);
    mSpeedZEditor.setBounds(416, 70, 78, 15);
    mRandomZLabel.setBounds(315, 93, 150, 10);
    mRandomZToggle.setBounds(386, 92, 88, 15);
    mRandomZCombo.setBounds(416, 92, 78, 15);

    mElevationActivateButton.setBounds(319, 138, 176, 20);

    if (mSpatMode == SpatMode::cube) {
        mTrajectoryTypeXYLabel.setVisible(true);
        mTrajectoryTypeZLabel.setVisible(true);
        mElevationTrajectoryTypeCombo.setVisible(true);
        mElevationActivateButton.setVisible(true);
        mElevationBackAndForthToggle.setVisible(true);
        mElevationDampeningEditor.setVisible(true);
        mSpeedZLabel.setVisible(true);
        mSpeedZEditor.setVisible(true);
        mRandomZLabel.setVisible(true);
        mRandomZToggle.setVisible(true);
        mRandomZCombo.setVisible(true);
        mElevationCycleSpeedSlider.setVisible(true);
    } else {
        mTrajectoryTypeXYLabel.setVisible(false);
        mTrajectoryTypeZLabel.setVisible(false);
        mElevationTrajectoryTypeCombo.setVisible(false);
        mElevationActivateButton.setVisible(false);
        mElevationBackAndForthToggle.setVisible(false);
        mElevationDampeningEditor.setVisible(false);
        mSpeedZLabel.setVisible(false);
        mSpeedZEditor.setVisible(false);
        mRandomZLabel.setVisible(false);
        mRandomZToggle.setVisible(false);
        mRandomZCombo.setVisible(false);
        mElevationCycleSpeedSlider.setVisible(false);
    }

    mDurationLabel.setBounds(495, 5, 90, 20);
    mDurationEditor.setBounds(500, 30, 90, 20);
    mDurationUnitCombo.setBounds(500, 60, 90, 20);

    // Hide Cycle Speed slider until we found the good way to handle it!
    mPositionCycleSpeedLabel.setBounds(5, 100, 150, 20);
    mPositionCycleSpeedSlider.setBounds(114, 114, 165, 20);
    mPositionCycleSpeedLabel.setVisible(false);
    mPositionCycleSpeedSlider.setVisible(true);
    
    mElevationCycleSpeedSlider.setBounds(320, 114, 165, 20);
}

} // namespace gris
