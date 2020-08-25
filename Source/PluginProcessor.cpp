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

#include "PluginProcessor.h"

#include "PluginEditor.h"

enum class FixedPositionType { terminal, initial };

//==============================================================================
// The parameter Layout creates the automatable mAudioProcessorValueTreeState.
AudioProcessorValueTreeState::ParameterLayout createParameterLayout()
{
    using Parameter = AudioProcessorValueTreeState::Parameter;

    std::vector<std::unique_ptr<Parameter>> parameters;

    parameters.push_back(std::make_unique<Parameter>(Automation::Ids::x,
                                                     String("Recording Trajectory X"),
                                                     String(),
                                                     NormalisableRange<float>(0.0f, 1.0f),
                                                     0.0f,
                                                     nullptr,
                                                     nullptr));
    parameters.push_back(std::make_unique<Parameter>(Automation::Ids::y,
                                                     String("Recording Trajectory Y"),
                                                     String(),
                                                     NormalisableRange<float>(0.0f, 1.0f),
                                                     0.0f,
                                                     nullptr,
                                                     nullptr));
    parameters.push_back(std::make_unique<Parameter>(Automation::Ids::z,
                                                     String("Recording Trajectory Z"),
                                                     String(),
                                                     NormalisableRange<float>(0.0f, 1.0f),
                                                     0.0f,
                                                     nullptr,
                                                     nullptr));

    parameters.push_back(std::make_unique<Parameter>(
        Automation::Ids::positionSourceLink,
        String("Source Link"),
        String(),
        NormalisableRange<float>(0.0f, static_cast<float>(POSITION_SOURCE_LINK_TYPES.size() - 1), 1.0f),
        0.0f,
        nullptr,
        nullptr,
        false,
        true,
        true));
    parameters.push_back(std::make_unique<Parameter>(Automation::Ids::elevationSourceLink,
                                                     String("Source Link Alt"),
                                                     String(),
                                                     NormalisableRange<float>(0.0f, 4.0f, 1.0f),
                                                     0.0f,
                                                     nullptr,
                                                     nullptr,
                                                     false,
                                                     true,
                                                     true));

    parameters.push_back(std::make_unique<Parameter>(Automation::Ids::positionPreset,
                                                     String("Position Preset"),
                                                     String(),
                                                     NormalisableRange<float>(0.0f, 50.0f, 1.0f),
                                                     0.0f,
                                                     nullptr,
                                                     nullptr,
                                                     false,
                                                     true,
                                                     true));

    parameters.push_back(std::make_unique<Parameter>(Automation::Ids::azimuthSpan,
                                                     String("Azimuth Span"),
                                                     String(),
                                                     NormalisableRange<float>(0.0f, 1.0f),
                                                     0.0f,
                                                     nullptr,
                                                     nullptr));
    parameters.push_back(std::make_unique<Parameter>(Automation::Ids::elevationSpan,
                                                     String("Elevation Span"),
                                                     String(),
                                                     NormalisableRange<float>(0.0f, 1.0f),
                                                     0.0f,
                                                     nullptr,
                                                     nullptr));

    return { parameters.begin(), parameters.end() };
}

//==============================================================================
ControlGrisAudioProcessor::ControlGrisAudioProcessor()
    :
#ifndef JucePlugin_PreferredChannelConfigurations
    AudioProcessor(BusesProperties()
    #if !JucePlugin_IsMidiEffect
        #if !JucePlugin_IsSynth
                       .withInput("Input", AudioChannelSet::stereo(), true)
        #endif // JucePlugin_IsSynth
                       .withOutput("Output", AudioChannelSet::stereo(), true)
    #endif // JucePlugin_IsMidiEffect
                       )
    ,
#endif // JucePlugin_PreferredChannelConfigurations
    mAudioProcessorValueTreeState(*this, nullptr, Identifier(JucePlugin_Name), createParameterLayout())

