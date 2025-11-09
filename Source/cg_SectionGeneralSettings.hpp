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
#include "cg_constants.hpp"
#include <Data/StrongTypes/sg_SourceIndex.hpp>

namespace gris
{
//==============================================================================
class SectionGeneralSettings final : public juce::Component
{
public:
    //==============================================================================
    struct Listener {
        virtual ~Listener() = default;

        virtual void oscFormatChangedCallback(SpatMode mode) = 0;
        virtual void oscPortChangedCallback(int oscPort) = 0;
        virtual void oscAddressChangedCallback(juce::String const & address) = 0;
        virtual void oscStateChangedCallback(bool state) = 0;
        virtual void numberOfSourcesChangedCallback(int numOfSources) = 0;
        virtual void firstSourceIdChangedCallback(SourceId firstSourceId) = 0;
    };

private:
    //==============================================================================
    GrisLookAndFeel & mGrisLookAndFeel;

    juce::ListenerList<Listener> mListeners;

    juce::Label mOscFormatLabel;
    juce::ComboBox mOscFormatCombo;

    juce::Label mOscPortLabel;
    TextEd mOscPortEditor;

    juce::Label mOscAddressLabel;
    TextEd mOscAddressEditor;

    juce::Label mNumOfSourcesLabel;
    TextEd mNumOfSourcesEditor;

    juce::Label mFirstSourceIdLabel;
    TextEd mFirstSourceIdEditor;

    juce::ToggleButton mPositionActivateButton;

public:
    //==============================================================================
    explicit SectionGeneralSettings(GrisLookAndFeel & grisLookAndFeel);
    //==============================================================================
    SectionGeneralSettings() = delete;
    ~SectionGeneralSettings() override = default;

    SectionGeneralSettings(SectionGeneralSettings const &) = delete;
    SectionGeneralSettings(SectionGeneralSettings &&) = delete;

    SectionGeneralSettings & operator=(SectionGeneralSettings const &) = delete;
    SectionGeneralSettings & operator=(SectionGeneralSettings &&) = delete;
    //==============================================================================

    // These are only setters, they dont send notification.
    //-----------------------------------------------------
    void setNumberOfSources(int numOfSources);
    void setFirstSourceId(SourceId firstSourceId);
    void setOscFormat(SpatMode mode);
    void setOscPortNumber(int oscPortNumber);
    void setOscAddress(juce::String const & address);
    void setActivateButtonState(bool shouldBeOn);

    void addListener(Listener * l) { mListeners.add(l); }
    void removeListener(Listener * l) { mListeners.remove(l); }
    //==============================================================================
    // overrides
    void resized() override;

private:
    //==============================================================================
    JUCE_LEAK_DETECTOR(SectionGeneralSettings)
};

} // namespace gris
