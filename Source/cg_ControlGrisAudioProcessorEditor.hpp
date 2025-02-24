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

#include "cg_BannerComponent.hpp"
#include "cg_FieldComponent.hpp"
#include "cg_SectionOscController.hpp"
#include "cg_SectionPositionPresets.hpp"
#include "cg_SectionSourcePosition.hpp"
#include "cg_SectionSourceSpan.hpp"
#include "cg_SectionTrajectory.hpp"

namespace gris
{
//==============================================================================
class ControlGrisAudioProcessorEditor final
    : public juce::AudioProcessorEditor
    , private juce::Value::Listener
    , public FieldComponent::Listener
    , public SectionSourceSpan::Listener
    , public SectionGeneralSettings::Listener
    , public SectionSourcePosition::Listener
    , public SectionTrajectory::Listener
    , public SectionOscController::Listener
    , public PositionPresetComponent::Listener
{
private:
    //==============================================================================
    ControlGrisAudioProcessor & mProcessor;
    GrisLookAndFeel mGrisLookAndFeel;
    juce::AudioProcessorValueTreeState & mAudioProcessorValueTreeState;

    PositionTrajectoryManager & mPositionTrajectoryManager;
    ElevationTrajectoryManager & mElevationTrajectoryManager;

    BannerComponent mMainBanner;
    BannerComponent mElevationBanner;
    BannerComponent mTrajectoryBanner;
    BannerComponent mSettingsBanner;
    BannerComponent mPositionPresetBanner;

    juce::ComboBox mElevationModeCombobox;
    juce::Label mElevationModeLabel;

    PositionFieldComponent mPositionField;
    ElevationFieldComponent mElevationField;

    SectionSourceSpan mSectionSourceSpan;
    SectionTrajectory mSectionTrajectory;

    juce::TabbedComponent mConfigurationComponent{ juce::TabbedButtonBar::Orientation::TabsAtTop };

    SectionGeneralSettings mSectionGeneralSettings;
    SectionSourcePosition mSectionSourcePosition;
    SectionOscController mSectionOscController;

    PositionPresetComponent mPositionPresetComponent;

    bool mIsInsideSetPluginState;
    SourceIndex mSelectedSource{};

    juce::Value mLastUiWidth;
    juce::Value mLastUiHeight;

public:
    //==============================================================================
    ControlGrisAudioProcessorEditor(ControlGrisAudioProcessor & controlGrisAudioProcessor,
                                    juce::AudioProcessorValueTreeState & vts,
                                    PositionTrajectoryManager & positionAutomationManager,
                                    ElevationTrajectoryManager & elevationAutomationManager);
    //==============================================================================
    ControlGrisAudioProcessorEditor() = delete;
    ~ControlGrisAudioProcessorEditor() override;
    //==============================================================================
    ControlGrisAudioProcessorEditor(ControlGrisAudioProcessorEditor const &) = delete;
    ControlGrisAudioProcessorEditor(ControlGrisAudioProcessorEditor &&) = delete;
    ControlGrisAudioProcessorEditor & operator=(ControlGrisAudioProcessorEditor const &) = delete;
    ControlGrisAudioProcessorEditor & operator=(ControlGrisAudioProcessorEditor &&) = delete;
    //==============================================================================
    void paint(juce::Graphics &) override;
    void resized() override;
    void valueChanged(juce::Value &) override;

    // FieldComponent::Listeners
    void fieldSourcePositionChangedCallback(SourceIndex sourceIndex, int whichField) override;

    // SectionSourceSpan::Listeners
    void selectedSourceClickedCallback() override;
    void parameterChangedCallback(SourceParameter sourceParameter, double value) override;
    void azimuthSpanDragStartedCallback() override;
    void azimuthSpanDragEndedCallback() override;
    void elevationSpanDragStartedCallback() override;
    void elevationSpanDragEndedCallback() override;

    // SectionGeneralSettings::Listeners
    void oscFormatChangedCallback(SpatMode mode) override;
    void oscPortChangedCallback(int oscPort) override;
    void oscAddressChangedCallback(juce::String const & address) override;
    void oscStateChangedCallback(bool state) override;
    void numberOfSourcesChangedCallback(int numOfSources) override;
    void firstSourceIdChangedCallback(SourceId firstSourceId) override;

    // SectionSourcePosition::Listeners
    void sourceSelectionChangedCallback(SourceIndex sourceIndex) override;
    void sourcesPlacementChangedCallback(SourcePlacement sourcePlacement) override;
    void speakerSetupSelectedCallback(juce::File speakerSetupFile) override;
    void sourcePositionChangedCallback(SourceIndex sourceIndex,
                                       std::optional<Radians> azimuth,
                                       std::optional<Radians> elevation,
                                       std::optional<float> x,
                                       std::optional<float> y,
                                       std::optional<float> z) override;

    // SectionTrajectory::Listeners
    void positionSourceLinkChangedCallback(PositionSourceLink sourceLink) override;
    void elevationSourceLinkChangedCallback(ElevationSourceLink sourceLink) override;
    void positionTrajectoryTypeChangedCallback(PositionTrajectoryType value) override;
    void elevationTrajectoryTypeChangedCallback(ElevationTrajectoryType value) override;
    void positionTrajectoryBackAndForthChangedCallback(bool value) override;
    void elevationTrajectoryBackAndForthChangedCallback(bool value) override;
    void positionTrajectoryDampeningCyclesChangedCallback(int value) override;
    void elevationTrajectoryDampeningCyclesChangedCallback(int value) override;
    void trajectoryDeviationPerCycleChangedCallback(float degrees) override;
    void trajectoryCycleDurationChangedCallback(double duration, int mode) override;
    void trajectoryDurationUnitChangedCallback(double duration, int mode) override;
    void positionTrajectoryStateChangedCallback(bool value) override;
    void elevationTrajectoryStateChangedCallback(bool value) override;

    // PositionPresetComponent::Listeners
    void positionPresetChangedCallback(int presetNumber) override;
    void positionPresetSavedCallback(int presetNumber) override;
    void positionPresetDeletedCallback(int presetNumber) override;

    // SectionOscController::Listeners
    void oscOutputPluginIdChangedCallback(int value) override;
    void oscInputConnectionChangedCallback(bool state, int oscPort) override;
    void oscOutputConnectionChangedCallback(bool state, juce::String oscAddress, int oscPort) override;

    void reloadUiState();
    void updateSpanLinkButton(bool state);
    void updateSourceLinkCombo(PositionSourceLink value);
    void updateElevationSourceLinkCombo(ElevationSourceLink value);
    void updatePositionPreset(int presetNumber);
    void updateElevationMode(ElevationMode mode);

    void elevationModeChangedStartedCallback();
    void elevationModeChangedEndedCallback();

    void refresh();

    void setSpatMode(SpatMode spatMode);

private:
    //==============================================================================
    JUCE_LEAK_DETECTOR(ControlGrisAudioProcessorEditor)
};

} // namespace gris