{
    setLatencySamples(0);

    // Size of the plugin window.
    mAudioProcessorValueTreeState.state.addChild({ "uiState", { { "width", 650 }, { "height", 700 } }, {} },
                                                 -1,
                                                 nullptr);

    // Global setting mAudioProcessorValueTreeState.
    mAudioProcessorValueTreeState.state.setProperty("oscFormat", 0, nullptr);
    mAudioProcessorValueTreeState.state.setProperty("oscPortNumber", 18032, nullptr);
    mAudioProcessorValueTreeState.state.setProperty("oscConnected", true, nullptr);
    mAudioProcessorValueTreeState.state.setProperty("oscInputPortNumber", 9000, nullptr);
    mAudioProcessorValueTreeState.state.setProperty("oscInputConnected", false, nullptr);
    mAudioProcessorValueTreeState.state.setProperty("oscOutputAddress", "192.168.1.100", nullptr);
    mAudioProcessorValueTreeState.state.setProperty("oscOutputPortNumber", 8000, nullptr);
    mAudioProcessorValueTreeState.state.setProperty("oscOutputConnected", false, nullptr);
    mAudioProcessorValueTreeState.state.setProperty("numberOfSources", 2, nullptr);
    mAudioProcessorValueTreeState.state.setProperty("firstSourceId", 1, nullptr);
    mAudioProcessorValueTreeState.state.setProperty("oscOutputPluginId", 1, nullptr);

    // Trajectory box persitent settings.
    mAudioProcessorValueTreeState.state.setProperty("trajectoryType",
                                                    static_cast<int>(PositionTrajectoryType::realtime),
                                                    nullptr);
    mAudioProcessorValueTreeState.state.setProperty("trajectoryTypeAlt",
                                                    static_cast<int>(ElevationTrajectoryType::realtime),
                                                    nullptr);
    mAudioProcessorValueTreeState.state.setProperty("backAndForth", false, nullptr);
    mAudioProcessorValueTreeState.state.setProperty("backAndForthAlt", false, nullptr);
    mAudioProcessorValueTreeState.state.setProperty("dampeningCycles", 0, nullptr);
    mAudioProcessorValueTreeState.state.setProperty("dampeningCyclesAlt", 0, nullptr);
    mAudioProcessorValueTreeState.state.setProperty("deviationPerCycle", 0, nullptr);
    mAudioProcessorValueTreeState.state.setProperty("cycleDuration", 5, nullptr);
    mAudioProcessorValueTreeState.state.setProperty("durationUnit", 1, nullptr);

    // Per source mAudioProcessorValueTreeState. Because there is no attachment to the automatable
    // mAudioProcessorValueTreeState, we need to keep track of the current parameter values to be
    // able to reload the last state of the plugin when we close/open the UI.
    for (int i{}; i < MAX_NUMBER_OF_SOURCES; ++i) {
        String oscId(i);
        // Non-automatable, per source, mAudioProcessorValueTreeState.
        mAudioProcessorValueTreeState.state.setProperty(String("p_azimuth_") + oscId,
                                                        i % 2 == 0 ? -90.0 : 90.0,
                                                        nullptr);
        mAudioProcessorValueTreeState.state.setProperty(String("p_elevation_") + oscId, 0.0, nullptr);
        mAudioProcessorValueTreeState.state.setProperty(String("p_distance_") + oscId, 1.0, nullptr);
        mAudioProcessorValueTreeState.state.setProperty(String("p_x_") + oscId, 0.0, nullptr);
        mAudioProcessorValueTreeState.state.setProperty(String("p_y_") + oscId, 0.0, nullptr);

        // Gives the source an initial id...
        mSources.get(i).setId(SourceId{ i + mFirstSourceId.toInt() });
        // .. and coordinates.
        auto const azimuth{ i % 2 == 0 ? Degrees{ -45.0f } : Degrees{ 45.0f } };
        mSources.get(i).setCoordinates(azimuth, MAX_ELEVATION, 1.0f, SourceLinkNotification::silent);
    }

    auto * paramX{ mAudioProcessorValueTreeState.getParameter(Automation::Ids::x) };
    if (paramX != nullptr) {
        paramX->setValue(mSources.getPrimarySource().getPos().getX());
    }
    auto * paramY{ mAudioProcessorValueTreeState.getParameter(Automation::Ids::y) };
    if (paramY != nullptr) {
        paramY->setValue(mSources.getPrimarySource().getPos().y);
    }

    // Automation values for the recording trajectory.
    mAudioProcessorValueTreeState.addParameterListener(Automation::Ids::x, this);
    mAudioProcessorValueTreeState.addParameterListener(Automation::Ids::y, this);
    mAudioProcessorValueTreeState.addParameterListener(Automation::Ids::z, this);
    mAudioProcessorValueTreeState.addParameterListener(Automation::Ids::positionSourceLink, this);
    mAudioProcessorValueTreeState.addParameterListener(Automation::Ids::elevationSourceLink, this);
    mAudioProcessorValueTreeState.addParameterListener(Automation::Ids::positionPreset, this);
    mAudioProcessorValueTreeState.addParameterListener(Automation::Ids::azimuthSpan, this);
    mAudioProcessorValueTreeState.addParameterListener(Automation::Ids::elevationSpan, this);

    mPositionAutomationManager.addListener(this);
    mElevationAutomationManager.addListener(this);

    // The timer's callback send OSC messages periodically.
    //-----------------------------------------------------
    startTimerHz(50);
}

//==============================================================================
ControlGrisAudioProcessor::~ControlGrisAudioProcessor()
{
    disconnectOSC();
}

//==============================================================================
void ControlGrisAudioProcessor::parameterChanged(String const & parameterID, float const newValue)
{
    if (std::isnan(newValue) || std::isinf(newValue)) {
        return;
    }

    Normalized const normalized{ newValue };
    if (parameterID.compare(Automation::Ids::x) == 0) {
        mSources.getPrimarySource().setX(normalized, SourceLinkNotification::notify);
    } else if (parameterID.compare(Automation::Ids::y) == 0) {
        Normalized const invNormalized{ 1.0f - newValue };
        mSources.getPrimarySource().setY(invNormalized, SourceLinkNotification::notify);
    } else if (parameterID.compare(Automation::Ids::z) == 0 && mSpatMode == SpatMode::cube) {
        auto const newElevation{ MAX_ELEVATION - (MAX_ELEVATION * normalized.toFloat()) };
        mSources.getPrimarySource().setElevation(newElevation, SourceLinkNotification::notify);
    }

    if (parameterID.compare(Automation::Ids::positionSourceLink) == 0) {
        auto const val{ static_cast<PositionSourceLink>(static_cast<int>(newValue) + 1) };
        setPositionSourceLink(val);
    }

    if (parameterID.compare(Automation::Ids::elevationSourceLink) == 0) {
        auto const val{ static_cast<ElevationSourceLink>(static_cast<int>(newValue) + 1) };
        if (val != mElevationAutomationManager.getSourceLink()) {
            setElevationSourceLink(val);
            auto * ed{ dynamic_cast<ControlGrisAudioProcessorEditor *>(getActiveEditor()) };
            if (ed != nullptr) {
                ed->updateElevationSourceLinkCombo(static_cast<ElevationSourceLink>(val));
            }
        }
    }

    if (parameterID.compare(Automation::Ids::positionPreset) == 0) {
        auto const value{ static_cast<int>(newValue) };
        auto const loaded{ mPresetManager.loadIfPresetChanged(value) };
        if (loaded) {
            mPositionAutomationManager.recomputeTrajectory();
            mElevationAutomationManager.recomputeTrajectory();
        }
    }

    if (parameterID.startsWith(Automation::Ids::azimuthSpan)) {
        for (auto & source : mSources) {
            source.setAzimuthSpan(normalized);
        }
    } else if (parameterID.startsWith(Automation::Ids::elevationSpan)) {
        for (auto & source : mSources) {
            source.setElevationSpan(normalized);
        }
    }
}

//========================================================
void ControlGrisAudioProcessor::setPositionSourceLink(PositionSourceLink const newSourceLink)
{
    if (newSourceLink != mPositionAutomationManager.getSourceLink()) {
        mPositionAutomationManager.setSourceLink(newSourceLink);

        auto * editor{ dynamic_cast<ControlGrisAudioProcessorEditor *>(getActiveEditor()) };
        if (editor != nullptr) {
            editor->updateSourceLinkCombo(newSourceLink);
        }

        mPositionSourceLinkEnforcer.setSourceLink(newSourceLink);
        mPositionSourceLinkEnforcer.enforceSourceLink();
    }
}

