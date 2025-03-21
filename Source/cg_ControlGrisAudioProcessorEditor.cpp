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
    , mSectionTrajectory(mGrisLookAndFeel)
    , mSectionGeneralSettings(mGrisLookAndFeel)
    , mSectionSourcePosition(mGrisLookAndFeel, controlGrisAudioProcessor.getSpatMode())
    , mSectionOscController(mGrisLookAndFeel)
    , mPositionPresetComponent(controlGrisAudioProcessor.getPresetsManager())
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

    mTrajectoryBanner.setLookAndFeel(&mGrisLookAndFeel);
    mTrajectoryBanner.setText("Trajectories", juce::NotificationType::dontSendNotification);
    addAndMakeVisible(&mTrajectoryBanner);

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
    addAndMakeVisible(&mSectionSourceSpan);

    mSectionTrajectory.setLookAndFeel(&mGrisLookAndFeel);
    mSectionTrajectory.addListener(this);
    addAndMakeVisible(mSectionTrajectory);
    mSectionTrajectory.setPositionSourceLink(mPositionTrajectoryManager.getSourceLink());
    mSectionTrajectory.setElevationSourceLink(
        static_cast<ElevationSourceLink>(mElevationTrajectoryManager.getSourceLink()));

    mSectionGeneralSettings.setLookAndFeel(&mGrisLookAndFeel);
    mSectionGeneralSettings.addListener(this);

    mSectionSourcePosition.setLookAndFeel(&mGrisLookAndFeel);
    mSectionSourcePosition.addListener(this);

    mSectionOscController.setLookAndFeel(&mGrisLookAndFeel);
    mSectionOscController.addListener(this);

    auto const bg{ mGrisLookAndFeel.findColour(juce::ResizableWindow::backgroundColourId) };

    mConfigurationComponent.setLookAndFeel(&mGrisLookAndFeel);
    mConfigurationComponent.setColour(juce::TabbedComponent::backgroundColourId, bg);
    mConfigurationComponent.addTab("Settings", bg, &mSectionGeneralSettings, false);
    mConfigurationComponent.addTab("Sources", bg, &mSectionSourcePosition, false);
    mConfigurationComponent.addTab("Controllers", bg, &mSectionOscController, false);
    addAndMakeVisible(mConfigurationComponent);

    mPositionPresetComponent.setLookAndFeel(&mGrisLookAndFeel);
    mPositionPresetComponent.addListener(this);
    addAndMakeVisible(&mPositionPresetComponent);

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

    // Set state for trajectory box persistent values.
    //------------------------------------------------
    mSectionTrajectory.setTrajectoryType(mAudioProcessorValueTreeState.state.getProperty("trajectoryType", 1));
    mSectionTrajectory.setElevationTrajectoryType(
        mAudioProcessorValueTreeState.state.getProperty("trajectoryTypeAlt", 1));
    mSectionTrajectory.setPositionBackAndForth(mAudioProcessorValueTreeState.state.getProperty("backAndForth", false));
    mSectionTrajectory.setElevationBackAndForth(
        mAudioProcessorValueTreeState.state.getProperty("backAndForthAlt", false));
    mSectionTrajectory.setPositionDampeningCycles(
        mAudioProcessorValueTreeState.state.getProperty("dampeningCycles", 0));
    mPositionTrajectoryManager.setPositionDampeningCycles(
        mAudioProcessorValueTreeState.state.getProperty("dampeningCycles", 0));
    mSectionTrajectory.setElevationDampeningCycles(
        mAudioProcessorValueTreeState.state.getProperty("dampeningCyclesAlt", 0));
    mElevationTrajectoryManager.setPositionDampeningCycles(
        mAudioProcessorValueTreeState.state.getProperty("dampeningCyclesAlt", 0));
    mSectionTrajectory.setDeviationPerCycle(mAudioProcessorValueTreeState.state.getProperty("deviationPerCycle", 0));
    mPositionTrajectoryManager.setDeviationPerCycle(
        Degrees{ mAudioProcessorValueTreeState.state.getProperty("deviationPerCycle", 0) });
    mSectionTrajectory.setCycleDuration(mAudioProcessorValueTreeState.state.getProperty("cycleDuration", 5.0));
    mSectionTrajectory.setDurationUnit(mAudioProcessorValueTreeState.state.getProperty("durationUnit", 1));

    // Update the position preset box.
    //--------------------------------
    auto const savedPresets{ mProcessor.getPresetsManager().getSavedPresets() };
    int index{ 1 };
    for (auto const saved : savedPresets) {
        mPositionPresetComponent.presetSaved(index++, saved);
    }

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
void ControlGrisAudioProcessorEditor::updateSourceLinkCombo(PositionSourceLink value)
{
    auto action = [=]() {
        mSectionTrajectory.getPositionSourceLinkCombo().setSelectedId(static_cast<int>(value),
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
        mSectionTrajectory.getElevationSourceLinkCombo().setSelectedId(static_cast<int>(value),
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
    mSectionTrajectory.setSpatMode(mode);
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
    auto const isNewSourceCount{ mProcessor.getSources().size() != numOfSources };
    auto const currentPositionSourceLink{ mPositionTrajectoryManager.getSourceLink() };
    auto const symmetricLinkAllowed{ numOfSources == 2 };
    mSectionTrajectory.setSymmetricLinkComboState(symmetricLinkAllowed);
    if (!symmetricLinkAllowed) {
        auto const isCurrentPositionSourceLinkSymmetric{ currentPositionSourceLink == PositionSourceLink::symmetricX
                                                         || currentPositionSourceLink
                                                                == PositionSourceLink::symmetricY };
        if (isCurrentPositionSourceLinkSymmetric) {
            mProcessor.setPositionSourceLink(PositionSourceLink::independent, SourceLinkEnforcer::OriginOfChange::user);
        }
    }
    mSelectedSource = {};
    mProcessor.setNumberOfSources(numOfSources);
    mSectionGeneralSettings.setNumberOfSources(numOfSources);
    mSectionTrajectory.setNumberOfSources(numOfSources);
    mSectionSourceSpan.setSelectedSource(&mProcessor.getSources()[mSelectedSource]);
    mPositionField.refreshSources();
    mElevationField.refreshSources();
    mSectionSourcePosition.setNumberOfSources(numOfSources, mProcessor.getFirstSourceId());
    if (isNewSourceCount) {
        sourcesPlacementChangedCallback(SourcePlacement::leftAlternate);
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
    // cache source link before we position all the sources
    auto const cachedSourceLink{ mPositionTrajectoryManager.getSourceLink() };
    mProcessor.setPositionSourceLink(PositionSourceLink::independent, SourceLinkEnforcer::OriginOfChange::automation);

    auto const isCubeMode{ mProcessor.getSpatMode() == SpatMode::cube };
    auto const distance{ isCubeMode ? 0.7f : 1.0f };
    auto const numOfSources{mProcessor.getSources().size()};
    auto const increment{360.0f / numOfSources};
    auto curOddAzimuth{ 0.0f + increment / 2 };
    auto curEvenAzimuth{ 360.0f - increment / 2 };

    auto const getAzimuthValue = [sourcePlacement, numOfSources, increment, &curOddAzimuth, &curEvenAzimuth](int const sourceIndex) {
        switch (sourcePlacement) {
        case SourcePlacement::leftAlternate:
            return (sourceIndex % 2 == 0) ? std::exchange(curEvenAzimuth, curEvenAzimuth - increment)
                                          : std::exchange(curOddAzimuth, curOddAzimuth + increment);
        case SourcePlacement::rightAlternate:
            return (sourceIndex % 2 == 0) ? std::exchange(curOddAzimuth, curOddAzimuth + increment)
                                          : std::exchange(curEvenAzimuth, curEvenAzimuth - increment);
        case SourcePlacement::leftClockwise:
            return 360.0f / numOfSources * sourceIndex - increment / 2;
        case SourcePlacement::leftCounterClockwise:
            return 360.0f / numOfSources * -sourceIndex - increment / 2;
        case SourcePlacement::rightClockwise:
            return 360.0f / numOfSources * sourceIndex + increment / 2;
        case SourcePlacement::rightCounterClockwise:
            return 360.0f / numOfSources * -sourceIndex + increment / 2;
        case SourcePlacement::topClockwise:
            return 360.0f / numOfSources * sourceIndex;
        case SourcePlacement::topCounterClockwise:
            return 360.0f / numOfSources * -sourceIndex;
        case SourcePlacement::undefined:
        default:
            jassertfalse;
            return 0.f;
        }
    };

    //position all sources
    for (auto i = 0; i < numOfSources; ++i) {
        auto & source{ mProcessor.getSources()[i] };
        auto const elevation{ isCubeMode ? source.getElevation() : MAX_ELEVATION };
        auto const azimuth{Degrees{ getAzimuthValue(i) }};
        source.setCoordinates(azimuth, elevation, distance, Source::OriginOfChange::userAnchorMove);
    }

    // TODO: why are we storing the _normalized_ positions in the processor?
    //then as a second pass, give the processor the normalized positions
    for (SourceIndex i{}; i < SourceIndex{ numOfSources }; ++i) {
        auto const & source{ mProcessor.getSources()[i] };
        mProcessor.setSourceParameterValue(i, SourceParameter::azimuth, source.getNormalizedAzimuth().get());
        mProcessor.setSourceParameterValue(i, SourceParameter::elevation, source.getNormalizedElevation().get());
        mProcessor.setSourceParameterValue(i, SourceParameter::distance, source.getDistance());
    }

    // update selected source
    mSectionSourcePosition.updateSelectedSource(&mProcessor.getSources()[mSelectedSource], SourceIndex{}, mProcessor.getSpatMode());
    mPositionTrajectoryManager.setTrajectoryType(mPositionTrajectoryManager.getTrajectoryType(), mProcessor.getSources().getPrimarySource().getPos());

    //set source link back to its cached value
    mProcessor.setPositionSourceLink(cachedSourceLink, SourceLinkEnforcer::OriginOfChange::automation);

    repaint();
}

//==============================================================================
std::pair<float, float> getAzimuthAndElevationFromPosition(float x, float y, float z)
{
    //this first part is the constructor from PolarVector
    auto const length = std::sqrt(x * x + y * y + z * z);
    if ((x == 0.0f && y == 0.0f) || length == 0.0f)
        return {};

    auto elevation = HALF_PI - Radians{ std::acos(std::clamp(z / length, -1.0f, 1.0f)) };
    auto azimuth = Radians{std::acos(std::clamp(x / std::sqrt(x * x + y * y), -1.0f, 1.0f))* (y < 0.0f ? -1.0f : 1.0f)};

    //then translate by pi/2
    azimuth = HALF_PI - azimuth;
    elevation = HALF_PI - elevation;

    //and at this point we have the azimuth and elevation sent to SpatGRIS
    return { azimuth.get(), elevation.get() };
}

juce::Point<float> getXyFromAzimuthAndElevation(float azimuth, float elevation, SpatMode spatMode)
{
    // first part taken from void Source::computeXY()
    float const radius{ [&] {
        if (spatMode == SpatMode::dome) { // azimuth - elevation
            auto const result{ elevation / MAX_ELEVATION.get() };
            jassert(result >= 0.0f && result <= 1.0f);
            return result;
        }
        //woof, not too sure here, probably need some other logic for non-dome
        jassertfalse;
        return 0.f;
        //jassert(!std::isnan(mDistance));
        //return mDistance;
    }() };

    auto const position = Source::getPositionFromAngle(Radians{ azimuth }, radius);

    // these other manipulations are from ControlGrisAudioProcessor::parameterChanged() and
    // Source::computeAzimuthElevation()
    return { (position.x + 1) / 2, 1 - ((position.y + 1) / 2) };
}

class GetAzimuthAndElevationFromPositionTest : public juce::UnitTest
{
public:
    GetAzimuthAndElevationFromPositionTest() : juce::UnitTest("GetAzimuthAndElevationFromPosition Test") {}

    void runTest() override
    {
        beginTest("Test with (0, 0.640747, 0.767752)");
        {
            auto const [azim, elev] = getAzimuthAndElevationFromPosition(0.f, 0.640747f, 0.767752f);
            expectWithinAbsoluteError(azim, 0.f, 0.001f);
            expectWithinAbsoluteError(elev, 0.69547f, 0.001f);

            auto const cartesianPosition = getXyFromAzimuthAndElevation(azim, elev, SpatMode::dome);
            expectWithinAbsoluteError(cartesianPosition.x, .5f, 0.001f);
            expectWithinAbsoluteError(cartesianPosition.y, 0.721375f, 0.001f);
        }
    }
};

static GetAzimuthAndElevationFromPositionTest getAzimuthAndElevationFromPositionTest;

//==============================================================================
void ControlGrisAudioProcessorEditor::speakerSetupSelectedCallback(const juce::File& speakerSetupFile)
{
    auto const showError = [](juce::String error) {
        juce::AlertWindow::showMessageBox(juce::AlertWindow::WarningIcon,
                                          "Error Loading Speaker Setup File",
                                          error,
                                          "OK");
    };

    // make sure our file exists
    if (!speakerSetupFile.existsAsFile()) {
        showError("This file does not exist: " + speakerSetupFile.getFullPathName());
        return;
    }

    // and is a valid speaker setup file
    auto const speakerSetup = juce::ValueTree::fromXml(speakerSetupFile.loadFileAsString());
    if (!speakerSetup.isValid() || speakerSetup.getType().toString() != "SPEAKER_SETUP" || speakerSetup.getNumChildren() < 1) {
        showError("This file is not a valid Speaker Setup file: " + speakerSetupFile.getFullPathName());
        return;
    }

    // now convert the speakerSetupFile to the xml that the preset manager is expecting
    juce::XmlElement speakerSetupXml{ "ITEM" };
    speakerSetupXml.setAttribute("ID", -1); // we're not using an ID here, that's the OG presets

    int curSourceNumber = -1;

    for (auto curSpeaker : speakerSetup) {
        // skip this speaker if it's direct out only
        if (static_cast<int> (curSpeaker["DIRECT_OUT_ONLY"]) == 1)
            continue;

        ++curSourceNumber;

        // speakerPosition has coordinates from -1 to 1, which we need to convert to 0 to 1
        auto const speakerPosition{ curSpeaker.getChildWithName("POSITION") };
        auto const x{ (static_cast<double>(speakerPosition["X"]) + 1) / 2 };
        double const y{ (static_cast<double>(speakerPosition["Y"]) + 1) / 2 };
        double const z{ (static_cast<double>(speakerPosition["Z"]) + 1) / 2 };

        speakerSetupXml.setAttribute("S" + juce::String(curSourceNumber + 1) + "_X", x);
        speakerSetupXml.setAttribute("S" + juce::String(curSourceNumber + 1) + "_Y", y);
        speakerSetupXml.setAttribute("S" + juce::String(curSourceNumber + 1) + "_Z", z);
        if (curSourceNumber == 0) {
            speakerSetupXml.setAttribute("S" + juce::String(curSourceNumber + 1) + "_terminal_X", x);
            speakerSetupXml.setAttribute("S" + juce::String(curSourceNumber + 1) + "_terminal_Y", y);
            speakerSetupXml.setAttribute("S" + juce::String(curSourceNumber + 1) + "_terminal_Z", z);
        }
    }

    speakerSetupXml.setAttribute("numberOfSources", curSourceNumber);

    // and finally, load the speaker setup as a preset
    mProcessor.getPresetsManager().load(speakerSetupXml);
    numberOfSourcesChangedCallback(mProcessor.getSources().size());
    mProcessor.updatePrimarySourceParameters(Source::ChangeType::position);
    if (mProcessor.getSpatMode() == SpatMode::cube) {
        mProcessor.updatePrimarySourceParameters(Source::ChangeType::elevation);
    }
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
// SectionTrajectory::Listener callbacks.
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
}

//==============================================================================
void ControlGrisAudioProcessorEditor::elevationTrajectoryStateChangedCallback(bool value)
{
    mElevationTrajectoryManager.setPositionActivateState(value);
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

    if (mSectionTrajectory.getPositionActivateState() != mPositionTrajectoryManager.getPositionActivateState()) {
        mSectionTrajectory.setPositionActivateState(mPositionTrajectoryManager.getPositionActivateState());
    }
    if (mSectionTrajectory.getElevationActivateState() != mElevationTrajectoryManager.getPositionActivateState()) {
        mSectionTrajectory.setElevationActivateState(mElevationTrajectoryManager.getPositionActivateState());
    }
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
    numberOfSourcesChangedCallback(mProcessor.getSources().size());

    if (auto const presetSourceId {mProcessor.getPresetsManager().getPresetSourceId(presetNumber)})
        firstSourceIdChangedCallback(SourceId{*presetSourceId});

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

    mMainBanner.setBounds(0, 0, fieldSize, 20);
    mPositionField.setBounds(0, 20, fieldSize, fieldSize);

    if (mProcessor.getSpatMode() == SpatMode::cube) {
        mMainBanner.setText("X - Y", juce::NotificationType::dontSendNotification);
        mElevationBanner.setVisible(true);
        mElevationField.setVisible(true);
        mElevationModeCombobox.setVisible(true);
        mElevationBanner.setBounds(fieldSize, 0, fieldSize, 20);
        mElevationModeLabel.setVisible(true);
        mElevationModeCombobox.setBounds(fieldSize + mElevationBanner.getBounds().getWidth() / 2,
                                         (mElevationBanner.getHeight() - mElevationModeCombobox.getHeight()) / 2,
                                         (mElevationBanner.getBounds().getWidth() / 2) - 4,
                                         16);
        mElevationModeLabel.setBounds(mElevationModeCombobox.getBounds().getX() - 60,
                                      (mElevationBanner.getHeight() - mElevationModeLabel.getHeight()) / 2,
                                      60,
                                      12);
        mElevationField.setBounds(fieldSize, 20, fieldSize, fieldSize);
    } else {
        mMainBanner.setText("Azimuth - Elevation", juce::NotificationType::dontSendNotification);
        mElevationBanner.setVisible(false);
        mElevationModeLabel.setVisible(false);
        mElevationModeCombobox.setVisible(false);
        mElevationField.setVisible(false);
    }

    mSectionSourceSpan.setBounds(0, fieldSize + 20, width, 50);

    mTrajectoryBanner.setBounds(0, fieldSize + 70, width, 20);
    mSectionTrajectory.setBounds(0, fieldSize + 90, width, 160);

    mSettingsBanner.setBounds(0, fieldSize + 250, width, 20);
    mConfigurationComponent.setBounds(0, fieldSize + 270, width, 160);

    mLastUiWidth = getWidth();
    mLastUiHeight = getHeight();

    mPositionPresetBanner.setBounds(width, 0, 50, 20);
    mPositionPresetComponent.setBounds(width, 20, 50, height - 20);
}

//==============================================================================
void ControlGrisAudioProcessorEditor::setSpatMode(SpatMode spatMode)
{
    mSectionSourcePosition.setSpatMode(spatMode);
}

} // namespace gris
