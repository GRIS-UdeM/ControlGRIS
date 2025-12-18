/**************************************************************************
 * Copyright 2025 UdeM - GRIS - Gaël LANE LÉPINE                          *
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
 * <https://www.gnu.org/licenses/>.                                       *
 *************************************************************************/

#include "cg_TextEditor.hpp"

namespace gris
{
//==============================================================================
TextEd::TextEd(GrisLookAndFeel & glaf) : mGrisLookAndFeel(glaf)
{
    setReadOnly(true);
    setCaretVisible(false);
}

//==============================================================================
void TextEd::mouseDown(const juce::MouseEvent & event)
{
    // Prevent simple clicks from starting edit mode or mouse selection
    if (event.getNumberOfClicks() == 1 || isReadOnly()) {
        unfocusAllComponents();
        return;
    }
}

//==============================================================================
void TextEd::mouseDoubleClick(const juce::MouseEvent & /*event*/)
{
    if (!isEnabled())
        return;

    auto popupEditor{ std::make_unique<juce::TextEditor>("TextEdEditor") };
    popupEditor->setLookAndFeel(&mGrisLookAndFeel);
    popupEditor->setJustification(juce::Justification::centred);
    popupEditor->addListener(this);
    popupEditor->setMultiLine(false);
    popupEditor->setSize(getWidth() + 20, 20);
    popupEditor->setInputRestrictions(12, "0123456789,.");
    popupEditor->setText(getText(), false);
    popupEditor->selectAll();

    auto & box = juce::CallOutBox::launchAsynchronously(std::move(popupEditor), getScreenBounds(), nullptr);
    box.setLookAndFeel(&mGrisLookAndFeel);
}

//==============================================================================
void TextEd::textEditorReturnKeyPressed(juce::TextEditor & ed)
{
    if (!ed.getText().isEmpty()) {
        auto text = ed.getText().replace(",", ".");
        setText(text, juce::sendNotification);
        onFocusLost();
    }

    auto callOutBox = dynamic_cast<juce::CallOutBox *>(ed.getParentComponent());

    if (callOutBox != nullptr) {
        callOutBox->dismiss();
    }
}

//==============================================================================
void TextEd::textEditorEscapeKeyPressed(juce::TextEditor & ed)
{
    auto callOutBox = dynamic_cast<juce::CallOutBox *>(ed.getParentComponent());

    if (callOutBox != nullptr) {
        callOutBox->dismiss();
    }
}

} // namespace gris