//==============================================================================
void ControlGrisAudioProcessor::setElevationSourceLink(ElevationSourceLink newSourceLink)
{
    if (newSourceLink != mElevationAutomationManager.getSourceLink()) {
        mElevationAutomationManager.setSourceLink(newSourceLink);

        auto * ed{ dynamic_cast<ControlGrisAudioProcessorEditor *>(getActiveEditor()) };
        if (ed != nullptr) {
            ed->updateElevationSourceLinkCombo(newSourceLink);
        }

        mElevationSourceLinkEnforcer.setSourceLink(newSourceLink);
        mElevationSourceLinkEnforcer.enforceSourceLink();
    }
}

//==============================================================================
void ControlGrisAudioProcessor::setSpatMode(SpatMode const spatMode)
{
    mSpatMode = spatMode;
    mAudioProcessorValueTreeState.state.setProperty("oscFormat", static_cast<int>(mSpatMode), nullptr);
    for (int i{}; i < MAX_NUMBER_OF_SOURCES; ++i) {
        mSources.get(i).setSpatMode(spatMode);
    }

    if (spatMode == SpatMode::dome) {
        // remove cube-specific gadgets
        mElevationSourceLinkEnforcer.setSourceLink(ElevationSourceLink::independent);
    } else {
        jassert(spatMode == SpatMode::cube);
        mElevationSourceLinkEnforcer.setSourceLink(mElevationAutomationManager.getSourceLink());
    }
}

//==============================================================================
void ControlGrisAudioProcessor::setOscPortNumber(int const oscPortNumber)
{
    mCurrentOSCPort = oscPortNumber;
    mAudioProcessorValueTreeState.state.setProperty("oscPortNumber", mCurrentOSCPort, nullptr);
}

//==============================================================================
void ControlGrisAudioProcessor::setFirstSourceId(SourceId const firstSourceId, bool const propagate)
{
    mFirstSourceId = firstSourceId;
    mAudioProcessorValueTreeState.state.setProperty("firstSourceId", mFirstSourceId.toInt(), nullptr);
    for (int i{}; i < MAX_NUMBER_OF_SOURCES; ++i) {
        mSources.get(i).setId(SourceId{ i + mFirstSourceId.toInt() });
    }

    if (propagate) {
        sendOscMessage();
    }
}

//==============================================================================
void ControlGrisAudioProcessor::setNumberOfSources(int const numOfSources, bool const propagate)
{
    mSources.setSize(numOfSources);
    mAudioProcessorValueTreeState.state.setProperty("numberOfSources", mSources.size(), nullptr);

    mPositionSourceLinkEnforcer.numberOfSourcesChanged();
    mElevationSourceLinkEnforcer.numberOfSourcesChanged();

    if (propagate) {
        sendOscMessage();
    }
}

//==============================================================================
bool ControlGrisAudioProcessor::createOscConnection(int oscPort)
{
    disconnectOSC();

    mOscConnected = mOscSender.connect("127.0.0.1", oscPort);
    if (!mOscConnected)
        std::cout << "Error: could not connect to UDP port " << oscPort << "." << std::endl;
    else
        mLastConnectedOSCPort = oscPort;

    return mOscConnected;
}

//==============================================================================
bool ControlGrisAudioProcessor::disconnectOSC()
{
    if (mOscConnected) {
        if (mOscSender.disconnect()) {
            mOscConnected = false;
            mLastConnectedOSCPort = -1;
        }
    }
    return !mOscConnected;
}

//==============================================================================
void ControlGrisAudioProcessor::handleOscConnection(bool const state)
{
    if (state) {
        if (mLastConnectedOSCPort != mCurrentOSCPort) {
            createOscConnection(mCurrentOSCPort);
        }
    } else {
        disconnectOSC();
    }
    mAudioProcessorValueTreeState.state.setProperty("oscConnected", getOscConnected(), nullptr);
}

//==============================================================================
void ControlGrisAudioProcessor::sendOscMessage()
{
    if (!mOscConnected)
        return;

    OSCAddressPattern const oscPattern("/spat/serv");
    OSCMessage message(oscPattern);

    for (auto const & source : mSources) {
        auto const azimuth{ source.getAzimuth().getAsRadians() };
        auto const elevation{ source.getElevation().getAsRadians() };
        auto const azimuthSpan{ source.getAzimuthSpan() * 2.0f };
        auto const elevationSpan{ source.getElevationSpan() * 0.5f };
        auto const distance{ mSpatMode == SpatMode::cube ? source.getDistance() / 0.6f : source.getDistance() };

        message.clear();
        message.addInt32(source.getId().toInt() - 1); // osc id starts at 0
        message.addFloat32(azimuth);
        message.addFloat32(elevation);
        message.addFloat32(azimuthSpan);
        message.addFloat32(elevationSpan);
        message.addFloat32(distance);
        message.addFloat32(0.0);

        auto const success{ mOscSender.send(message) };
        jassert(success);
    }
}

//==============================================================================
bool ControlGrisAudioProcessor::createOscInputConnection(int const oscPort)
{
    disconnectOSCInput(oscPort);

    mOscInputConnected = mOscInputReceiver.connect(oscPort);
    if (!mOscInputConnected) {
        std::cout << "Error: could not connect to UDP input port " << oscPort << "." << std::endl;
    } else {
        mOscInputReceiver.addListener(this);
        mCurrentOSCInputPort = oscPort;
        mAudioProcessorValueTreeState.state.setProperty("oscInputPortNumber", oscPort, nullptr);
    }

    mAudioProcessorValueTreeState.state.setProperty("oscInputConnected", getOscInputConnected(), nullptr);

    return mOscInputConnected;
}

//==============================================================================
bool ControlGrisAudioProcessor::disconnectOSCInput(int const oscPort)
{
    if (mOscInputConnected) {
        if (mOscInputReceiver.disconnect()) {
            mOscInputConnected = false;
        }
    }

    mAudioProcessorValueTreeState.state.setProperty("oscInputPortNumber", oscPort, nullptr);
    mAudioProcessorValueTreeState.state.setProperty("oscInputConnected", getOscInputConnected(), nullptr);

    return !mOscInputConnected;
}

//==============================================================================
void ControlGrisAudioProcessor::oscBundleReceived(OSCBundle const & bundle)
{
    for (auto const & element : bundle) {
        if (element.isMessage())
            oscMessageReceived(element.getMessage());
        else if (element.isBundle())
            oscBundleReceived(element.getBundle());
    }
}

