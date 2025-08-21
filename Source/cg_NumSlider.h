/**************************************************************************
 * Copyright 2024 UdeM - GRIS - Gaël LANE LÉPINE                         *
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

#include <JuceHeader.h>

namespace gris
{
class GrisLookAndFeel;

//==============================================================================
class NumSlider
    : public juce::Slider
    , private juce::TextEditor::Listener
{
public:
    //==============================================================================
    NumSlider(GrisLookAndFeel & grisLookAndFeel);

    void mouseWheelMove(const juce::MouseEvent & event, const juce::MouseWheelDetails & wheel) override;
    void paint(juce::Graphics & g) override;
    void valueChanged() override;
    void mouseUp(const juce::MouseEvent & event) override;

    void setDefaultNumDecimalPlacesToDisplay(int numDec);

private:
    //==============================================================================
    void textEditorReturnKeyPressed(juce::TextEditor & ed) override;
    void textEditorEscapeKeyPressed(juce::TextEditor & ed) override;

    GrisLookAndFeel & mGrisLookAndFeel;

    //==============================================================================
    juce::Time mLastTime{ 0 };
    double mLastValue{ 0 };
    int mDefaultNumDecimalToDisplay{ 1 };

    //==============================================================================
    JUCE_LEAK_DETECTOR(NumSlider)
};
} // namespace gris
