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

#pragma once

#include "cg_ControlGrisLookAndFeel.hpp"
#include <JuceHeader.h>

namespace gris
{
//==============================================================================
class TextEd
    : public juce::TextEditor
    , private juce::TextEditor::Listener
{
public:
    //==============================================================================
    TextEd() = delete;
    explicit TextEd(GrisLookAndFeel & glaf);

    ~TextEd() override = default;
    //==============================================================================
    TextEd(TextEd const &) = delete;
    TextEd(TextEd &&) = delete;
    TextEd & operator=(TextEd const &) = delete;
    TextEd & operator=(TextEd &&) = delete;

    //==============================================================================
    void mouseDown(const juce::MouseEvent & event) override;
    void mouseDoubleClick(const juce::MouseEvent & event) override;

    void textEditorReturnKeyPressed(juce::TextEditor & ed) override;
    void textEditorEscapeKeyPressed(juce::TextEditor & ed) override;

private:
    //==============================================================================
    GrisLookAndFeel & mGrisLookAndFeel;
    bool mIsCurrentlyEditing{};
    juce::String mCurrentText;

    //==============================================================================
    JUCE_LEAK_DETECTOR(TextEd)
};
} // namespace gris