//==============================================================================
void ControlGrisAudioProcessor::oscMessageReceived(OSCMessage const & message)
{
    auto positionSourceLinkToProcess{ PositionSourceLink::undefined };
    auto elevationSourceLinkToProcess{ ElevationSourceLink::undefined };
    auto x{ -1.0f };
    auto y{ -1.0f };
    auto z{ -1.0f };
    auto const address{ message.getAddressPattern().toString() };
    auto const pluginInstance{ juce::String{ "/controlgris/" } + juce::String{ getOscOutputPluginId() } };
    if ((address == String(pluginInstance + "/traj/1/x") || address == String(pluginInstance + "/traj/1/xyz/1"))
        && mPositionAutomationManager.getTrajectoryType() == PositionTrajectoryType::realtime) {
        x = message[0].getFloat32();
    } else if ((address == String(pluginInstance + "/traj/1/y") || address == String(pluginInstance + "/traj/1/xyz/2"))
               && mPositionAutomationManager.getTrajectoryType() == PositionTrajectoryType::realtime) {
        y = message[0].getFloat32();
    } else if ((address == String(pluginInstance + "/traj/1/z") || address == String(pluginInstance + "/traj/1/xyz/3"))
               && static_cast<ElevationTrajectoryType>(mElevationAutomationManager.getTrajectoryType())
                      == ElevationTrajectoryType::realtime) {
        z = message[0].getFloat32();
    } else if (address == String(pluginInstance + "/traj/1/xy")
               && mPositionAutomationManager.getTrajectoryType() == PositionTrajectoryType::realtime) {
        x = message[0].getFloat32();
        y = message[1].getFloat32();
    } else if (address == String(pluginInstance + "/traj/1/xyz")) {
        if (mPositionAutomationManager.getTrajectoryType() == PositionTrajectoryType::realtime) {
            x = message[0].getFloat32();
            y = message[1].getFloat32();
        }
        if (static_cast<ElevationTrajectoryType>(mElevationAutomationManager.getTrajectoryType())
            == ElevationTrajectoryType::realtime) {
            z = message[2].getFloat32();
        }
    } else if (address == String(pluginInstance + "/azispan")) {
        for (auto & source : mSources) {
            source.setAzimuthSpan(Normalized{ message[0].getFloat32() });
        }
        auto const gestureLock{ mChangeGesturesManager.getScopedLock(Automation::Ids::azimuthSpan) };
        mAudioProcessorValueTreeState.getParameter(Automation::Ids::azimuthSpan)
            ->setValueNotifyingHost(message[0].getFloat32());
    } else if (address == String(pluginInstance + "/elespan")) {
        for (auto & source : mSources)
            source.setElevationSpan(Normalized{ message[0].getFloat32() });
        auto const gestureLock{ mChangeGesturesManager.getScopedLock(Automation::Ids::elevationSpan) };
        mAudioProcessorValueTreeState.getParameter(Automation::Ids::elevationSpan)
            ->setValueNotifyingHost(message[0].getFloat32());
    } else if (address == String(pluginInstance + "/sourcelink/1/1")) {
        if (message[0].getFloat32() == 1)
            positionSourceLinkToProcess = static_cast<PositionSourceLink>(1);
    } else if (address == String(pluginInstance + "/sourcelink/2/1")) {
        if (message[0].getFloat32() == 1)
            positionSourceLinkToProcess = static_cast<PositionSourceLink>(2);
    } else if (address == String(pluginInstance + "/sourcelink/3/1")) {
        if (message[0].getFloat32() == 1)
            positionSourceLinkToProcess = static_cast<PositionSourceLink>(3);
    } else if (address == String(pluginInstance + "/sourcelink/4/1")) {
        if (message[0].getFloat32() == 1)
            positionSourceLinkToProcess = static_cast<PositionSourceLink>(4);
    } else if (address == String(pluginInstance + "/sourcelink/5/1")) {
        if (message[0].getFloat32() == 1)
            positionSourceLinkToProcess = static_cast<PositionSourceLink>(5);
    } else if (address == String(pluginInstance + "/sourcelink/6/1")) {
        if (message[0].getFloat32() == 1)
            positionSourceLinkToProcess = static_cast<PositionSourceLink>(6);
    } else if (address == String(pluginInstance + "/sourcelink")) {
        positionSourceLinkToProcess = static_cast<PositionSourceLink>(message[0].getFloat32()); // 1 -> 6
    } else if (address == String(pluginInstance + "/sourcelinkalt/1/1")) {
        if (message[0].getFloat32() == 1)
            elevationSourceLinkToProcess = static_cast<ElevationSourceLink>(1);
    } else if (address == String(pluginInstance + "/sourcelinkalt/2/1")) {
        if (message[0].getFloat32() == 1)
            elevationSourceLinkToProcess = static_cast<ElevationSourceLink>(2);
    } else if (address == String(pluginInstance + "/sourcelinkalt/3/1")) {
        if (message[0].getFloat32() == 1)
            elevationSourceLinkToProcess = static_cast<ElevationSourceLink>(3);
    } else if (address == String(pluginInstance + "/sourcelinkalt/4/1")) {
        if (message[0].getFloat32() == 1)
            elevationSourceLinkToProcess = static_cast<ElevationSourceLink>(4);
    } else if (address == String(pluginInstance + "/sourcelinkalt/5/1")) {
        if (message[0].getFloat32() == 1)
            elevationSourceLinkToProcess = static_cast<ElevationSourceLink>(5);
    } else if (address == String(pluginInstance + "/sourcelinkalt")) {
        elevationSourceLinkToProcess = static_cast<ElevationSourceLink>(message[0].getFloat32()); // 1 -> 5
    } else if (address == String(pluginInstance + "/presets")) {
        int newPreset = (int)message[0].getFloat32(); // 1 -> 50
        auto const loaded{ mPresetManager.loadIfPresetChanged(newPreset) };
        if (loaded) {
            mPositionAutomationManager.recomputeTrajectory();
            mElevationAutomationManager.recomputeTrajectory();
        }
        auto * ed{ dynamic_cast<ControlGrisAudioProcessorEditor *>(getActiveEditor()) };
        if (ed != nullptr) {
            ed->updatePositionPreset(newPreset);
        }
    }

    if (x != -1.0f && y != -1.0f) {
        mSources.getPrimarySource().setPos(Point<float>{ x, y }, SourceLinkNotification::notify);
        sourcePositionChanged(SourceIndex{ 0 }, 0);
        mPresetManager.loadIfPresetChanged(0);
    } else if (y != -1.0f) {
        mSources.getPrimarySource().setY(y, SourceLinkNotification::notify);
        sourcePositionChanged(SourceIndex{ 0 }, 0);
        mPresetManager.loadIfPresetChanged(0);
    } else if (x != -1.0f) {
        mSources.getPrimarySource().setX(x, SourceLinkNotification::notify);
        sourcePositionChanged(SourceIndex{ 0 }, 0);
        mPresetManager.loadIfPresetChanged(0);
    }

    if (z != -1.0f) {
        mSources.getPrimarySource().setY(z, SourceLinkNotification::notify);
        mElevationAutomationManager.sendTrajectoryPositionChangedEvent();
        mPresetManager.loadIfPresetChanged(0);
    }

    if (static_cast<bool>(positionSourceLinkToProcess)) {
        setPositionSourceLink(positionSourceLinkToProcess);
    }

    if (static_cast<bool>(elevationSourceLinkToProcess)) {
        setElevationSourceLink(elevationSourceLinkToProcess);
    }
}

