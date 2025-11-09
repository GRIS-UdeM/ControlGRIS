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

#include "cg_TextEditor.h"

namespace gris
{
//==============================================================================
void TextEd::mouseDown(const juce::MouseEvent & event)
{
    // Prevent simple clicks from starting edit mode
    if ((event.getNumberOfClicks() == 1 && !mIsCurrentlyEditing) || isReadOnly()) {
        unfocusAllComponents();
        return;
    }

    // double-click
    mIsCurrentlyEditing = true;
    mCurrentText = getText();
    juce::TextEditor::mouseDown(event);
}

//==============================================================================
void TextEd::mouseDrag(const juce::MouseEvent & event)
{
    if (mIsCurrentlyEditing) {
        juce::TextEditor::mouseDrag(event);
        return;
    }
}

//==============================================================================
void TextEd::mouseDoubleClick(const juce::MouseEvent & event)
{
    selectAll();
}

//==============================================================================
void TextEd::stopEditing()
{
    mIsCurrentlyEditing = false;
}

//==============================================================================
void TextEd::resetCurrentText()
{
    setText(mCurrentText);
    mCurrentText.clear();
    stopEditing();
    unfocusAllComponents();
}

} // namespace gris
