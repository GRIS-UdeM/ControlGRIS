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

#include "cg_ControlGrisAudioProcessor.hpp"
#include "cg_ControlGrisLookAndFeel.hpp"
#include "cg_TextEditor.hpp"
#include "cg_constants.hpp"
#include <Data/StrongTypes/sg_SourceIndex.hpp>

namespace gris
{
class SourcesTableListComponent;
class SectionGeneralSettings;
//==============================================================================
class SourcesTableListBoxModel
    : public juce::TableListBoxModel
    , private juce::ChangeListener
{
public:
    //==============================================================================
    explicit SourcesTableListBoxModel(GrisLookAndFeel & grisLookAndFeel,
                                      ControlGrisAudioProcessor & processor,
                                      SourcesTableListComponent & parentComponent);
    //==============================================================================
    SourcesTableListBoxModel() = delete;
    ~SourcesTableListBoxModel() override = default;

    SourcesTableListBoxModel(const SourcesTableListBoxModel & other) = delete;
    SourcesTableListBoxModel(SourcesTableListBoxModel && other) = delete;

    SourcesTableListBoxModel & operator=(const SourcesTableListBoxModel & other) = delete;
    SourcesTableListBoxModel & operator=(SourcesTableListBoxModel && other) = delete;

    //==============================================================================
    int getNumRows() override;
    void paintRowBackground(juce::Graphics &, int rowNumber, int width, int height, bool rowIsSelected) override;
    void paintCell(juce::Graphics &, int rowNumber, int columnId, int width, int height, bool rowIsSelected) override;
    void cellClicked(int rowNumber, int columnId, const juce::MouseEvent & event) override;
    void changeListenerCallback(juce::ChangeBroadcaster * source) override;

private:
    //==============================================================================
    GrisLookAndFeel & mGrisLookAndFeel;
    ControlGrisAudioProcessor & mProcessor;
    SourcesTableListComponent & mSourcesTableListComponent;

    SourceIndex mEditedColourSrcIndex;

    //==============================================================================
    JUCE_LEAK_DETECTOR(SourcesTableListBoxModel)
};

//==============================================================================
class SourcesTableListComponent : public juce::Component
{
    //==============================================================================
    class TableHeader : public juce::TableHeaderComponent
    {
    public:
        //==============================================================================
        TableHeader() = delete;
        ~TableHeader() override = default;

        TableHeader(const TableHeader & other) = delete;
        TableHeader(TableHeader && other) = delete;

        TableHeader & operator=(TableHeader const &) = delete;
        TableHeader & operator=(TableHeader &&) = delete;

        //==============================================================================
        explicit TableHeader(SourcesTableListComponent & parent);

        //==============================================================================
        void columnClicked(int columnId, const juce::ModifierKeys & mods) override;

    private:
        //==============================================================================
        SourcesTableListComponent & mSourcesTableListComponent;
    };

public:
    //==============================================================================
    explicit SourcesTableListComponent(GrisLookAndFeel & grisLookAndFeel,
                                       ControlGrisAudioProcessor & processor,
                                       SectionGeneralSettings & sectionGeneralSettings);
    //==============================================================================
    SourcesTableListComponent() = delete;
    ~SourcesTableListComponent() override = default;

    SourcesTableListComponent(const SourcesTableListComponent & other) = delete;
    SourcesTableListComponent(SourcesTableListComponent && other) = delete;

    SourcesTableListComponent & operator=(SourcesTableListComponent const &) = delete;
    SourcesTableListComponent & operator=(SourcesTableListComponent &&) = delete;

    //==============================================================================
    juce::TableListBox & getTableListBox();
    SectionGeneralSettings & getSectionGeneralSettings();

private:
    //==============================================================================
    GrisLookAndFeel & mGrisLookAndFeel;
    ControlGrisAudioProcessor & mProcessor;
    SectionGeneralSettings & mSectionGeneralSettings;

    SourcesTableListBoxModel mSourcesTableModel;
    juce::TableListBox mSourcesTableListBox;

    //==============================================================================
    JUCE_LEAK_DETECTOR(SourcesTableListComponent)
};

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
        virtual void sourcesColourChangedCallback(SourceIndex sourceIndex) = 0;
        virtual void allSourcesColourChangedCallback() = 0;
    };

private:
    //==============================================================================
    GrisLookAndFeel & mGrisLookAndFeel;
    ControlGrisAudioProcessor & mProcessor;

    juce::ListenerList<Listener> mListeners;

    juce::Label mOscFormatLabel;
    juce::ComboBox mOscFormatCombo;

    juce::Label mOscPortLabel;
    TextEd mOscPortEditor{ mGrisLookAndFeel };

    juce::Label mOscAddressLabel;
    TextEd mOscAddressEditor{ mGrisLookAndFeel };

    juce::Label mNumOfSourcesLabel;
    TextEd mNumOfSourcesEditor{ mGrisLookAndFeel };

    juce::Label mFirstSourceIdLabel;
    TextEd mFirstSourceIdEditor{ mGrisLookAndFeel };

    juce::TextButton mSourcesColourEditButton;

    juce::ToggleButton mPositionActivateButton;

public:
    //==============================================================================
    explicit SectionGeneralSettings(GrisLookAndFeel & grisLookAndFeel, ControlGrisAudioProcessor & processor);
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

    void updateSourcesColour(SourceIndex sourceIndex);
    void updateAllSourcesColour();
    //==============================================================================
    // overrides
    void resized() override;

private:
    //==============================================================================
    JUCE_LEAK_DETECTOR(SectionGeneralSettings)
};

} // namespace gris