//==============================================================================
bool ControlGrisAudioProcessor::createOscOutputConnection(String const & oscAddress, int const oscPort)
{
    disconnectOSCOutput(oscAddress, oscPort);

    mOscOutputConnected = mOscOutputSender.connect(oscAddress, oscPort);
    if (!mOscOutputConnected)
        std::cout << "Error: could not connect to UDP output port " << oscPort << " on address " << oscAddress << "."
                  << std::endl;
    else {
        mCurrentOSCOutputPort = oscPort;
        mCurrentOSCOutputAddress = oscAddress;
        mAudioProcessorValueTreeState.state.setProperty("oscOutputPortNumber", oscPort, nullptr);
        mAudioProcessorValueTreeState.state.setProperty("oscOutputAddress", oscAddress, nullptr);
    }

    mAudioProcessorValueTreeState.state.setProperty("oscOutputConnected", getOscOutputConnected(), nullptr);

    return mOscOutputConnected;
}

bool ControlGrisAudioProcessor::disconnectOSCOutput(String const & oscAddress, int const oscPort)
{
    if (mOscOutputConnected) {
        if (mOscOutputSender.disconnect()) {
            mOscOutputConnected = false;
        }
    }

    mAudioProcessorValueTreeState.state.setProperty("oscOutputPortNumber", oscPort, nullptr);
    mAudioProcessorValueTreeState.state.setProperty("oscOutputAddress", oscAddress, nullptr);
    mAudioProcessorValueTreeState.state.setProperty("oscOutputConnected", getOscOutputConnected(), nullptr);

    return !mOscOutputConnected;
}

//==============================================================================
void ControlGrisAudioProcessor::setOscOutputPluginId(int const pluginId)
{
    mAudioProcessorValueTreeState.state.setProperty("oscOutputPluginId", pluginId, nullptr);
}

//==============================================================================
int ControlGrisAudioProcessor::getOscOutputPluginId() const
{
    return mAudioProcessorValueTreeState.state.getProperty("oscOutputPluginId", 1);
}

//==============================================================================
void ControlGrisAudioProcessor::sendOscOutputMessage()
{
    constexpr auto impossibleNumber{ std::numeric_limits<float>::min() };

    static auto lastTrajectoryX{ impossibleNumber };
    static auto lastTrajectoryY{ impossibleNumber };
    static auto lastTrajectoryZ{ impossibleNumber };
    static Normalized lastAzimuthSpan{ impossibleNumber };
    static Normalized lastElevationSpan{ impossibleNumber };
    static auto lastPositionLink{ PositionSourceLink::undefined };
    static auto lastElevationLink{ ElevationSourceLink::undefined };
    static auto lastPresetNumber{ std::numeric_limits<int>::min() };

    if (!mOscOutputConnected) {
        return;
    }

    OSCMessage message(OSCAddressPattern("/tmp"));

    auto const pluginInstance = juce::String{ "/controlgris/" } + String{ getOscOutputPluginId() };

    auto const trajectoryHandlePosition{ mSources.getPrimarySource().getPos() };
    auto const trajectory1x = trajectoryHandlePosition.getX();
    auto const trajectory1y = trajectoryHandlePosition.getY();
    auto const trajectory1z = trajectoryHandlePosition.getY();

    if (lastTrajectoryX != trajectory1x) {
        message.setAddressPattern(OSCAddressPattern(pluginInstance + "/traj/1/x"));
        message.addFloat32(trajectory1x);
        mOscOutputSender.send(message);
        message.clear();

        message.setAddressPattern(OSCAddressPattern(pluginInstance + "/traj/1/xyz/1"));
        message.addFloat32(trajectory1x);
        mOscOutputSender.send(message);
        message.clear();
    }

    if (lastTrajectoryY != trajectory1y) {
        message.setAddressPattern(OSCAddressPattern(pluginInstance + "/traj/1/y"));
        message.addFloat32(trajectory1y);
        mOscOutputSender.send(message);
        message.clear();

        message.setAddressPattern(OSCAddressPattern(pluginInstance + "/traj/1/xyz/2"));
        message.addFloat32(trajectory1y);
        mOscOutputSender.send(message);
        message.clear();
    }

    if (lastTrajectoryZ != trajectory1z) {
        message.setAddressPattern(OSCAddressPattern(pluginInstance + "/traj/1/z"));
        message.addFloat32(trajectory1z);
        mOscOutputSender.send(message);
        message.clear();

        message.setAddressPattern(OSCAddressPattern(pluginInstance + "/traj/1/xyz/3"));
        message.addFloat32(trajectory1z);
        mOscOutputSender.send(message);
        message.clear();
    }

    if (lastTrajectoryX != trajectory1x || lastTrajectoryY != trajectory1y || lastTrajectoryZ != trajectory1z) {
        message.setAddressPattern(OSCAddressPattern(pluginInstance + "/traj/1/xy"));
        message.addFloat32(trajectory1x);
        message.addFloat32(trajectory1y);
        mOscOutputSender.send(message);
        message.clear();

        message.setAddressPattern(OSCAddressPattern(pluginInstance + "/traj/1/xyz"));
        message.addFloat32(trajectory1x);
        message.addFloat32(trajectory1y);
        message.addFloat32(trajectory1z);
        mOscOutputSender.send(message);
        message.clear();
    }

    lastTrajectoryX = trajectory1x;
    lastTrajectoryY = trajectory1y;
    lastTrajectoryZ = trajectory1z;

    if (lastAzimuthSpan != mSources.getPrimarySource().getAzimuthSpan()) {
        message.setAddressPattern(OSCAddressPattern(pluginInstance + "/azispan"));
        message.addFloat32(mSources.getPrimarySource().getAzimuthSpan().toFloat());
        mOscOutputSender.send(message);
        message.clear();
        lastAzimuthSpan = mSources.getPrimarySource().getAzimuthSpan();
    }

    if (lastElevationSpan != mSources.getPrimarySource().getElevationSpan()) {
        message.setAddressPattern(OSCAddressPattern(pluginInstance + "/elespan"));
        message.addFloat32(mSources.getPrimarySource().getElevationSpan().toFloat());
        mOscOutputSender.send(message);
        message.clear();
        lastElevationSpan = mSources.getPrimarySource().getElevationSpan();
    }

    if (mPositionAutomationManager.getSourceLink() != lastPositionLink) {
        message.setAddressPattern(OSCAddressPattern(pluginInstance + "/sourcelink"));
        message.addInt32(static_cast<int32>(mPositionAutomationManager.getSourceLink()));
        mOscOutputSender.send(message);
        message.clear();

        String pattern = pluginInstance + String("/sourcelink/")
                         + String(static_cast<int>(mPositionAutomationManager.getSourceLink())) + String("/1");
        message.setAddressPattern(OSCAddressPattern(pattern));
        message.addInt32(1);
        mOscOutputSender.send(message);
        message.clear();

        lastPositionLink = mPositionAutomationManager.getSourceLink();
    }

    if (static_cast<ElevationSourceLink>(mElevationAutomationManager.getSourceLink()) != lastElevationLink) {
        message.setAddressPattern(OSCAddressPattern(pluginInstance + "/sourcelinkalt"));
        message.addInt32(static_cast<int32>(mElevationAutomationManager.getSourceLink()));
        mOscOutputSender.send(message);
        message.clear();

        String patternAlt = pluginInstance + String("/sourcelinkalt/")
                            + String(static_cast<int>(mElevationAutomationManager.getSourceLink())) + String("/1");
        message.setAddressPattern(OSCAddressPattern(patternAlt));
        message.addInt32(1);
        mOscOutputSender.send(message);
        message.clear();

        lastElevationLink = static_cast<ElevationSourceLink>(mElevationAutomationManager.getSourceLink());
    }

    auto const currentPreset{ mPresetManager.getCurrentPreset() };
    if (currentPreset != lastPresetNumber) {
        message.setAddressPattern(OSCAddressPattern(pluginInstance + "/presets"));
        message.addInt32(currentPreset);
        mOscOutputSender.send(message);
        message.clear();

        lastPresetNumber = currentPreset;
    }
}

