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
#include "cg_Source.hpp"

namespace gris
{
//==============================================================================

// TODO: handle negative values when _minValue is negative
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
        auto const currentText{ editor.getText() };
        auto const isNewInputDigit{ newInput.containsOnly("0123456789") };
        auto const validNewInput{ isNewInputDigit ? newInput : "" };
        auto newText{ currentText + validNewInput };
        auto const selectedRange{ editor.getHighlightedRegion() };

        if (!selectedRange.isEmpty() && !validNewInput.isEmpty())
            newText = currentText.replaceSection(selectedRange.getStart(), selectedRange.getLength(), validNewInput);

        if (newText.isEmpty())
            return newText;

        const auto value{ newText.getIntValue() };
        if (value >= minValue && value <= maxValue && isNewInputDigit)
            return validNewInput;

        if (selectedRange.isEmpty())
            return {};
        else
            return currentText.substring(selectedRange.getStart(), selectedRange.getEnd());
    }

private:
    int minValue;
    int maxValue;
};

// Unit test for NumberRangeInputFilter
class NumberRangeInputFilterTest : public juce::UnitTest
{
public:
    NumberRangeInputFilterTest() : juce::UnitTest("NumberRangeInputFilterTest") {}

    void runTest() override
    {
        beginTest("NumberRangeInputFilter allows valid input");

        {
            juce::TextEditor editor;
            editor.setInputFilter(new NumberRangeInputFilter(1, 128), true);

            editor.insertTextAtCaret("12");
            expectEquals(editor.getText().getIntValue(), 12);
        }

        beginTest("NumberRangeInputFilter disallows out-of-bound inputs");

        {
            juce::TextEditor editor;
            editor.setInputFilter(new NumberRangeInputFilter(1, 8), true);

            editor.insertTextAtCaret("-1");
            expectEquals(editor.getText().getIntValue(), 0);

            editor.insertTextAtCaret("9");
            expectEquals(editor.getText().getIntValue(), 0);

            editor.insertTextAtCaret("&");
            expectEquals(editor.getText().getIntValue(), 0);

            editor.insertTextAtCaret(juce::CharPointer_UTF8("é"));
            expectEquals(editor.getText().getIntValue(), 0);
        }

        beginTest("NumberRangeInputFilter handles partial input");

        {
            juce::TextEditor editor;
            editor.setInputFilter(new NumberRangeInputFilter(1, 128), true);

            // append 3 to 12 --> should be 123
            editor.insertTextAtCaret("12");
            editor.insertTextAtCaret("3");
            expectEquals(editor.getText().getIntValue(), 123);

            // append 9 to 12 --> should stay at 12
            editor.clear();
            editor.insertTextAtCaret("12");
            editor.insertTextAtCaret("9");
            expectEquals(editor.getText().getIntValue(), 12);

            // replace the middle 1 in 111 with 2 --> should be 121
            editor.clear();
            editor.insertTextAtCaret("111");
            editor.setHighlightedRegion({ 1, 2 });
            editor.insertTextAtCaret("2");
            expectEquals(editor.getText().getIntValue(), 121);

            // replace the middle 1 in 111 with 222 --> should stay 111
            editor.clear();
            editor.insertTextAtCaret("111");
            editor.setHighlightedRegion({ 1, 2 });
            editor.insertTextAtCaret("222");
            expectEquals(editor.getText().getIntValue(), 111);

            // replace the 23 in 123 with random garbage --> should stay 111
            editor.clear();
            editor.insertTextAtCaret("123");
            editor.setHighlightedRegion({ 1, 3 });
            editor.insertTextAtCaret(juce::CharPointer_UTF8("ééé123ööö"));
            expectEquals(editor.getText().getIntValue(), 123);
        }
    }
};

// This will automatically create an instance of the test class and add it to the list of tests to be run.
static NumberRangeInputFilterTest numberRangeInputFilterTest;

