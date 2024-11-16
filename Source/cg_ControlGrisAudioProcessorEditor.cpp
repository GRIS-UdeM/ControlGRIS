/*
 This file is part of ControlGris.

 Developers: Olivier BELANGER, Gaël LANE LÉPINE

 ControlGris is free software: you can redistribute it and/or modify
 it under the terms of the GNU Lesser General Public License as
 published by the Free Software Foundation, either version 3 of the
 License, or (at your option) any later version.

 ControlGris is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU Lesser General Public License for more details.

 You should have received a copy of the GNU Lesser General Public
 License along with ControlGris.  If not, see
 <http://www.gnu.org/licenses/>.
*/

//==============================================================================

#include "cg_ControlGrisAudioProcessorEditor.hpp"

#include "cg_ControlGrisAudioProcessor.hpp"
#include "cg_constants.hpp"

namespace gris
{
//==============================================================================
ControlGrisAudioProcessorEditor::ControlGrisAudioProcessorEditor(
    ControlGrisAudioProcessor & controlGrisAudioProcessor,
    juce::AudioProcessorValueTreeState & vts,
    PositionTrajectoryManager & positionAutomationManager,
    ElevationTrajectoryManager & elevationAutomationManager)
    : AudioProcessorEditor(&controlGrisAudioProcessor)
    , mProcessor(controlGrisAudioProcessor)
    , mAudioProcessorValueTreeState(vts)
    , mPositionTrajectoryManager(positionAutomationManager)
    , mElevationTrajectoryManager(elevationAutomationManager)
    , mPositionField(controlGrisAudioProcessor.getSources(), positionAutomationManager)
    , mElevationField(controlGrisAudioProcessor.getSources(), elevationAutomationManager)
    , mSectionSourceSpan(mGrisLookAndFeel)
    , mSectionAbstractSpatialization(mGrisLookAndFeel, vts)
    , mSectionSoundReactiveSpatialization(mGrisLookAndFeel, mProcessor)
    , mSectionGeneralSettings(mGrisLookAndFeel)
    , mSectionSourcePosition(mGrisLookAndFeel, controlGrisAudioProcessor.getSpatMode(), mSectionSourceSpan)
    , mSectionOscController(mGrisLookAndFeel)
    , mPositionPresetComponent(controlGrisAudioProcessor.getPresetsManager(), mPositionPresetInfoComponent)
    , mPositionPresetInfoComponent(mGrisLookAndFeel)
{
    setLookAndFeel(&mGrisLookAndFeel);

    mIsInsideSetPluginState = false;
    mSelectedSource = {};

    // Set up the interface.
    //----------------------
    mMainBanner.setLookAndFeel(&mGrisLookAndFeel);
    mMainBanner.setText("Azimuth - Elevation", juce::NotificationType::dontSendNotification);
    addAndMakeVisible(&mMainBanner);

    mElevationBanner.setLookAndFeel(&mGrisLookAndFeel);
    mElevationBanner.setText("Z", juce::dontSendNotification);
    addAndMakeVisible(&mElevationBanner);

    mSourcesBanner.setLookAndFeel(&mGrisLookAndFeel);
    mSourcesBanner.setText("Sources", juce::dontSendNotification);
    addAndMakeVisible(&mSourcesBanner);

    mElevationModeLabel.setEditable(false, false);
    mElevationModeLabel.setText("Mode", juce::dontSendNotification);
    mElevationModeLabel.setJustificationType(juce::Justification::centredRight);
    addAndMakeVisible(&mElevationModeLabel);

    mElevationModeCombobox.setLookAndFeel(&mGrisLookAndFeel);
    mElevationModeCombobox.addItemList(ELEVATION_MODE_TYPES, 1);
    mElevationModeCombobox.onChange = [this] {
        elevationModeChangedStartedCallback();
        auto const howMany{ static_cast<float>(ELEVATION_MODE_TYPES.size() - 1) };
        auto const value{ (static_cast<float>(mElevationModeCombobox.getSelectedId()) - 1.0f) / howMany };
        auto * parameter{ mAudioProcessorValueTreeState.getParameter(Automation::Ids::ELEVATION_MODE) };
        auto const gestureLock{ mProcessor.getChangeGestureManager().getScopedLock(Automation::Ids::ELEVATION_MODE) };
        parameter->setValueNotifyingHost(value);
        elevationModeChangedEndedCallback();
    };
    addAndMakeVisible(&mElevationModeCombobox);

    auto const width{ getWidth() - 50 }; // Remove position preset space.
    auto const fieldSize{ std::max(width / 2, MIN_FIELD_WIDTH) };
    // only setting positions in resized() is not sufficient
    mElevationModeCombobox.setBounds(fieldSize + mElevationBanner.getBounds().getWidth() / 2,
                                     (mElevationBanner.getHeight() - mElevationModeCombobox.getHeight()) / 2,
                                     (mElevationBanner.getBounds().getWidth() / 2) - 4,
                                     16);
    mElevationModeLabel.setBounds(mElevationModeCombobox.getBounds().getX() - 60,
                                  (mElevationBanner.getHeight() - mElevationModeLabel.getHeight()) / 2,
                                  60,
                                  12);

    mSpatializationBanner.setLookAndFeel(&mGrisLookAndFeel);
    mSpatializationBanner.setText("Spatialization", juce::NotificationType::dontSendNotification);
    addAndMakeVisible(&mSpatializationBanner);

    mSettingsBanner.setLookAndFeel(&mGrisLookAndFeel);
    mSettingsBanner.setText("Configuration", juce::NotificationType::dontSendNotification);
    addAndMakeVisible(&mSettingsBanner);

    mPositionPresetBanner.setLookAndFeel(&mGrisLookAndFeel);
    mPositionPresetBanner.setText("Preset", juce::NotificationType::dontSendNotification);
    addAndMakeVisible(&mPositionPresetBanner);

    mPositionField.setLookAndFeel(&mGrisLookAndFeel);
    mPositionField.addListener(this);
    addAndMakeVisible(&mPositionField);

    mElevationField.setLookAndFeel(&mGrisLookAndFeel);
    mElevationField.addListener(this);
    addAndMakeVisible(&mElevationField);

    mSectionSourceSpan.setLookAndFeel(&mGrisLookAndFeel);
    mSectionSourceSpan.addListener(this);

    mSectionAbstractSpatialization.setLookAndFeel(&mGrisLookAndFeel);
    mSectionAbstractSpatialization.addListener(this);

    mSectionGeneralSettings.setLookAndFeel(&mGrisLookAndFeel);
    mSectionGeneralSettings.addListener(this);

    mSectionSourcePosition.setLookAndFeel(&mGrisLookAndFeel);
    mSectionSourcePosition.addListener(this);
    addAndMakeVisible(&mSectionSourcePosition);
    mSectionSourcePosition.setPositionSourceLink(mPositionTrajectoryManager.getSourceLink());
    mSectionSourcePosition.setElevationSourceLink(
        static_cast<ElevationSourceLink>(mElevationTrajectoryManager.getSourceLink()));

    mSectionOscController.setLookAndFeel(&mGrisLookAndFeel);
    mSectionOscController.addListener(this);

    auto const bg{ mGrisLookAndFeel.findColour(juce::ResizableWindow::backgroundColourId) };

    mConfigurationComponent.setLookAndFeel(&mGrisLookAndFeel);
    mConfigurationComponent.setColour(juce::TabbedComponent::backgroundColourId, bg);
    mConfigurationComponent.setTabBarDepth(24);
    mConfigurationComponent.addTab("Settings", bg, &mSectionGeneralSettings, false);
    mConfigurationComponent.addTab("Controllers", bg, &mSectionOscController, false);
    addAndMakeVisible(mConfigurationComponent);

    mSpatializationComponent.setLookAndFeel(&mGrisLookAndFeel);
    mSpatializationComponent.setColour(juce::TabbedComponent::backgroundColourId, bg);
    mSpatializationComponent.setTabBarDepth(24);
    mSpatializationComponent.addTab(mSectionSoundReactiveSpatialization.getName(),
                                    bg,
                                    &mSectionSoundReactiveSpatialization,
                                    false);
    mSpatializationComponent.addTab("Abstract Spatialization", bg, &mSectionAbstractSpatialization, false);
    addAndMakeVisible(mSpatializationComponent);

    mPositionPresetComponent.setLookAndFeel(&mGrisLookAndFeel);
    mPositionPresetComponent.addListener(this);
    // addAndMakeVisible(&mPositionPresetComponent);

    mPositionPresetViewport.setViewedComponent(&mPositionPresetComponent);
    mPositionPresetViewport.setScrollBarsShown(false, false, true, true);
    addAndMakeVisible(&mPositionPresetViewport);

    mPositionPresetInfoComponent.setAppVersionLabelText(juce::String("v. ") + JucePlugin_VersionString
                                                            + juce::String("\n") + juce::String("RC.2"),
                                                        juce::NotificationType::dontSendNotification);
    addAndMakeVisible(&mPositionPresetInfoComponent);

    // Add sources to the fields.
    //---------------------------
    mPositionField.refreshSources();
    mElevationField.refreshSources();

    mSectionSourceSpan.setSelectedSource(&mProcessor.getSources()[mSelectedSource]);

    // Manage dynamic window size of the plugin.
    //------------------------------------------
    setResizable(true, true);
    setResizeLimits(MIN_FIELD_WIDTH + 50, MIN_FIELD_WIDTH + 20, 1800, 1300);

    mLastUiWidth.referTo(
        mProcessor.getValueTreeState().state.getChildWithName("uiState").getPropertyAsValue("width", nullptr));
    mLastUiHeight.referTo(
        mProcessor.getValueTreeState().state.getChildWithName("uiState").getPropertyAsValue("height", nullptr));

    // set our component's initial size to be the last one that was stored in the filter's settings
    setSize(mLastUiWidth.getValue(), mLastUiHeight.getValue());

    mLastUiWidth.addListener(this);
    mLastUiHeight.addListener(this);

    // Load the last saved state of the plugin.
    //-----------------------------------------
    reloadUiState();
}

//==============================================================================
ControlGrisAudioProcessorEditor::~ControlGrisAudioProcessorEditor()
{
    mConfigurationComponent.setLookAndFeel(nullptr);
    mSpatializationComponent.setLookAndFeel(nullptr);
    setLookAndFeel(nullptr);
}

//==============================================================================
void ControlGrisAudioProcessorEditor::reloadUiState()
{
    mIsInsideSetPluginState = true;

    // Set global settings values.
    //----------------------------
    oscFormatChangedCallback(mProcessor.getSpatMode());
    oscPortChangedCallback(mProcessor.getOscPortNumber());
    oscAddressChangedCallback(mProcessor.getOscAddress());
    oscStateChangedCallback(mProcessor.isOscActive());
    firstSourceIdChangedCallback(mProcessor.getFirstSourceId());
    numberOfSourcesChangedCallback(mProcessor.getSources().size());

    mSectionOscController.setOscOutputPluginId(mAudioProcessorValueTreeState.state.getProperty("oscOutputPluginId", 1));
    mSectionOscController.setOscReceiveToggleState(
        mAudioProcessorValueTreeState.state.getProperty("oscInputConnected", false));
    mSectionOscController.setOscReceiveInputPort(
        mAudioProcessorValueTreeState.state.getProperty("oscInputPortNumber", 9000));

    mSectionOscController.setOscSendToggleState(
        mAudioProcessorValueTreeState.state.getProperty("oscOutputConnected", false));
    mSectionOscController.setOscSendOutputAddress(
        mAudioProcessorValueTreeState.state.getProperty("oscOutputAddress", "192.168.1.100"));
    mSectionOscController.setOscSendOutputPort(
        mAudioProcessorValueTreeState.state.getProperty("oscOutputPortNumber", 8000));

    // Set state for abstraction spatialization box persistent values.
    //------------------------------------------------
    mSectionAbstractSpatialization.setTrajectoryType(
        mAudioProcessorValueTreeState.state.getProperty("trajectoryType", 1));
    mSectionAbstractSpatialization.setElevationTrajectoryType(
        mAudioProcessorValueTreeState.state.getProperty("trajectoryTypeAlt", 1));
    mSectionAbstractSpatialization.setPositionBackAndForth(
        mAudioProcessorValueTreeState.state.getProperty("backAndForth", false));
    mSectionAbstractSpatialization.setElevationBackAndForth(
        mAudioProcessorValueTreeState.state.getProperty("backAndForthAlt", false));
    mSectionAbstractSpatialization.setPositionDampeningCycles(
        mAudioProcessorValueTreeState.state.getProperty("dampeningCycles", 0));
    mPositionTrajectoryManager.setPositionDampeningCycles(
        mAudioProcessorValueTreeState.state.getProperty("dampeningCycles", 0));
    mSectionAbstractSpatialization.setElevationDampeningCycles(
        mAudioProcessorValueTreeState.state.getProperty("dampeningCyclesAlt", 0));
    mElevationTrajectoryManager.setPositionDampeningCycles(
        mAudioProcessorValueTreeState.state.getProperty("dampeningCyclesAlt", 0));
    mSectionAbstractSpatialization.setDeviationPerCycle(
        mAudioProcessorValueTreeState.state.getProperty("deviationPerCycle", 0));
    mPositionTrajectoryManager.setDeviationPerCycle(
        Degrees{ mAudioProcessorValueTreeState.state.getProperty("deviationPerCycle", 0) });
    mSectionAbstractSpatialization.setCycleDuration(
        mAudioProcessorValueTreeState.state.getProperty("cycleDuration", 5.0));
    mSectionAbstractSpatialization.setDurationUnit(mAudioProcessorValueTreeState.state.getProperty("durationUnit", 1));

    // Update the position preset box.
    //--------------------------------
    auto const savedPresets{ mProcessor.getPresetsManager().getSavedPresets() };
    int index{ 1 };
    for (auto const saved : savedPresets) {
        mPositionPresetComponent.presetSaved(index++, saved);
    }

    // Update the Spatialization Tab
    mSpatializationComponent.setCurrentTabIndex(mAudioProcessorValueTreeState.state.getProperty("soundSpatSelTab", 0));
    mSectionAbstractSpatialization.actualizeValueTreeState();

    // Update the interface.
    //----------------------
    mSectionSourceSpan.setSelectedSource(&mProcessor.getSources()[mSelectedSource]);
    mPositionField.setSelectedSource(mSelectedSource);
    mElevationField.setSelectedSource(mSelectedSource);
    mSectionSourcePosition.updateSelectedSource(&mProcessor.getSources()[mSelectedSource],
                                                mSelectedSource,
                                                mProcessor.getSpatMode());

    auto const preset{ static_cast<int>(static_cast<float>(
        mAudioProcessorValueTreeState.getParameterAsValue(Automation::Ids::POSITION_PRESET).getValue())) };
    mPositionPresetComponent.setPreset(preset, false);

    auto const elevModeValue{ mAudioProcessorValueTreeState.getParameterAsValue(Automation::Ids::ELEVATION_MODE) };
    auto const elevMode{ static_cast<ElevationMode>(static_cast<int>(elevModeValue.getValue())) };
    updateElevationMode(elevMode);

    mIsInsideSetPluginState = false;
}

//==============================================================================
void ControlGrisAudioProcessorEditor::updateSpanLinkButton(bool state)
{
    mSectionSourceSpan.setSpanLinkState(state);
}

//==============================================================================
void ControlGrisAudioProcessorEditor::updateSpeedLinkButton(bool state)
{
    mSectionAbstractSpatialization.setSpeedLinkState(state);
}

//==============================================================================
void ControlGrisAudioProcessorEditor::updateSourceLinkCombo(PositionSourceLink value)
{
    auto action = [=]() {
        mSectionSourcePosition.getPositionSourceLinkCombo().setSelectedId(static_cast<int>(value),
                                                                          juce::NotificationType::dontSendNotification);
    };
    auto const isMessageThread{ juce::MessageManager::getInstance()->isThisTheMessageThread() };
    if (isMessageThread) {
        action();
    } else {
        juce::MessageManager::callAsync(action);
    }
}

//==============================================================================
void ControlGrisAudioProcessorEditor::updateElevationSourceLinkCombo(ElevationSourceLink value)
{
    juce::MessageManager::callAsync([=] {
        mSectionSourcePosition.getElevationSourceLinkCombo().setSelectedId(
            static_cast<int>(value),
            juce::NotificationType::dontSendNotification);
    });
}

//==============================================================================
void ControlGrisAudioProcessorEditor::updatePositionPreset(int const presetNumber)
{
    juce::MessageManagerLock mml{};
    mPositionPresetComponent.setPreset(presetNumber, true);
}

//==============================================================================
void ControlGrisAudioProcessorEditor::updateElevationMode(ElevationMode mode)
{
    auto const updateElevMode = [=]() {
        mElevationModeCombobox.setSelectedId(static_cast<int>(mode) + 1, juce::dontSendNotification);
        mElevationField.setElevationMode(mode);
    };

    if (juce::MessageManager::getInstance()->isThisTheMessageThread()) {
        updateElevMode();
    } else {
        juce::MessageManager::callAsync([=] { updateElevMode(); });
    }
}

//==============================================================================
void ControlGrisAudioProcessorEditor::elevationModeChangedStartedCallback()
{
    mProcessor.getChangeGestureManager().beginGesture(Automation::Ids::ELEVATION_MODE);
}

//==============================================================================
void ControlGrisAudioProcessorEditor::elevationModeChangedEndedCallback()
{
    mProcessor.getChangeGestureManager().endGesture(Automation::Ids::ELEVATION_MODE);
}

//==============================================================================
void ControlGrisAudioProcessorEditor::updateAudioAnalysisNumInputChannels()
{
    mSectionSoundReactiveSpatialization.updateChannelMixCombo();
}

//==============================================================================
// Value::Listener callback. Called when the stored window size changes.
void ControlGrisAudioProcessorEditor::valueChanged(juce::Value &)
{
    setSize(mLastUiWidth.getValue(), mLastUiHeight.getValue());
}

//==============================================================================
// SectionGeneralSettings::Listener callbacks.
void ControlGrisAudioProcessorEditor::oscFormatChangedCallback(SpatMode mode)
{
    mSectionGeneralSettings.setOscFormat(mode);
    mProcessor.setSpatMode(mode);
    auto const selectionIsLBAP{ mode == SpatMode::cube };
    mSectionSourceSpan.setDistanceEnabled(selectionIsLBAP);
    mPositionField.setSpatMode(mode);
    mSectionAbstractSpatialization.setSpatMode(mode);
    repaint();
    resized();
}

//==============================================================================
void ControlGrisAudioProcessorEditor::oscPortChangedCallback(int const oscPort)
{
    mProcessor.setOscPortNumber(oscPort);
    mSectionGeneralSettings.setOscPortNumber(oscPort);
}

//==============================================================================
void ControlGrisAudioProcessorEditor::oscAddressChangedCallback(juce::String const & address)
{
    mProcessor.setOscAddress(address);
    mSectionGeneralSettings.setOscAddress(address);
}

//==============================================================================
void ControlGrisAudioProcessorEditor::oscStateChangedCallback(bool const state)
{
    mProcessor.setOscActive(state);
    mSectionGeneralSettings.setActivateButtonState(mProcessor.isOscActive());
}

//==============================================================================
void ControlGrisAudioProcessorEditor::numberOfSourcesChangedCallback(int const numOfSources)
{
    if (mProcessor.getSources().size() != numOfSources || mIsInsideSetPluginState) {
        auto const initSourcePlacement{ mProcessor.getSources().size() != numOfSources };
        auto const currentPositionSourceLink{ mPositionTrajectoryManager.getSourceLink() };
        auto const symmetricLinkAllowed{ numOfSources == 2 };
        mSectionSourcePosition.setSymmetricLinkComboState(symmetricLinkAllowed);
        if (!symmetricLinkAllowed) {
            auto const isCurrentPositionSourceLinkSymmetric{ currentPositionSourceLink == PositionSourceLink::symmetricX
                                                             || currentPositionSourceLink
                                                                    == PositionSourceLink::symmetricY };
            if (isCurrentPositionSourceLinkSymmetric) {
                mProcessor.setPositionSourceLink(PositionSourceLink::independent,
                                                 SourceLinkEnforcer::OriginOfChange::user);
            }
        }

        mSelectedSource = {};
        mProcessor.setNumberOfSources(numOfSources);
        mSectionGeneralSettings.setNumberOfSources(numOfSources);
        mSectionSourceSpan.setSelectedSource(&mProcessor.getSources()[mSelectedSource]);
        mPositionField.refreshSources();
        mElevationField.refreshSources();
        mSectionSourcePosition.setNumberOfSources(numOfSources, mProcessor.getFirstSourceId());
        if (initSourcePlacement) {
            sourcesPlacementChangedCallback(SourcePlacement::leftAlternate);
        }
    }
}

//==============================================================================
void ControlGrisAudioProcessorEditor::firstSourceIdChangedCallback(SourceId const firstSourceId)
{
    mProcessor.setFirstSourceId(firstSourceId);
    mSectionGeneralSettings.setFirstSourceId(firstSourceId);
    mSectionSourceSpan.setSelectedSource(&mProcessor.getSources()[mSelectedSource]);
    mSectionSourcePosition.setNumberOfSources(mProcessor.getSources().size(), firstSourceId);

    mPositionField.rebuildSourceComponents(mProcessor.getSources().size());
    mElevationField.rebuildSourceComponents(mProcessor.getSources().size());
    if (mProcessor.getSpatMode() == SpatMode::cube)
        mElevationField.repaint();

    mSectionSourceSpan.repaint();
}

//==============================================================================
// SectionSourcePosition::Listener callbacks.
void ControlGrisAudioProcessorEditor::sourceSelectionChangedCallback(SourceIndex const sourceIndex)
{
    mSelectedSource = sourceIndex;

    mSectionSourceSpan.setSelectedSource(&mProcessor.getSources()[mSelectedSource]);
    mPositionField.setSelectedSource(mSelectedSource);
    mElevationField.setSelectedSource(mSelectedSource);
    mSectionSourcePosition.updateSelectedSource(&mProcessor.getSources()[mSelectedSource],
                                                mSelectedSource,
                                                mProcessor.getSpatMode());
}

//==============================================================================
void ControlGrisAudioProcessorEditor::sourcesPlacementChangedCallback(SourcePlacement const sourcePlacement)
{
    auto const numOfSources = mProcessor.getSources().size();
    constexpr Degrees azims2[2] = { Degrees{ -90.0f }, Degrees{ 90.0f } };
    constexpr Degrees azims4[4] = { Degrees{ -45.0f }, Degrees{ 45.0f }, Degrees{ -135.0f }, Degrees{ 135.0f } };
    constexpr Degrees azims6[6] = { Degrees{ -30.0f }, Degrees{ 30.0f },   Degrees{ -90.0f },
                                    Degrees{ 90.0f },  Degrees{ -150.0f }, Degrees{ 150.0f } };
    constexpr Degrees azims8[8] = { Degrees{ -22.5f },  Degrees{ 22.5f },  Degrees{ -67.5f },  Degrees{ 67.5f },
                                    Degrees{ -112.5f }, Degrees{ 112.5f }, Degrees{ -157.5f }, Degrees{ 157.5f } };

    auto const isCubeMode{ mProcessor.getSpatMode() == SpatMode::cube };

    auto const offset{ Degrees{ 360.0f } / static_cast<float>(numOfSources) / 2.0f };
    auto const distance{ isCubeMode ? 0.7f : 1.0f };

    auto const getAzimuths = [&]() {
        if (numOfSources <= 2) {
            return azims2;
        }
        if (numOfSources <= 4) {
            return azims4;
        }
        if (numOfSources <= 6) {
            return azims6;
        }
        return azims8;
    };

    auto const * const azimuths{ getAzimuths() };

    auto const getAzimuthValue = [&](int const sourceIndex) {
        switch (sourcePlacement) {
        case SourcePlacement::leftAlternate:
            return azimuths[sourceIndex];
        case SourcePlacement::rightAlternate:
            return -azimuths[sourceIndex];
        case SourcePlacement::leftClockwise:
            return Degrees{ 360.0f } / static_cast<float>(numOfSources) * static_cast<float>(sourceIndex) - offset;
        case SourcePlacement::leftCounterClockwise:
            return Degrees{ 360.0f } / static_cast<float>(numOfSources) * static_cast<float>(-sourceIndex) - offset;
        case SourcePlacement::rightClockwise:
            return Degrees{ 360.0f } / static_cast<float>(numOfSources) * static_cast<float>(sourceIndex) + offset;
        case SourcePlacement::rightCounterClockwise:
            return Degrees{ 360.0f } / static_cast<float>(numOfSources) * static_cast<float>(-sourceIndex) + offset;
        case SourcePlacement::topClockwise:
            return Degrees{ 360.0f } / static_cast<float>(numOfSources) * static_cast<float>(sourceIndex);
        case SourcePlacement::topCounterClockwise:
            return Degrees{ 360.0f } / static_cast<float>(numOfSources) * static_cast<float>(-sourceIndex);
        case SourcePlacement::undefined:
        default:
            jassertfalse;
            break;
        }
        return Degrees{};
    };

    auto const sourceLink{ mPositionTrajectoryManager.getSourceLink() };
    mProcessor.setPositionSourceLink(PositionSourceLink::independent, SourceLinkEnforcer::OriginOfChange::automation);

    auto const setPosition = [&](int const sourceIndex) {
        auto & source{ mProcessor.getSources()[sourceIndex] };
        auto const elevation{ isCubeMode ? source.getElevation() : MAX_ELEVATION };
        source.setCoordinates(getAzimuthValue(sourceIndex),
                              elevation,
                              distance,
                              Source::OriginOfChange::userAnchorMove);
    };

    for (auto sourceIndex{ numOfSources - 1 }; sourceIndex >= 0; --sourceIndex) {
        setPosition(sourceIndex);
    }

    // mProcessor.updatePrimarySourceParameters(Source::ChangeType::position);

    for (SourceIndex i{}; i < SourceIndex{ numOfSources }; ++i) {
        mProcessor.setSourceParameterValue(i,
                                           SourceParameter::azimuth,
                                           mProcessor.getSources()[i].getNormalizedAzimuth().get());
        mProcessor.setSourceParameterValue(i,
                                           SourceParameter::elevation,
                                           mProcessor.getSources()[i].getNormalizedElevation().get());
        mProcessor.setSourceParameterValue(i, SourceParameter::distance, mProcessor.getSources()[i].getDistance());
    }

    mSectionSourcePosition.updateSelectedSource(&mProcessor.getSources()[mSelectedSource],
                                                SourceIndex{},
                                                mProcessor.getSpatMode());

    mPositionTrajectoryManager.setTrajectoryType(mPositionTrajectoryManager.getTrajectoryType(),
                                                 mProcessor.getSources().getPrimarySource().getPos());

    mProcessor.setPositionSourceLink(sourceLink, SourceLinkEnforcer::OriginOfChange::automation);

    repaint();
}

//==============================================================================
void ControlGrisAudioProcessorEditor::sourcePositionChangedCallback(SourceIndex const sourceIndex,
                                                                    std::optional<Radians> const azimuth,
                                                                    std::optional<Radians> const elevation,
                                                                    std::optional<float> const x,
                                                                    std::optional<float> const y,
                                                                    std::optional<float> const z)
{
    auto & source{ mProcessor.getSources()[sourceIndex] };

    if (azimuth) {
        source.setAzimuth(*azimuth, Source::OriginOfChange::userMove);
    } else if (elevation) {
        source.setElevation(*elevation, Source::OriginOfChange::userMove);
    } else if (x) {
        source.setX(*x, Source::OriginOfChange::userMove);
    } else if (y) {
        source.setY(*y, Source::OriginOfChange::userMove);
    } else if (z) {
        source.setElevation(MAX_ELEVATION * *z, Source::OriginOfChange::userMove);
    } else {
        jassertfalse;
    }
}

//==============================================================================
// SectionSourceSpan::Listener callbacks.
void ControlGrisAudioProcessorEditor::parameterChangedCallback(SourceParameter const sourceParameter,
                                                               double const value)
{
    mProcessor.setSourceParameterValue(mSelectedSource, sourceParameter, static_cast<float>(value));

    mPositionField.repaint();
    if (mProcessor.getSpatMode() == SpatMode::cube) {
        mElevationField.repaint();
    }
}

//==============================================================================
void ControlGrisAudioProcessorEditor::selectedSourceClickedCallback()
{
    // increment source index
    mSelectedSource = SourceIndex{ (mSelectedSource.get() + 1) % mProcessor.getSources().size() };

    mSectionSourceSpan.setSelectedSource(&mProcessor.getSources()[mSelectedSource]);
    mPositionField.setSelectedSource(mSelectedSource);
    mElevationField.setSelectedSource(mSelectedSource);
    mSectionSourcePosition.updateSelectedSource(&mProcessor.getSources()[mSelectedSource],
                                                mSelectedSource,
                                                mProcessor.getSpatMode());
}

//==============================================================================
void ControlGrisAudioProcessorEditor::azimuthSpanDragStartedCallback()
{
    mProcessor.getChangeGestureManager().beginGesture(Automation::Ids::AZIMUTH_SPAN);
}

//==============================================================================
void ControlGrisAudioProcessorEditor::azimuthSpanDragEndedCallback()
{
    mProcessor.getChangeGestureManager().endGesture(Automation::Ids::AZIMUTH_SPAN);
}

//==============================================================================
void ControlGrisAudioProcessorEditor::elevationSpanDragStartedCallback()
{
    mProcessor.getChangeGestureManager().beginGesture(Automation::Ids::ELEVATION_SPAN);
}

//==============================================================================
void ControlGrisAudioProcessorEditor::elevationSpanDragEndedCallback()
{
    mProcessor.getChangeGestureManager().endGesture(Automation::Ids::ELEVATION_SPAN);
}

//==============================================================================
// SectionAbstractSpatialization::Listener callbacks.
void ControlGrisAudioProcessorEditor::positionSourceLinkChangedCallback(PositionSourceLink const sourceLink)
{
    mProcessor.setPositionSourceLink(sourceLink, SourceLinkEnforcer::OriginOfChange::user);

    auto const howMany{ static_cast<float>(POSITION_SOURCE_LINK_TYPES.size() - 1) };
    auto const value{ (static_cast<float>(sourceLink) - 1.0f) / howMany };
    auto * parameter{ mAudioProcessorValueTreeState.getParameter(Automation::Ids::POSITION_SOURCE_LINK) };
    auto const gestureLock{ mProcessor.getChangeGestureManager().getScopedLock(Automation::Ids::POSITION_SOURCE_LINK) };
    parameter->setValueNotifyingHost(value);
}

//==============================================================================
void ControlGrisAudioProcessorEditor::elevationSourceLinkChangedCallback(ElevationSourceLink const sourceLink)
{
    mProcessor.setElevationSourceLink(sourceLink, SourceLinkEnforcer::OriginOfChange::user);

    auto const howMany{ static_cast<float>(ELEVATION_SOURCE_LINK_TYPES.size() - 1) };
    auto const value{ (static_cast<float>(sourceLink) - 1.0f) / howMany };
    auto * parameter{ mAudioProcessorValueTreeState.getParameter(Automation::Ids::ELEVATION_SOURCE_LINK) };
    auto const gestureLock{ mProcessor.getChangeGestureManager().getScopedLock(
        Automation::Ids::ELEVATION_SOURCE_LINK) };
    parameter->setValueNotifyingHost(value);
}

//==============================================================================
void ControlGrisAudioProcessorEditor::positionTrajectoryTypeChangedCallback(PositionTrajectoryType value)
{
    mAudioProcessorValueTreeState.state.setProperty("trajectoryType", static_cast<int>(value), nullptr);
    mPositionTrajectoryManager.setTrajectoryType(value, mProcessor.getSources()[0].getPos());
    mPositionField.repaint();
}

//==============================================================================
void ControlGrisAudioProcessorEditor::elevationTrajectoryTypeChangedCallback(ElevationTrajectoryType value)
{
    mAudioProcessorValueTreeState.state.setProperty("trajectoryTypeAlt", static_cast<int>(value), nullptr);
    mElevationTrajectoryManager.setTrajectoryType(value);
    mElevationField.repaint();
}

//==============================================================================
void ControlGrisAudioProcessorEditor::positionTrajectoryBackAndForthChangedCallback(bool value)
{
    mAudioProcessorValueTreeState.state.setProperty("backAndForth", value, nullptr);
    mPositionTrajectoryManager.setPositionBackAndForth(value);
}

//==============================================================================
void ControlGrisAudioProcessorEditor::elevationTrajectoryBackAndForthChangedCallback(bool value)
{
    mAudioProcessorValueTreeState.state.setProperty("backAndForthAlt", value, nullptr);
    mElevationTrajectoryManager.setPositionBackAndForth(value);
}

//==============================================================================
void ControlGrisAudioProcessorEditor::positionTrajectoryDampeningCyclesChangedCallback(int value)
{
    mAudioProcessorValueTreeState.state.setProperty("dampeningCycles", value, nullptr);
    mPositionTrajectoryManager.setPositionDampeningCycles(value);
}

//==============================================================================
void ControlGrisAudioProcessorEditor::elevationTrajectoryDampeningCyclesChangedCallback(int value)
{
    mAudioProcessorValueTreeState.state.setProperty("dampeningCyclesAlt", value, nullptr);
    mElevationTrajectoryManager.setPositionDampeningCycles(value);
}

//==============================================================================
void ControlGrisAudioProcessorEditor::trajectoryDeviationPerCycleChangedCallback(float degrees)
{
    mAudioProcessorValueTreeState.state.setProperty("deviationPerCycle", degrees, nullptr);
    mPositionTrajectoryManager.setDeviationPerCycle(Degrees{ degrees });
}

//==============================================================================
void ControlGrisAudioProcessorEditor::trajectoryCycleDurationChangedCallback(double duration, int mode)
{
    mAudioProcessorValueTreeState.state.setProperty("cycleDuration", duration, nullptr);
    double dur = duration;
    if (mode == 2) {
        dur = duration * 60.0 / mProcessor.getBpm();
    }
    mPositionTrajectoryManager.setPlaybackDuration(dur);
    mElevationTrajectoryManager.setPlaybackDuration(dur);
}

//==============================================================================
void ControlGrisAudioProcessorEditor::trajectoryDurationUnitChangedCallback(double duration, int mode)
{
    mAudioProcessorValueTreeState.state.setProperty("durationUnit", mode, nullptr);
    double dur = duration;
    if (mode == 2) {
        dur = duration * 60.0 / mProcessor.getBpm();
    }
    mPositionTrajectoryManager.setPlaybackDuration(dur);
    mElevationTrajectoryManager.setPlaybackDuration(dur);
}

//==============================================================================
void ControlGrisAudioProcessorEditor::positionTrajectoryStateChangedCallback(bool value)
{
    mPositionTrajectoryManager.setPositionActivateState(value);
    if (value) {
        mProcessor.setAudioAnalysisState(!value);
    }
}

//==============================================================================
void ControlGrisAudioProcessorEditor::elevationTrajectoryStateChangedCallback(bool value)
{
    mElevationTrajectoryManager.setPositionActivateState(value);
    if (value) {
        mProcessor.setAudioAnalysisState(!value);
    }
}

//==============================================================================
void ControlGrisAudioProcessorEditor::positionTrajectoryCurrentSpeedChangedCallback(double value)
{
    mPositionTrajectoryManager.setTrajectoryCurrentSpeed(value);
}

//==============================================================================
void ControlGrisAudioProcessorEditor::elevationTrajectoryCurrentSpeedChangedCallback(double value)
{
    mElevationTrajectoryManager.setTrajectoryCurrentSpeed(value);
}

//==============================================================================
void ControlGrisAudioProcessorEditor::positionTrajectoryRandomEnableChangedCallback(bool isEnabled)
{
    mPositionTrajectoryManager.setTrajectoryRandomEnabled(isEnabled);
}

//==============================================================================
void ControlGrisAudioProcessorEditor::positionTrajectoryRandomLoopChangedCallback(bool shouldLoop)
{
    mPositionTrajectoryManager.setTrajectoryRandomLoop(shouldLoop);
}

//==============================================================================
void ControlGrisAudioProcessorEditor::positionTrajectoryRandomTypeChangedCallback(TrajectoryRandomType type)
{
    mPositionTrajectoryManager.setTrajectoryRandomType(type);
}

//==============================================================================
void ControlGrisAudioProcessorEditor::positionTrajectoryRandomProximityChangedCallback(double value)
{
    mPositionTrajectoryManager.setTrajectoryRandomProximity(value);
}

//==============================================================================
void ControlGrisAudioProcessorEditor::positionTrajectoryRandomTimeMinChangedCallback(double value)
{
    mPositionTrajectoryManager.setTrajectoryRandomTimeMin(value);
}

//==============================================================================
void ControlGrisAudioProcessorEditor::positionTrajectoryRandomTimeMaxChangedCallback(double value)
{
    mPositionTrajectoryManager.setTrajectoryRandomTimeMax(value);
}

//==============================================================================
void ControlGrisAudioProcessorEditor::elevationTrajectoryRandomEnableChangedCallback(bool isEnabled)
{
    mElevationTrajectoryManager.setTrajectoryRandomEnabled(isEnabled);
}

//==============================================================================
void ControlGrisAudioProcessorEditor::elevationTrajectoryRandomLoopChangedCallback(bool shouldLoop)
{
    mElevationTrajectoryManager.setTrajectoryRandomLoop(shouldLoop);
}

//==============================================================================
void ControlGrisAudioProcessorEditor::elevationTrajectoryRandomTypeChangedCallback(TrajectoryRandomType type)
{
    mElevationTrajectoryManager.setTrajectoryRandomType(type);
}

//==============================================================================
void ControlGrisAudioProcessorEditor::elevationTrajectoryRandomProximityChangedCallback(double value)
{
    mElevationTrajectoryManager.setTrajectoryRandomProximity(value);
}

//==============================================================================
void ControlGrisAudioProcessorEditor::elevationTrajectoryRandomTimeMinChangedCallback(double value)
{
    mElevationTrajectoryManager.setTrajectoryRandomTimeMin(value);
}

//==============================================================================
void ControlGrisAudioProcessorEditor::elevationTrajectoryRandomTimeMaxChangedCallback(double value)
{
    mElevationTrajectoryManager.setTrajectoryRandomTimeMax(value);
}

//==============================================================================
// Update the interface if anything has changed (mostly automations).
void ControlGrisAudioProcessorEditor::refresh()
{
    mSectionSourceSpan.setSelectedSource(&mProcessor.getSources()[mSelectedSource]);
    mSectionSourcePosition.updateSelectedSource(&mProcessor.getSources()[mSelectedSource],
                                                mSelectedSource,
                                                mProcessor.getSpatMode());

    mPositionField.setIsPlaying(mProcessor.isPlaying());
    mElevationField.setIsPlaying(mProcessor.isPlaying());

    if (mSectionAbstractSpatialization.getPositionActivateState()
        != mPositionTrajectoryManager.getPositionActivateState()) {
        mSectionAbstractSpatialization.setPositionActivateState(mPositionTrajectoryManager.getPositionActivateState());
    }
    if (mSectionAbstractSpatialization.getElevationActivateState()
        != mElevationTrajectoryManager.getPositionActivateState()) {
        mSectionAbstractSpatialization.setElevationActivateState(
            mElevationTrajectoryManager.getPositionActivateState());
    }
    if (mSectionSoundReactiveSpatialization.getAudioAnalysisActivateState() != mProcessor.getAudioAnalysisState()) {
        mSectionSoundReactiveSpatialization.setAudioAnalysisActivateState(mProcessor.getAudioAnalysisState());
    }
}

//==============================================================================
void ControlGrisAudioProcessorEditor::addNewParamValueToDataGraph()
{
    // called by the audio thread
    mSectionSoundReactiveSpatialization.addNewParamValueToDataGraph();
}

//==============================================================================
// FieldComponent::Listener callback.
void ControlGrisAudioProcessorEditor::fieldSourcePositionChangedCallback(SourceIndex const sourceIndex, int whichField)
{
    mProcessor.sourcePositionChanged(sourceIndex, whichField);
    mSelectedSource = sourceIndex;
    mSectionSourceSpan.setSelectedSource(&mProcessor.getSources()[sourceIndex]);
    mPositionField.setSelectedSource(mSelectedSource);
    mElevationField.setSelectedSource(mSelectedSource);
    mSectionSourcePosition.updateSelectedSource(&mProcessor.getSources()[mSelectedSource],
                                                mSelectedSource,
                                                mProcessor.getSpatMode());
}

//==============================================================================
// PositionPresetComponent::Listener callback.
void ControlGrisAudioProcessorEditor::positionPresetChangedCallback(int const presetNumber)
{
    mProcessor.getPresetsManager().forceLoad(presetNumber);

    auto * parameter{ mAudioProcessorValueTreeState.getParameter(Automation::Ids::POSITION_PRESET) };
    auto const newValue{ static_cast<float>(presetNumber) / static_cast<float>(NUMBER_OF_POSITION_PRESETS) };

    auto const gestureLock{ mProcessor.getChangeGestureManager().getScopedLock(Automation::Ids::POSITION_PRESET) };
    parameter->setValueNotifyingHost(newValue);

    mProcessor.updatePrimarySourceParameters(Source::ChangeType::position);
    if (mProcessor.getSpatMode() == SpatMode::cube) {
        mProcessor.updatePrimarySourceParameters(Source::ChangeType::elevation);
    }
}

//==============================================================================
void ControlGrisAudioProcessorEditor::positionPresetSavedCallback(int presetNumber)
{
    mProcessor.getPresetsManager().save(presetNumber);
}

//==============================================================================
void ControlGrisAudioProcessorEditor::positionPresetDeletedCallback(int presetNumber)
{
    [[maybe_unused]] auto const success{ mProcessor.getPresetsManager().deletePreset(presetNumber) };
    jassert(success);
}

//==============================================================================
// SectionOscController::Listener callback.
void ControlGrisAudioProcessorEditor::oscOutputPluginIdChangedCallback(int const value)
{
    mProcessor.setOscOutputPluginId(value);
}

//==============================================================================
void ControlGrisAudioProcessorEditor::oscInputConnectionChangedCallback(bool const state, int const oscPort)
{
    if (state) {
        [[maybe_unused]] auto const success{ mProcessor.createOscInputConnection(oscPort) };
    } else {
        [[maybe_unused]] auto const success{ mProcessor.disconnectOscInput(oscPort) };
    }
}

//==============================================================================
void ControlGrisAudioProcessorEditor::oscOutputConnectionChangedCallback(bool const state,
                                                                         juce::String const oscAddress,
                                                                         int const oscPort)
{
    if (state) {
        [[maybe_unused]] auto const success{ mProcessor.createOscOutputConnection(oscAddress, oscPort) };
    } else {
        [[maybe_unused]] auto const success{ mProcessor.disconnectOscOutput(oscAddress, oscPort) };
    }
}

//==============================================================================
void ControlGrisAudioProcessorEditor::paint(juce::Graphics & g)
{
    g.fillAll(mGrisLookAndFeel.findColour(juce::ResizableWindow::backgroundColourId));
}

//==============================================================================
void ControlGrisAudioProcessorEditor::resized()
{
    auto const width{ getWidth() - 50 }; // Remove position preset space.
    auto const height{ getHeight() };
    auto const fieldSize{ std::max(width / 2, MIN_FIELD_WIDTH) };
    auto preseHeight{ 645 };

    if (height >= 665) {
        preseHeight = height - 20;
    }

    mMainBanner.setBounds(0, 0, fieldSize, 20);
    mPositionField.setBounds(0, 20, fieldSize, fieldSize);

    mSourcesBanner.setBounds(0, fieldSize + 20, fieldSize, 20);
    mSectionSourcePosition.setBounds(0, fieldSize + 40, fieldSize, 150);
    mSettingsBanner.setBounds(fieldSize - 1, fieldSize + 20, width, 20);
    mConfigurationComponent.setBounds(fieldSize - 1, fieldSize + 40, fieldSize, 151);

    mElevationBanner.setBounds(fieldSize, 0, fieldSize, 20);
    mElevationField.setBounds(fieldSize, 20, fieldSize, fieldSize);
    mElevationModeCombobox.setBounds(fieldSize + mElevationBanner.getBounds().getWidth() / 2,
                                     (mElevationBanner.getHeight() - mElevationModeCombobox.getHeight()) / 2,
                                     (mElevationBanner.getBounds().getWidth() / 2) - 4,
                                     16);
    mElevationModeLabel.setBounds(mElevationModeCombobox.getBounds().getX() - 60,
                                  (mElevationBanner.getHeight() - mElevationModeLabel.getHeight()) / 2,
                                  60,
                                  12);

    mSpatializationBanner.setBounds(0, fieldSize + 70 + 100 + 20, width, 20);
    mSpatializationComponent.setBounds(0, fieldSize + 90 + 100 + 20, width, 190);

    if (mProcessor.getSpatMode() == SpatMode::cube) {
        mMainBanner.setText("X - Y", juce::NotificationType::dontSendNotification);
        mElevationBanner.setVisible(true);
        mElevationField.setVisible(true);
        mElevationModeCombobox.setVisible(true);
        mElevationModeLabel.setVisible(true);
    } else {
        mMainBanner.setText("Azimuth - Elevation", juce::NotificationType::dontSendNotification);
        mElevationBanner.setVisible(false);
        mElevationModeLabel.setVisible(false);
        mElevationModeCombobox.setVisible(false);
        mElevationField.setVisible(false);
    }

    mLastUiWidth = getWidth();
    mLastUiHeight = getHeight();

    mPositionPresetViewport.setBounds(width, 20, 50, height - 20);
    mPositionPresetBanner.setBounds(width, 0, 50, 20);
    mPositionPresetComponent.setBounds(width, 20, 50, preseHeight);
    mPositionPresetInfoComponent.setBounds(width, height - 45, 50, 60);
}

//==============================================================================
void ControlGrisAudioProcessorEditor::setSpatMode(SpatMode spatMode)
{
    mSectionSourcePosition.setSpatMode(spatMode);
    mSectionSoundReactiveSpatialization.setSpatMode(spatMode);
}

//==============================================================================
TabbedSpatializationComponent::TabbedSpatializationComponent(juce::TabbedButtonBar::Orientation orientation,
                                                             ControlGrisAudioProcessor & audioProcessor)
    : juce::TabbedComponent(orientation)
    , mAudioProcessor(audioProcessor)
{
}

//==============================================================================
void TabbedSpatializationComponent::currentTabChanged(int newCurrentTabIndex, const juce::String & newCurrentTabName)
{
    mAudioProcessor.setSelectedSoundSpatializationTab(newCurrentTabIndex);
}

} // namespace gris