//==============================================================================
void ControlGrisAudioProcessor::timerCallback()
{
    auto * editor{ dynamic_cast<ControlGrisAudioProcessorEditor *>(getActiveEditor()) };

    // automation
    if (mLastTimerTime != getCurrentTime()) {
        auto const deltaTime{ getCurrentTime() - getInitTimeOnPlay() };
        if (mPositionAutomationManager.getPositionActivateState()) {
            mPositionAutomationManager.setTrajectoryDeltaTime(deltaTime);
        }
        if (mSpatMode == SpatMode::cube && mElevationAutomationManager.getPositionActivateState()) {
            mElevationAutomationManager.setTrajectoryDeltaTime(deltaTime);
        }
    }

    mLastTimerTime = getCurrentTime();

    if (mCanStopActivate && !mIsPlaying) {
        if (mPositionAutomationManager.getPositionActivateState())
            mPositionAutomationManager.setPositionActivateState(false);
        if (mElevationAutomationManager.getPositionActivateState())
            mElevationAutomationManager.setPositionActivateState(false);
        mCanStopActivate = false;

        if (editor != nullptr) {
            editor->updateSpanLinkButton(false);
        }
    }

    if (editor != nullptr) {
        editor->refresh();
    }

    sendOscMessage();
    sendOscOutputMessage();
}

//==============================================================================
void ControlGrisAudioProcessor::setPluginState()
{
    // If no preset is loaded, try to restore the last saved positions.
    if (mPresetManager.getCurrentPreset() == 0) {
        for (auto & source : mSources) {
            auto const index{ source.getIndex().toString() };
            source.setAzimuth(
                Normalized{ mAudioProcessorValueTreeState.state.getProperty(String("p_azimuth_") + index) },
                SourceLinkNotification::notify);
            source.setElevation(
                Normalized{ mAudioProcessorValueTreeState.state.getProperty(String("p_elevation_") + index) },
                SourceLinkNotification::notify);
            source.setDistance(mAudioProcessorValueTreeState.state.getProperty(String("p_distance_") + index),
                               SourceLinkNotification::notify);
        }
    }

    auto * editor{ dynamic_cast<ControlGrisAudioProcessorEditor *>(getActiveEditor()) };
    if (editor != nullptr) {
        editor->reloadUiState();
    }

    sendOscMessage();
}

//==============================================================================
void ControlGrisAudioProcessor::sourcePositionChanged(SourceIndex sourceIndex, int whichField)
{
    auto const & source{ mSources[sourceIndex] };
    if (whichField == 0) {
        if (getSpatMode() == SpatMode::dome) {
            setSourceParameterValue(sourceIndex, SourceParameter::azimuth, source.getNormalizedAzimuth().toFloat());
            setSourceParameterValue(sourceIndex, SourceParameter::elevation, source.getNormalizedElevation().toFloat());
        } else {
            setSourceParameterValue(sourceIndex, SourceParameter::azimuth, source.getNormalizedAzimuth().toFloat());
            setSourceParameterValue(sourceIndex, SourceParameter::distance, source.getDistance());
        }
        if (source.isPrimarySource()) {
            mPositionAutomationManager.setTrajectoryType(mPositionAutomationManager.getTrajectoryType(),
                                                         mSources.getPrimarySource().getPos());
        }
    } else {
        setSourceParameterValue(sourceIndex, SourceParameter::elevation, source.getNormalizedElevation().toFloat());
        mElevationAutomationManager.setTrajectoryType(mElevationAutomationManager.getTrajectoryType());
    }

    //    // any position change invalidates current preset.
    //    setPositionPreset(0);
}

