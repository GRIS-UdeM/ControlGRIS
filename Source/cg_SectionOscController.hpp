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
#include "cg_TextEditor.h"

namespace gris
{
//==============================================================================
class SectionOscController final : public juce::Component
{
public:
    struct Listener {
        virtual ~Listener() = default;

        virtual void oscOutputPluginIdChangedCallback(int value) = 0;
        virtual void oscInputConnectionChangedCallback(bool state, int oscPort) = 0;
        virtual void oscOutputConnectionChangedCallback(bool state, juce::String oscAddress, int oscPort) = 0;
    };

private:
    //==============================================================================
    GrisLookAndFeel & mGrisLookAndFeel;

    juce::ListenerList<Listener> mListeners{};

    juce::ToggleButton mOscReceiveToggle{};
    juce::ToggleButton mOscSendToggle{};

    juce::Label mOscOutputPluginIdLabel{};
    TextEd mOscOutputPluginIdEditor{};

    TextEd mOscReceiveIpEditor{};
    TextEd mOscReceivePortEditor{};
    TextEd mOscSendIpEditor{};
    TextEd mOscSendPortEditor{};

    int mLastOscReceivePort{ 9000 };
    int mLastOscSendPort{ 8000 };
    juce::String mLastOscSendAddress{};

public:
    //==============================================================================
    explicit SectionOscController(GrisLookAndFeel & grisLookAndFeel);
    //==============================================================================
    SectionOscController() = delete;
    ~SectionOscController() override { setLookAndFeel(nullptr); } // TODO : necessary ?

    SectionOscController(SectionOscController const &) = delete;
    SectionOscController(SectionOscController &&) = delete;

    SectionOscController & operator=(SectionOscController const &) = delete;
    SectionOscController & operator=(SectionOscController &&) = delete;
    //==============================================================================
    void resized() override;

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
    JUCE_LEAK_DETECTOR(SectionOscController)
};

} // namespace gris