//==============================================================================
SectionGeneralSettings::SectionGeneralSettings(GrisLookAndFeel & grisLookAndFeel, ControlGrisAudioProcessor & processor)
    : mGrisLookAndFeel(grisLookAndFeel)
    , mProcessor(processor)
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
    mOscPortEditor.setLookAndFeel(&mGrisLookAndFeel);
    mOscPortEditor.setFont(grisLookAndFeel.getFont());
    mOscPortEditor.setExplicitFocusOrder(4);
    mOscPortEditor.setText(defaultPort);
    mOscPortEditor.setInputRestrictions(5, "0123456789");
    mOscPortEditor.onFocusLost = [this, defaultPort] {
        mOscPortEditor.moveCaretToEnd();
        if (!mOscPortEditor.isEmpty()) {
            mListeners.call([&](Listener & l) { l.oscPortChangedCallback(mOscPortEditor.getText().getIntValue()); });
        } else {
            mListeners.call([&](Listener & l) {
                l.oscPortChangedCallback(defaultPort.getIntValue());
                mOscPortEditor.setText(defaultPort);
            });
        }
        unfocusAllComponents();
    };
    addAndMakeVisible(mOscPortEditor);

    mOscAddressLabel.setText("IP Address:", juce::NotificationType::dontSendNotification);
    addAndMakeVisible(mOscAddressLabel);

    juce::String const defaultAddress{ "127.0.0.1" };
    mOscAddressEditor.setLookAndFeel(&mGrisLookAndFeel);
    mOscAddressEditor.setFont(grisLookAndFeel.getFont());
    mOscAddressEditor.setExplicitFocusOrder(5);
    mOscAddressEditor.setText(defaultAddress);
    mOscAddressEditor.setInputRestrictions(15, "0123456789.");
    mOscAddressEditor.onFocusLost = [this, defaultAddress]() -> void {
        mOscAddressEditor.moveCaretToEnd();
        if (!mOscAddressEditor.isEmpty()) {
            mListeners.call([&](Listener & l) { l.oscAddressChangedCallback(mOscAddressEditor.getText()); });
        } else {
            mListeners.call([&](Listener & l) {
                l.oscAddressChangedCallback(defaultAddress);
                mOscAddressEditor.setText(defaultAddress);
            });
        }
        unfocusAllComponents();
    };
    addAndMakeVisible(mOscAddressEditor);

    mNumOfSourcesLabel.setText("Number of Sources:", juce::NotificationType::dontSendNotification);
    addAndMakeVisible(&mNumOfSourcesLabel);

    mNumOfSourcesEditor.setLookAndFeel(&mGrisLookAndFeel);
    mNumOfSourcesEditor.setExplicitFocusOrder(2);
    mNumOfSourcesEditor.setFont(grisLookAndFeel.getFont());
    mNumOfSourcesEditor.setText("2");
    mNumOfSourcesEditor.setInputFilter(new NumberRangeInputFilter(1, Sources::MAX_NUMBER_OF_SOURCES), true);
    mNumOfSourcesEditor.onFocusLost = [this] {
        mNumOfSourcesEditor.moveCaretToEnd();
        if (!mNumOfSourcesEditor.isEmpty()) {
            mListeners.call(
                [&](Listener & l) { l.numberOfSourcesChangedCallback(mNumOfSourcesEditor.getText().getIntValue()); });
        } else {
            mListeners.call([&](Listener & l) {
                l.numberOfSourcesChangedCallback(1);
                mNumOfSourcesEditor.setText("1");
            });
        }
        unfocusAllComponents();
    };
    addAndMakeVisible(&mNumOfSourcesEditor);

    mFirstSourceIdLabel.setText("First Source ID:", juce::NotificationType::dontSendNotification);
    addAndMakeVisible(&mFirstSourceIdLabel);

    mFirstSourceIdEditor.setLookAndFeel(&mGrisLookAndFeel);
    mFirstSourceIdEditor.setExplicitFocusOrder(3);
    mFirstSourceIdEditor.setFont(grisLookAndFeel.getFont());
    mFirstSourceIdEditor.setText("1");
    mFirstSourceIdEditor.setInputRestrictions(3, "0123456789");
    mFirstSourceIdEditor.onFocusLost = [this] {
        mFirstSourceIdEditor.moveCaretToEnd();
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
        unfocusAllComponents();
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
void SectionGeneralSettings::resized()
{
    mOscFormatLabel.setBounds(5, 12, 90, 10);
    mOscFormatCombo.setBounds(115, 9, 110, 15);

    mOscPortLabel.setBounds(5, 34, 90, 10);
    mOscPortEditor.setBounds(115, 31, 110, 15);

    mOscAddressLabel.setBounds(5, 56, 90, 10);
    mOscAddressEditor.setBounds(115, 53, 110, 15);

    mNumOfSourcesLabel.setBounds(5, 78, 130, 10);
    mNumOfSourcesEditor.setBounds(115, 75, 24, 15);

    mFirstSourceIdLabel.setBounds(5, 100, 130, 10);
    mFirstSourceIdEditor.setBounds(115, 97, 24, 15);

    mPositionActivateButton.setBounds(175, 100, 150, 15);
}

} // namespace gris