// Called whenever a source has changed.
//--------------------------------------
void ControlGrisAudioProcessor::setSourceParameterValue(SourceIndex const sourceIndex,
                                                        SourceParameter const parameterId,
                                                        float const value)
{
    Normalized const normalized{ static_cast<float>(value) };
    auto const param_id{ sourceIndex.toString() };
    auto & source{ mSources[sourceIndex] };
    switch (parameterId) {
    case SourceParameter::azimuth:
        source.setAzimuth(normalized, SourceLinkNotification::notify);
        mAudioProcessorValueTreeState.state.setProperty("p_azimuth_" + param_id, value, nullptr);
        break;
    case SourceParameter::elevation:
        source.setElevation(normalized, SourceLinkNotification::notify);
        mAudioProcessorValueTreeState.state.setProperty(String("p_elevation_") + param_id, value, nullptr);
        break;
    case SourceParameter::distance:
        source.setDistance(value, SourceLinkNotification::notify);
        mAudioProcessorValueTreeState.state.setProperty(String("p_distance_") + param_id, value, nullptr);
        break;
    case SourceParameter::x:
        source.setX(value, SourceLinkNotification::notify);
        break;
    case SourceParameter::y:
        source.setY(value, SourceLinkNotification::notify);
        break;
    case SourceParameter::azimuthSpan:
        for (auto & sourceRef : mSources) {
            sourceRef.setAzimuthSpan(normalized);
        }
        mAudioProcessorValueTreeState.getParameter(Automation::Ids::azimuthSpan)->setValueNotifyingHost(value);
        break;
    case SourceParameter::elevationSpan:
        for (auto & sourceRef : mSources) {
            sourceRef.setElevationSpan(normalized);
        }
        mAudioProcessorValueTreeState.getParameter(Automation::Ids::elevationSpan)->setValueNotifyingHost(value);
        break;
    }
}

//==============================================================================
void ControlGrisAudioProcessor::trajectoryPositionChanged(AutomationManager * manager,
                                                          Point<float> const position,
                                                          Radians const elevation)
{
    // TODO: change gestures might have to be initiated some other way ? (osc input)
    auto const normalizedPosition{ (position + Point<float>{ 1.0f, 1.0f }) / 2.0f };
    if (manager == &mPositionAutomationManager) {
        if (!isPlaying()) {
            mPositionAutomationManager.setPrimarySourcePosition(position);
            mAudioProcessorValueTreeState.getParameter(Automation::Ids::x)->setValue(normalizedPosition.getX());
            mAudioProcessorValueTreeState.getParameter(Automation::Ids::y)->setValue(1.0f - normalizedPosition.getY());
        }

        mAudioProcessorValueTreeState.getParameter(Automation::Ids::x)
            ->setValueNotifyingHost(normalizedPosition.getX());
        mAudioProcessorValueTreeState.getParameter(Automation::Ids::y)
            ->setValueNotifyingHost(1.0f - normalizedPosition.getY());

    } else if (manager == &mElevationAutomationManager) {
        auto const normalizedElevation{ 1.0f - elevation / MAX_ELEVATION };
        if (!isPlaying()) {
            mAudioProcessorValueTreeState.getParameter(Automation::Ids::z)->setValue(normalizedElevation);
        }

        mAudioProcessorValueTreeState.getParameter(Automation::Ids::z)->setValueNotifyingHost(normalizedElevation);
    }
}

//==============================================================================
String const ControlGrisAudioProcessor::getName() const
{
    return JucePlugin_Name;
}

bool ControlGrisAudioProcessor::acceptsMidi() const
{
#if JucePlugin_WantsMidiInput
    return true;
#else
    return false;
#endif
}

//==============================================================================
bool ControlGrisAudioProcessor::producesMidi() const
{
#if JucePlugin_ProducesMidiOutput
    return true;
#else
    return false;
#endif
}

//==============================================================================
bool ControlGrisAudioProcessor::isMidiEffect() const
{
#if JucePlugin_IsMidiEffect
    return true;
#else
    return false;
#endif
}

//==============================================================================
void ControlGrisAudioProcessor::initialize()
{
    mNeedsInitialization = true;
    mLastTime = mLastTimerTime = 10000000.0;
    mCanStopActivate = true;
}

//==============================================================================
void ControlGrisAudioProcessor::prepareToPlay([[maybe_unused]] double const sampleRate,
                                              [[maybe_unused]] int const samplesPerBlock)
{
    if (!mIsPlaying) {
        initialize();
    }
}

//==============================================================================
#ifndef JucePlugin_PreferredChannelConfigurations
bool ControlGrisAudioProcessor::isBusesLayoutSupported(const BusesLayout & layouts) const
{
    #if JucePlugin_IsMidiEffect
    ignoreUnused(layouts);
    return true;
    #else
    // This is the place where you check if the layout is supported.
    // In this template code we only support mono or stereo.
    if (layouts.getMainOutputChannelSet() != AudioChannelSet::mono()
        && layouts.getMainOutputChannelSet() != AudioChannelSet::stereo())
        return false;

            // This checks if the input layout matches the output layout
        #if !JucePlugin_IsSynth
    if (layouts.getMainOutputChannelSet() != layouts.getMainInputChannelSet())
        return false;
        #endif

    return true;
    #endif
}
#endif

