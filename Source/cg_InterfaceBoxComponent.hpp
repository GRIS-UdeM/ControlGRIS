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

#pragma once

#include <JuceHeader.h>

#include "cg_ControlGrisLookAndFeel.hpp"

namespace gris
{
//==============================================================================
class InterfaceBoxComponent final
    : public juce::Component
    , public juce::TextEditor::Listener
{
public:
    struct Listener {
        virtual ~Listener() {}

        virtual void oscOutputPluginIdChanged(int value) = 0;
        virtual void oscInputConnectionChanged(bool state, int oscPort) = 0;
        virtual void oscOutputConnectionChanged(bool state, juce::String oscAddress, int oscPort) = 0;
    };

private:
    //==============================================================================
    GrisLookAndFeel & mGrisLookAndFeel;

    juce::ListenerList<Listener> mListeners{};

    juce::ToggleButton mOscReceiveToggle{};
    juce::ToggleButton mOscSendToggle{};

    juce::Label mOscOutputPluginIdLabel{};
    juce::TextEditor mOscOutputPluginIdEditor{};

    juce::TextEditor mOscReceiveIpEditor{};
    juce::TextEditor mOscReceivePortEditor{};
    juce::TextEditor mOscSendIpEditor{};
    juce::TextEditor mOscSendPortEditor{};

    int mLastOscReceivePort{ 9000 };
    int mLastOscSendPort{ 8000 };
    juce::String mLastOscSendAddress{};

public:
    //==============================================================================
    explicit InterfaceBoxComponent(GrisLookAndFeel & grisLookAndFeel);
    ~InterfaceBoxComponent() final { setLookAndFeel(nullptr); }
    //==============================================================================
    void textEditorReturnKeyPressed([[maybe_unused]] juce::TextEditor & editor) final { unfocusAllComponents(); }
    void paint(juce::Graphics &) final;
    void resized() final;

    void setOscOutputPluginId(int const id) { mOscOutputPluginIdEditor.setText(juce::String(id)); }

    void setOscReceiveToggleState(bool state);
    void setOscReceiveInputPort(int port);

    void setOscSendToggleState(bool state);
    void setOscSendOutputAddress(juce::String const & address);
    void setOscSendOutputPort(int port);

    void addListener(Listener * l) { mListeners.add(l); }
    [[maybe_unused]] void removeListener(Listener * l) { mListeners.remove(l); }

private:
    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(InterfaceBoxComponent)
};

} // namespace gris