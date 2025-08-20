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

#include "cg_SectionPositionPresets.hpp"

#include "cg_PresetsManager.hpp"

namespace gris
{
//==============================================================================
void PresetButton::setSavedState(bool const savedState)
{
    mSaved = savedState;
    refresh();
}

//==============================================================================
void PresetButton::setLoadedState(bool const loadedState)
{
    mLoaded = loadedState;
    setToggleState(loadedState, juce::NotificationType::dontSendNotification);
    refresh();
}

//==============================================================================
void PresetButton::refresh()
{
    if (mLoaded) {
        setColour(TextButton::buttonColourId, juce::Colour::fromRGB(255, 165, 25));
    } else {
        if (mSaved) {
            setColour(TextButton::buttonColourId, juce::Colour::fromRGB(255, 255, 255));
        } else {
            setColour(TextButton::buttonColourId, juce::Colour::fromRGB(172, 172, 172));
        }
    }
}

//==============================================================================
void PresetButton::clicked(juce::ModifierKeys const & mods)
{
    mListeners.call([&](Listener & l) { l.presetButtonClickedCallback(this); });
    if (!mods.isShiftDown()) {
        mLoaded = getToggleState();
    }
    refresh();
}

//==============================================================================
void PresetButton::internalClickCallback(juce::ModifierKeys const & mods)
{
    if (mods.isShiftDown()) {
        mListeners.call([&](Listener & l) { l.savingPresetClickedCallback(this); });
        mSaved = true;
    } else if (mSaved && mods.isAltDown()) {
        mListeners.call([&](Listener & l) { l.deletingPresetClickedCallback(this); });
        setToggleState(false, juce::NotificationType::dontSendNotification);
        mLoaded = mSaved = false;
    } else if (mSaved) {
        TextButton::internalClickCallback(mods); //this will cause PresetButton::clicked() to be called right above
    }
    refresh();
}

//===============================================================================
PositionPresetComponent::PositionPresetComponent(PresetsManager & presetsManager,
                                                 PositionPresetInfoComponent & positionPresetStateComponent)
    : mPositionPresetStateComponent(positionPresetStateComponent)
    , mPresetsManager(presetsManager)
{
    setName("PositionPresetComponent");

    constexpr int groupId = 1;

    for (int i{}; i < NUMBER_OF_POSITION_PRESETS; ++i) {
        auto button{ std::make_unique<PresetButton>() };

        button->setRadioGroupId(groupId);
        button->setClickingTogglesState(true);
        button->setButtonText(juce::String(i + 1));
        button->addListener(this);

        addAndMakeVisible(button.get());

        mPresets.add(button.release());
    }

    mPresetsManager.addChangeListener(this);
}

//==============================================================================
PositionPresetComponent::~PositionPresetComponent()
{
    mPresetsManager.removeChangeListener(this);
}

//==============================================================================
void PositionPresetComponent::setPreset(int const value, bool const notify)
{
    if (mCurrentSelection >= 0) {
        mPresets[mCurrentSelection]->setLoadedState(false);
    }

    if (value == 0) {
        return;
    }

    if (mPresets[value - 1]->isSaved()) {
        mPresets[value - 1]->setToggleState(true, juce::NotificationType::dontSendNotification);
        mCurrentSelection = value - 1;
        if (notify) {
            presetButtonClickedCallback(mPresets[value - 1]);
        }
    }
}

//==============================================================================
void PositionPresetComponent::presetSaved(int const presetNumber, bool const isSaved) const
{
    if (mPresets[presetNumber - 1] != nullptr) {
        mPresets[presetNumber - 1]->setSavedState(isSaved);
    }
}

//==============================================================================
void PositionPresetComponent::changeListenerCallback([[maybe_unused]] juce::ChangeBroadcaster * changeBroadcaster)
{
    auto const currentPreset{ mPresetsManager.getCurrentPreset() };
    setPreset(currentPreset, false);
}

//==============================================================================
void PositionPresetComponent::presetButtonClickedCallback(PresetButton * button)
{
    if (button->getToggleState()) {
        mCurrentSelection = button->getButtonText().getIntValue() - 1;
        mPositionPresetStateComponent.setActionLogText(juce::String("Load ") + button->getButtonText(),
                                                       juce::NotificationType::dontSendNotification);
        mListeners.call([&](Listener & l) { l.positionPresetChangedCallback(button->getButtonText().getIntValue()); });
    }
}

//==============================================================================
void PositionPresetComponent::savingPresetClickedCallback(PresetButton * button)
{
    mPositionPresetStateComponent.setActionLogText(juce::String("Save ") + button->getButtonText(),
                                                   juce::NotificationType::dontSendNotification);
    mListeners.call([&](Listener & l) { l.positionPresetSavedCallback(button->getButtonText().getIntValue()); });
}

//==============================================================================
void PositionPresetComponent::deletingPresetClickedCallback(PresetButton * button)
{
    mPositionPresetStateComponent.setActionLogText(juce::String("Del ") + button->getButtonText(),
                                                   juce::NotificationType::dontSendNotification);
    mListeners.call([&](Listener & l) { l.positionPresetDeletedCallback(button->getButtonText().getIntValue()); });
}

//==============================================================================
void PositionPresetComponent::resized()
{
    for (int i{}; i < NUMBER_OF_POSITION_PRESETS; ++i) {
        auto const x{ i % 2 == 0 ? 1 : 25 };
        auto const y{ (i / 2) * 24 + 1 };
        mPresets[i]->setBounds(x, y, 23, 23);
    }
}

//==============================================================================
PositionPresetInfoComponent::PositionPresetInfoComponent(GrisLookAndFeel & grisLookAndFeel)
    : mGrisLookAndFeel(grisLookAndFeel)
{
    mActionLog.setColour(juce::Label::backgroundColourId, mGrisLookAndFeel.getGreyColor());
    mActionLog.setColour(juce::Label::outlineColourId, juce::Colour::fromRGB(0, 0, 0));
    mActionLog.setColour(juce::Label::textColourId, juce::Colour::fromRGB(0, 0, 0));
    addAndMakeVisible(&mActionLog);

    addAndMakeVisible(&mAppVersionLabel);
}

//==============================================================================
void PositionPresetInfoComponent::resized()
{
    mActionLog.setBounds(1, 2, 47, 20);
    mAppVersionLabel.setBounds(3, 22, 47, 20);
}

//==============================================================================
void PositionPresetInfoComponent::timerCallback()
{
    if (mTimerFlashCounter > 0) {
        if (mTimerFlashCounter % 2 == 0) {
            mActionLog.setColour(juce::Label::backgroundColourId, mGrisLookAndFeel.getOnColor());
        } else {
            mActionLog.setColour(juce::Label::backgroundColourId, mGrisLookAndFeel.getGreyColor());
        }
        --mTimerFlashCounter;
    } else {
        stopTimer();
        resetTimer();
    }
}

//==============================================================================
void PositionPresetInfoComponent::setActionLogText(juce::String const & text, juce::NotificationType notificationType)
{
    mActionLog.setText(text, notificationType);
    stopTimer();
    resetTimer();
    startTimer(500);
    timerCallback();
}

//==============================================================================
void PositionPresetInfoComponent::setAppVersionLabelText(juce::String const & text,
                                                         juce::NotificationType notificationType)
{
    mAppVersionLabel.setText(text, notificationType);
}

void PositionPresetInfoComponent::resetTimer()
{
    mTimerFlashCounter = 6;
}

} // namespace gris