//==============================================================================
void ControlGrisAudioProcessor::processBlock([[maybe_unused]] AudioBuffer<float> & buffer,
                                             [[maybe_unused]] MidiBuffer & midiMessages)
{
    auto const wasPlaying{ mIsPlaying };
    AudioPlayHead * audioPlayHead = getPlayHead();
    if (audioPlayHead != nullptr) {
        AudioPlayHead::CurrentPositionInfo currentPositionInfo{};
        audioPlayHead->getCurrentPosition(currentPositionInfo);
        mIsPlaying = currentPositionInfo.isPlaying;
        mBpm = currentPositionInfo.bpm;
        if (mNeedsInitialization) {
            mInitTimeOnPlay = mCurrentTime
                = currentPositionInfo.timeInSeconds < 0.0 ? 0.0 : currentPositionInfo.timeInSeconds;
            mNeedsInitialization = false;
        } else {
            mCurrentTime = currentPositionInfo.timeInSeconds;
        }
    }

    if (!wasPlaying && mIsPlaying) { // Initialization here only for Logic (also Reaper and Live), which are not
        PluginHostType hostType;     // calling prepareToPlay every time the sequence starts.
        if (hostType.isLogic() || hostType.isReaper() || hostType.isAbletonLive()) {
            initialize();
        }
    }

    // deal with trajectory recording gestures
    bool const isPositionTrajectoryActive{ mPositionAutomationManager.getPositionActivateState() };
    bool const isElevationTrajectoryActive{ mElevationAutomationManager.getPositionActivateState() };

    static bool positionGestureStarted{ false };
    static bool elevationGestureStarted{ false };

    if (isPositionTrajectoryActive && mIsPlaying && !positionGestureStarted) {
        positionGestureStarted = true;
        mChangeGesturesManager.beginGesture(Automation::Ids::x);
        mChangeGesturesManager.beginGesture(Automation::Ids::y);
    } else if ((!isPositionTrajectoryActive || !mIsPlaying) && positionGestureStarted) {
        positionGestureStarted = false;
        mChangeGesturesManager.endGesture(Automation::Ids::x);
        mChangeGesturesManager.endGesture(Automation::Ids::y);
    }
    if (mSpatMode == SpatMode::cube) {
        if (isElevationTrajectoryActive && mIsPlaying && !elevationGestureStarted) {
            elevationGestureStarted = true;
            mChangeGesturesManager.beginGesture(Automation::Ids::z);
        } else if ((!isElevationTrajectoryActive || !mIsPlaying) && elevationGestureStarted) {
            elevationGestureStarted = false;
            mChangeGesturesManager.endGesture(Automation::Ids::z);
        }
    }

    // Some manipulations might not end gestures properly and hang automation readings altogether.
    //    if (!mIsPlaying && wasPlaying) {
    //        mChangeGesturesManager.endGesture(Automation::Ids::positionPreset);
    //        mChangeGesturesManager.endGesture(Automation::Ids::positionSourceLink);
    //        mChangeGesturesManager.endGesture(Automation::Ids::elevationSourceLink);
    //        mChangeGesturesManager.endGesture(Automation::Ids::elevationSpan);
    //        mChangeGesturesManager.endGesture(Automation::Ids::azimuthSpan);
    //        mChangeGesturesManager.endGesture(Automation::Ids::x);
    //        mChangeGesturesManager.endGesture(Automation::Ids::y);
    //        mChangeGesturesManager.endGesture(Automation::Ids::z);
    //    }

    mLastTime = mCurrentTime;
}

//==============================================================================
AudioProcessorEditor * ControlGrisAudioProcessor::createEditor()
{
    return new ControlGrisAudioProcessorEditor(*this,
                                               mAudioProcessorValueTreeState,
                                               mPositionAutomationManager,
                                               mElevationAutomationManager);
}

//==============================================================================
void ControlGrisAudioProcessor::getStateInformation(MemoryBlock & destData)
{
    for (int sourceIndex{}; sourceIndex < MAX_NUMBER_OF_SOURCES; ++sourceIndex) {
        juce::String const id{ sourceIndex };
        Identifier const azimuthId{ String{ "p_azimuth_" } + id };
        Identifier const elevationId{ String{ "p_elevation_" } + id };
        Identifier const distanceId{ String{ "p_distance_" } + id };
        auto const & source{ mSources[sourceIndex] };
        auto const normalizedAzimuth{ source.getNormalizedAzimuth().toFloat() };
        auto const normalizedElevation{ source.getNormalizedElevation().toFloat() };
        auto const distance{ source.getDistance() };
        mAudioProcessorValueTreeState.state.setProperty(azimuthId + id, normalizedAzimuth, nullptr);
        mAudioProcessorValueTreeState.state.setProperty(elevationId + id, normalizedElevation, nullptr);
        mAudioProcessorValueTreeState.state.setProperty(distanceId + id, distance, nullptr);
    }

    auto state{ mAudioProcessorValueTreeState.copyState() };

    auto xmlState{ state.createXml() };

    if (xmlState != nullptr) {
        auto * childExist = xmlState->getChildByName(FIXED_POSITION_DATA_TAG);
        if (childExist) {
            xmlState->removeChildElement(childExist, true);
        }
        if (mFixPositionData.getNumChildElements() > 0) {
            auto * positionData{ new XmlElement{ mFixPositionData } };
            xmlState->addChildElement(positionData);
        }
        copyXmlToBinary(*xmlState, destData);
    }
}

//==============================================================================
void ControlGrisAudioProcessor::setStateInformation(void const * data, int const sizeInBytes)
{
    MessageManagerLock mmLock{};

    auto xmlState{ getXmlFromBinary(data, sizeInBytes) };

    if (xmlState != nullptr) {
        // Set global settings values.
        //----------------------------
        ValueTree valueTree = ValueTree::fromXml(*xmlState);
        setSpatMode((SpatMode)(int)valueTree.getProperty("oscFormat", 0));
        setOscPortNumber(valueTree.getProperty("oscPortNumber", 18032));
        handleOscConnection(valueTree.getProperty("oscConnected", true));
        setNumberOfSources(valueTree.getProperty("numberOfSources", 1), false);
        setFirstSourceId(SourceId{ valueTree.getProperty("firstSourceId", 1) });
        setOscOutputPluginId(valueTree.getProperty("oscOutputPluginId", 1));

        if (valueTree.getProperty("oscInputConnected", false)) {
            createOscInputConnection(valueTree.getProperty("oscInputPortNumber", 9000));
        }

        if (valueTree.getProperty("oscOutputConnected", false)) {
            createOscOutputConnection(valueTree.getProperty("oscOutputAddress", "192.168.1.100"),
                                      valueTree.getProperty("oscOutputPortNumber", 8000));
        }

        // Load saved fixed positions.
        //----------------------------
        XmlElement * positionData = xmlState->getChildByName(FIXED_POSITION_DATA_TAG);
        if (positionData) {
            mFixPositionData.deleteAllChildElements();
            mFixPositionData = *positionData;
        }
        // Replace the state and call automated parameter current values.
        //---------------------------------------------------------------
        mAudioProcessorValueTreeState.replaceState(ValueTree::fromXml(*xmlState));
    }

    setPluginState();
}

//==============================================================================
// This creates new instances of the plugin..
AudioProcessor * JUCE_CALLTYPE createPluginFilter()
{
    return new ControlGrisAudioProcessor{};
}
