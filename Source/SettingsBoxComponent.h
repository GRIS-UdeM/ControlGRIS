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

#include "../JuceLibraryCode/JuceHeader.h"

#include "ControlGrisConstants.h"
#include "GrisLookAndFeel.h"

//==============================================================================
class SettingsBoxComponent final : public Component
{
public:
    //==============================================================================
    struct Listener {
        virtual ~Listener() {}

        virtual void settingsBoxOscFormatChanged(SpatMode mode) = 0;
        virtual void settingsBoxOscPortNumberChanged(int oscPort) = 0;
        virtual void settingsBoxOscActivated(bool state) = 0;
        virtual void settingsBoxNumberOfSourcesChanged(int numOfSources) = 0;
        virtual void settingsBoxFirstSourceIdChanged(int firstSourceId) = 0;
    };

private:
    //==============================================================================
    ListenerList<Listener> mListeners;

    Label mOscFormatLabel;
    ComboBox mOscFormatCombo;

    Label mOscPortLabel;
    TextEditor mOscPortEditor;

    Label mNumOfSourcesLabel;
    TextEditor mNumOfSourcesEditor;

    Label mFirstSourceIdLabel;
    TextEditor mFirstSourceIdEditor;

    ToggleButton mPositionActivateButton;

public:
    //==============================================================================
    SettingsBoxComponent();
    ~SettingsBoxComponent() final;

    void paint(Graphics &) override;
    void resized() override;

    // These are only setters, they dont send notification.
    //-----------------------------------------------------
    void setNumberOfSources(int numOfSources);
    void setFirstSourceId(int firstSourceId);
    void setOscFormat(SpatMode mode);
    void setOscPortNumber(int oscPortNumber);
    void setActivateButtonState(bool shouldBeOn);

    void addListener(Listener * l) { mListeners.add(l); }
    void removeListener(Listener * l) { mListeners.remove(l); }

private:
    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(SettingsBoxComponent)
};
