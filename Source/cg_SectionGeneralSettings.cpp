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

#include "cg_SectionGeneralSettings.hpp"

namespace gris
{
//==============================================================================
/**
 * @class NumberRangeInputFilter
 * @brief A filter to restrict text input to a specified numeric range.
 */
class NumberRangeInputFilter : public juce::TextEditor::InputFilter
{
public:
    NumberRangeInputFilter(int _minValue, int _maxValue) : minValue(_minValue), maxValue(_maxValue) {}

    /**
     * @brief Filters the new text input to ensure it falls within the specified range.
     * @param editor The text editor where the input is being entered.
     * @param newInput The new text input.
     * @return The filtered text input.
     */
    juce::String filterNewText(juce::TextEditor & editor, const juce::String & newInput) override
    {
        const auto currentText{ editor.getText() };
        const auto newText{ currentText + newInput };

        if (newText.isEmpty())
            return newText;

        const auto value{ newText.getIntValue() };
        if (value >= minValue && value <= maxValue)
            return newInput;

        return {};
    }

private:
    int minValue;
    int maxValue;
};

//==============================================================================
SectionGeneralSettings::SectionGeneralSettings(GrisLookAndFeel & grisLookAndFeel) : mGrisLookAndFeel(grisLookAndFeel)
{
    mOscFormatLabel.setText("Mode:", juce::NotificationType::dontSendNotification);
    addAndMakeVisible(mOscFormatLabel);

    mOscFormatCombo.addItem("DOME - SpatGris", 1);
    mOscFormatCombo.addItem("CUBE - SpatGris", 2);
    mOscFormatCombo.onChange = [this] {
        mListeners.call([&](Listener & l) {
            l.oscFormatChangedCallback(static_cast<SpatMode>(mOscFormatCombo.getSelectedId() - 1));
        });
    };
    mOscFormatCombo.setSelectedId(1);
    addAndMakeVisible(mOscFormatCombo);

    mOscPortLabel.setText("OSC Port:", juce::NotificationType::dontSendNotification);
    addAndMakeVisible(mOscPortLabel);

    juce::String defaultPort("18032");
    mOscPortEditor.setExplicitFocusOrder(4);
    mOscPortEditor.setText(defaultPort);
    mOscPortEditor.setInputRestrictions(5, "0123456789");
    mOscPortEditor.onReturnKey = [this] { mOscFormatCombo.grabKeyboardFocus(); };
    mOscPortEditor.onFocusLost = [this, defaultPort] {
        if (!mOscPortEditor.isEmpty()) {
            mListeners.call([&](Listener & l) { l.oscPortChangedCallback(mOscPortEditor.getText().getIntValue()); });
        } else {
            mListeners.call([&](Listener & l) {
                l.oscPortChangedCallback(defaultPort.getIntValue());
                mOscPortEditor.setText(defaultPort);
            });
        }
    };
    addAndMakeVisible(mOscPortEditor);

    mOscAddressLabel.setText("IP Address:", juce::NotificationType::dontSendNotification);
    addAndMakeVisible(mOscAddressLabel);

    juce::String const defaultAddress{ "127.0.0.1" };
    mOscAddressEditor.setExplicitFocusOrder(5);
    mOscAddressEditor.setText(defaultAddress);
    mOscAddressEditor.setInputRestrictions(15, "0123456789.");
    mOscAddressEditor.onReturnKey = [this]() -> void { mOscFormatCombo.grabKeyboardFocus(); };
    mOscAddressEditor.onFocusLost = [this, defaultAddress]() -> void {
        if (!mOscAddressEditor.isEmpty()) {
            mListeners.call([&](Listener & l) { l.oscAddressChangedCallback(mOscAddressEditor.getText()); });
        } else {
            mListeners.call([&](Listener & l) {
                l.oscAddressChangedCallback(defaultAddress);
                mOscAddressEditor.setText(defaultAddress);
            });
        }
    };
    addAndMakeVisible(mOscAddressEditor);

    mNumOfSourcesLabel.setText("Number of Sources:", juce::NotificationType::dontSendNotification);
    addAndMakeVisible(&mNumOfSourcesLabel);

    mNumOfSourcesEditor.setExplicitFocusOrder(2);
    mNumOfSourcesEditor.setText("2");
    mNumOfSourcesEditor.setInputFilter(new NumberRangeInputFilter(1, gris::MAX_NUMBER_OF_SOURCES), true);

    mNumOfSourcesEditor.onReturnKey = [this] { mOscFormatCombo.grabKeyboardFocus(); };
    mNumOfSourcesEditor.onFocusLost = [this] {
        if (!mNumOfSourcesEditor.isEmpty()) {
            mListeners.call(
                [&](Listener & l) { l.numberOfSourcesChangedCallback(mNumOfSourcesEditor.getText().getIntValue()); });
        } else {
            mListeners.call([&](Listener & l) {
                l.numberOfSourcesChangedCallback(1);
                mNumOfSourcesEditor.setText("1");
            });
        }
    };
    addAndMakeVisible(&mNumOfSourcesEditor);

    mFirstSourceIdLabel.setText("First Source ID:", juce::NotificationType::dontSendNotification);
    addAndMakeVisible(&mFirstSourceIdLabel);

    mFirstSourceIdEditor.setExplicitFocusOrder(3);
    mFirstSourceIdEditor.setText("1");
    mFirstSourceIdEditor.setInputRestrictions(3, "0123456789");
    mFirstSourceIdEditor.onReturnKey = [this] { mOscFormatCombo.grabKeyboardFocus(); };
    mFirstSourceIdEditor.onFocusLost = [this] {
        if (!mFirstSourceIdEditor.isEmpty()) {
            mListeners.call([&](Listener & l) {
                l.firstSourceIdChangedCallback(SourceId{ mFirstSourceIdEditor.getText().getIntValue() });
            });
        } else {
            mListeners.call([&](Listener & l) {
                l.firstSourceIdChangedCallback(SourceId{ 1 });
                mFirstSourceIdEditor.setText("1");
            });
        }
    };
    addAndMakeVisible(&mFirstSourceIdEditor);

    mPositionActivateButton.setExplicitFocusOrder(1);
    mPositionActivateButton.setButtonText("Activate OSC");
    mPositionActivateButton.onClick = [this] {
        mListeners.call([&](Listener & l) { l.oscStateChangedCallback(mPositionActivateButton.getToggleState()); });
    };
    addAndMakeVisible(&mPositionActivateButton);
}

//==============================================================================
void SectionGeneralSettings::setOscFormat(SpatMode mode)
{
    mOscFormatCombo.setSelectedId(static_cast<int>(mode) + 1, juce::NotificationType::dontSendNotification);
}

//==============================================================================
void SectionGeneralSettings::setOscPortNumber(int const oscPortNumber)
{
    mOscPortEditor.setText(juce::String(oscPortNumber));
}

//==============================================================================
void SectionGeneralSettings::setOscAddress(juce::String const & address)
{
    mOscAddressEditor.setText(address);
}

//==============================================================================
void SectionGeneralSettings::setNumberOfSources(int const numOfSources)
{
    mNumOfSourcesEditor.setText(juce::String(numOfSources));
}

//==============================================================================
void SectionGeneralSettings::setFirstSourceId(SourceId const firstSourceId)
{
    mFirstSourceIdEditor.setText(firstSourceId.toString());
}

//==============================================================================
void SectionGeneralSettings::setActivateButtonState(bool const shouldBeOn)
{
    mPositionActivateButton.setToggleState(shouldBeOn, juce::NotificationType::dontSendNotification);
}

//==============================================================================
void SectionGeneralSettings::paint(juce::Graphics & g)
{
    g.fillAll(mGrisLookAndFeel.findColour(juce::ResizableWindow::backgroundColourId));
}

//==============================================================================
void SectionGeneralSettings::resized()
{
    mOscFormatLabel.setBounds(5, 10, 90, 15);
    mOscFormatCombo.setBounds(95, 10, 150, 20);

    mOscPortLabel.setBounds(5, 40, 90, 15);
    mOscPortEditor.setBounds(95, 40, 150, 20);

    mOscAddressLabel.setBounds(5, 70, 90, 15);
    mOscAddressEditor.setBounds(95, 70, 150, 20);

    mPositionActivateButton.setBounds(5, 100, 150, 20);

    mNumOfSourcesLabel.setBounds(265, 10, 130, 15);
    mNumOfSourcesEditor.setBounds(395, 10, 40, 15);

    mFirstSourceIdLabel.setBounds(265, 40, 130, 15);
    mFirstSourceIdEditor.setBounds(395, 40, 40, 15);
}

} // namespace gris
