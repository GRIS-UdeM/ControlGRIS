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

#include "cg_ControlGrisAudioProcessor.hpp"

#include "cg_ControlGrisAudioProcessorEditor.hpp"
#include "cg_Source.hpp"
#include "cg_TrajectoryManager.hpp"

namespace gris
{
//==============================================================================
// The parameter Layout creates the automatable mAudioProcessorValueTreeState.
juce::AudioProcessorValueTreeState::ParameterLayout createParameterLayout()
{
    using Parameter = juce::AudioProcessorValueTreeState::Parameter;
    using Attributes = juce::AudioProcessorValueTreeStateParameterAttributes;

    juce::AudioProcessorValueTreeState::ParameterLayout layout;

    layout.add(
        std::make_unique<Parameter>(juce::ParameterID{ Automation::Ids::X, 1 },
                                    juce::String("Recording Trajectory X"),
                                    juce::NormalisableRange<float>(0.0f, 1.0f),
                                    0.0f,
                                    Attributes()),
        std::make_unique<Parameter>(juce::ParameterID{ Automation::Ids::Y, 1 },
                                    juce::String("Recording Trajectory Y"),
                                    juce::NormalisableRange<float>(0.0f, 1.0f),
                                    0.0f,
                                    Attributes()),
        std::make_unique<Parameter>(juce::ParameterID{ Automation::Ids::Z, 1 },
                                    juce::String("Recording Trajectory Z"),
                                    juce::NormalisableRange<float>(0.0f, 1.0f),
                                    0.0f,
                                    Attributes()),
        std::make_unique<Parameter>(
            juce::ParameterID{ Automation::Ids::POSITION_SOURCE_LINK, 1 },
            juce::String("Source Link"),
            juce::NormalisableRange<float>(0.0f, static_cast<float>(POSITION_SOURCE_LINK_TYPES.size() - 1), 1.0f),
            0.0f,
            Attributes().withMeta(false).withAutomatable(true).withDiscrete(true)),
        std::make_unique<Parameter>(juce::ParameterID{ Automation::Ids::ELEVATION_SOURCE_LINK, 1 },
                                    juce::String("Source Link Alt"),
                                    juce::NormalisableRange<float>(0.0f, 4.0f, 1.0f),
                                    0.0f,
                                    Attributes().withMeta(false).withAutomatable(true).withDiscrete(true)),
        std::make_unique<Parameter>(juce::ParameterID{ Automation::Ids::POSITION_PRESET, 1 },
                                    juce::String("Position Preset"),
                                    juce::NormalisableRange<float>(1.0f, 50.0f, 1.0f),
                                    1.0f,
                                    Attributes().withMeta(false).withAutomatable(true).withDiscrete(true)),
        std::make_unique<Parameter>(juce::ParameterID{ Automation::Ids::AZIMUTH_SPAN, 1 },
                                    juce::String("Azimuth Span"),
                                    juce::NormalisableRange<float>(0.0f, 1.0f),
                                    0.0f,
                                    Attributes()),
        std::make_unique<Parameter>(juce::ParameterID{ Automation::Ids::ELEVATION_SPAN, 1 },
                                    juce::String("Elevation Span"),
                                    juce::NormalisableRange<float>(0.0f, 1.0f),
                                    0.0f,
                                    Attributes()),
        std::make_unique<Parameter>(
            juce::ParameterID{ Automation::Ids::ELEVATION_MODE, 1 },
            juce::String("Elevation Mode"),
            juce::NormalisableRange<float>(0.0f, static_cast<float>(ELEVATION_MODE_TYPES.size() - 1), 1.0f),
            0.0f,
            Attributes().withDiscrete(true).withAutomatable(true)));

    return layout;
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
    mAudioProcessorValueTreeState(*this, nullptr, juce::Identifier(JucePlugin_Name), createParameterLayout())
    , mAzimuthDome(mAudioProcessorValueTreeState, mParamFunctions)
    , mElevationDome(mAudioProcessorValueTreeState, mParamFunctions)
    , mHSpanDome(mAudioProcessorValueTreeState, mParamFunctions)
    , mVSpanDome(mAudioProcessorValueTreeState, mParamFunctions)
    , mXCube(mAudioProcessorValueTreeState, mParamFunctions)
    , mYCube(mAudioProcessorValueTreeState, mParamFunctions)
    , mZCube(mAudioProcessorValueTreeState, mParamFunctions)
    , mHSpanCube(mAudioProcessorValueTreeState, mParamFunctions)
    , mVSpanCube(mAudioProcessorValueTreeState, mParamFunctions)
    , mSpatParametersDomeRefs{ &mAzimuthDome, &mElevationDome, &mHSpanDome, &mVSpanDome }
    , mSpatParametersCubeRefs{ &mXCube, &mYCube, &mZCube, &mHSpanCube, &mVSpanCube }
    , mSpatParametersDomeValueRefs{ &mAzimuthDomeValue, &mElevationDomeValue, &mHspanDomeValue, &mVspanDomeValue }
    , mSpatParametersCubeValueRefs{ &mXCubeValue, &mYCubeValue, &mZCubeValue, &mHspanCubeValue, &mVspanCubeValue }
    , mDomeOnsetDetectionRefs{ &mOnsetDetectionAzimuth,
                               &mOnsetDetectionElevation,
                               &mOnsetDetectionHSpan,
                               &mOnsetDetectionVSpan }
    , mCubeOnsetDetectionRefs{ &mOnsetDetectionX,
                               &mOnsetDetectionY,
                               &mOnsetDetectionZ,
                               &mOnsetDetectionHSpan,
                               &mOnsetDetectionVSpan }

{
#if JUCE_DEBUG
    juce::UnitTestRunner testRunner;
    testRunner.runAllTests();
#endif

    setLatencySamples(0);

    // Size of the plugin window.
    mAudioProcessorValueTreeState.state.addChild({ "uiState", { { "width", 650 }, { "height", 730 } }, {} },
                                                 -1,
                                                 nullptr);

    // Global setting mAudioProcessorValueTreeState.
    mAudioProcessorValueTreeState.state.setProperty("oscFormat", 0, nullptr);
    mAudioProcessorValueTreeState.state.setProperty("oscPortNumber", 18032, nullptr);
    mAudioProcessorValueTreeState.state.setProperty("oscAddress", "127.0.0.1", nullptr);
    mAudioProcessorValueTreeState.state.setProperty("oscConnected", true, nullptr);
    mAudioProcessorValueTreeState.state.setProperty("oscInputPortNumber", 9000, nullptr);
    mAudioProcessorValueTreeState.state.setProperty("oscInputConnected", false, nullptr);
    mAudioProcessorValueTreeState.state.setProperty("oscOutputAddress", "192.168.1.100", nullptr);
    mAudioProcessorValueTreeState.state.setProperty("oscOutputPortNumber", 8000, nullptr);
    mAudioProcessorValueTreeState.state.setProperty("oscOutputConnected", false, nullptr);
    mAudioProcessorValueTreeState.state.setProperty("numberOfSources", 2, nullptr);
    mAudioProcessorValueTreeState.state.setProperty(PRESET_FIRST_SOURCE_ID_XML_TAG, 1, nullptr);
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

    mSources.init(this);
    mPositionSourceLinkEnforcer.numberOfSourcesChanged();
    mElevationSourceLinkEnforcer.numberOfSourcesChanged();
    // Per source mAudioProcessorValueTreeState. Because there is no attachment to the automatable
    // mAudioProcessorValueTreeState, we need to keep track of the current parameter values to be
    // able to reload the last state of the plugin when we close/open the UI.
    for (int i{}; i < mSources.MAX_NUMBER_OF_SOURCES; ++i) {
        juce::String oscId(i);
        // Non-automatable, per source, mAudioProcessorValueTreeState.
        mAudioProcessorValueTreeState.state.setProperty(juce::String("p_azimuth_") + oscId,
                                                        i % 2 == 0 ? -90.0 : 90.0,
                                                        nullptr);
        mAudioProcessorValueTreeState.state.setProperty(juce::String("p_elevation_") + oscId, 0.0, nullptr);
        mAudioProcessorValueTreeState.state.setProperty(juce::String("p_distance_") + oscId, 1.0, nullptr);
        mAudioProcessorValueTreeState.state.setProperty(juce::String("p_x_") + oscId, 0.0, nullptr);
        mAudioProcessorValueTreeState.state.setProperty(juce::String("p_y_") + oscId, 0.0, nullptr);

        // Gives the source an initial id...
        auto & source{ mSources.get(i) };
        source.setId(SourceId{ i + mFirstSourceId.get() });
        // .. and colour
        source.setColorFromIndex(mSources.size());
        // .. and coordinates.
        auto const azimuth{ i % 2 == 0 ? Degrees{ -45.0f } : Degrees{ 45.0f } };
        source.setCoordinates(Radians{ azimuth },
                              Radians{ MAX_ELEVATION },
                              1.0f,
                              Source::OriginOfChange::userAnchorMove);
    }

    auto * paramX{ mAudioProcessorValueTreeState.getParameter(Automation::Ids::X) };
    if (paramX != nullptr) {
        paramX->setValue(mSources.getPrimarySource().getPos().getX());
    }
    auto * paramY{ mAudioProcessorValueTreeState.getParameter(Automation::Ids::Y) };
    if (paramY != nullptr) {
        paramY->setValue(mSources.getPrimarySource().getPos().y);
    }

    // Automation values for the recording trajectory.
    mAudioProcessorValueTreeState.addParameterListener(Automation::Ids::X, this);
    mAudioProcessorValueTreeState.addParameterListener(Automation::Ids::Y, this);
    mAudioProcessorValueTreeState.addParameterListener(Automation::Ids::Z, this);
    mAudioProcessorValueTreeState.addParameterListener(Automation::Ids::POSITION_SOURCE_LINK, this);
    mAudioProcessorValueTreeState.addParameterListener(Automation::Ids::ELEVATION_SOURCE_LINK, this);
    mAudioProcessorValueTreeState.addParameterListener(Automation::Ids::POSITION_PRESET, this);
    mAudioProcessorValueTreeState.addParameterListener(Automation::Ids::AZIMUTH_SPAN, this);
    mAudioProcessorValueTreeState.addParameterListener(Automation::Ids::ELEVATION_SPAN, this);
    mAudioProcessorValueTreeState.addParameterListener(Automation::Ids::ELEVATION_MODE, this);

    // The timer's callback send OSC messages periodically.
    //-----------------------------------------------------
    startTimerHz(50);
}

//==============================================================================
ControlGrisAudioProcessor::~ControlGrisAudioProcessor()
{
    [[maybe_unused]] auto const success{ disconnectOsc() };
}

//==============================================================================
void ControlGrisAudioProcessor::parameterChanged(juce::String const & parameterId, float const newValue)
{
    if (std::isnan(newValue) || std::isinf(newValue)) {
        jassertfalse;
        return;
    }

    Normalized const normalized{ newValue };
#if DEBUG_COORDINATES
    DBG("newValue: " + juce::String(newValue));
    DBG("normalized: " + juce::String(normalized.get()));
#endif
    if (parameterId.compare(Automation::Ids::X) == 0) {
        mSources.getPrimarySource().setX(normalized, Source::OriginOfChange::automation);
    } else if (parameterId.compare(Automation::Ids::Y) == 0) {
        Normalized const invNormalized{ 1.0f - newValue };
#if DEBUG_COORDINATES
        DBG("invNormalized: " + juce::String(invNormalized.get()));
#endif
        mSources.getPrimarySource().setY(invNormalized, Source::OriginOfChange::automation);
    } else if (parameterId.compare(Automation::Ids::Z) == 0 && mSpatMode == SpatMode::cube) {
        auto const newElevation{ MAX_ELEVATION - (MAX_ELEVATION * normalized.get()) };
        mSources.getPrimarySource().setElevation(Radians{ newElevation }, Source::OriginOfChange::automation);
    }

    if (parameterId.compare(Automation::Ids::POSITION_SOURCE_LINK) == 0) {
        auto const val{ static_cast<PositionSourceLink>(newValue + 1.0f) };
        setPositionSourceLink(val, SourceLinkEnforcer::OriginOfChange::automation);
    }

    if (parameterId.compare(Automation::Ids::ELEVATION_SOURCE_LINK) == 0) {
        auto const val{ static_cast<ElevationSourceLink>(newValue + 1.0f) };
        setElevationSourceLink(val, SourceLinkEnforcer::OriginOfChange::automation);
    }

    if (parameterId.compare(Automation::Ids::POSITION_PRESET) == 0) {
        auto const value{ static_cast<int>(newValue) };
        mPresetManager.loadIfPresetChanged(value);
    }

    if (parameterId.startsWith(Automation::Ids::AZIMUTH_SPAN)) {
        for (auto & source : mSources) {
            source.setAzimuthSpan(normalized);
        }
    } else if (parameterId.startsWith(Automation::Ids::ELEVATION_SPAN)) {
        for (auto & source : mSources) {
            source.setElevationSpan(normalized);
        }
    }

    if (parameterId.compare(Automation::Ids::ELEVATION_MODE) == 0) {
        mElevationMode = static_cast<ElevationMode>(newValue);
        auto * editor{ dynamic_cast<ControlGrisAudioProcessorEditor *>(getActiveEditor()) };
        if (editor != nullptr) {
            editor->updateElevationMode(mElevationMode);
        }
    }
}

//==============================================================================
void ControlGrisAudioProcessor::setPositionSourceLink(PositionSourceLink newSourceLink,
                                                      SourceLinkEnforcer::OriginOfChange const originOfChange)
{
    auto const isSymmetricLink{ newSourceLink == PositionSourceLink::symmetricX
                                || newSourceLink == PositionSourceLink::symmetricY };
    if (mSources.size() != 2 && isSymmetricLink) {
        newSourceLink = PositionSourceLink::independent;
    }

    mPositionTrajectoryManager.setSourceLink(newSourceLink);

    auto * editor{ dynamic_cast<ControlGrisAudioProcessorEditor *>(getActiveEditor()) };
    if (editor != nullptr) {
        editor->updateSourceLinkCombo(newSourceLink);
    }

    mPositionSourceLinkEnforcer.setSourceLink(newSourceLink, originOfChange);
    mPositionSourceLinkEnforcer.enforceSourceLink();
}

//==============================================================================
void ControlGrisAudioProcessor::setElevationSourceLink(ElevationSourceLink const newSourceLink,
                                                       SourceLinkEnforcer::OriginOfChange const originOfChange)
{
    mElevationTrajectoryManager.setSourceLink(newSourceLink);

    auto * ed{ dynamic_cast<ControlGrisAudioProcessorEditor *>(getActiveEditor()) };
    if (ed != nullptr) {
        ed->updateElevationSourceLinkCombo(newSourceLink);
    }

    mElevationSourceLinkEnforcer.setSourceLink(newSourceLink, originOfChange);
    mElevationSourceLinkEnforcer.enforceSourceLink();
}

//==============================================================================
void ControlGrisAudioProcessor::setElevationSourceLinkScale(double scale)
{
    mElevationSourceLinkEnforcer.setElevationSourceLinkScale(scale);
    mElevationSourceLinkEnforcer.enforceSourceLink();
}

//==============================================================================
void ControlGrisAudioProcessor::setSpatMode(SpatMode const spatMode)
{
    mSpatMode = spatMode;
    mAudioProcessorValueTreeState.state.setProperty("oscFormat", static_cast<int>(mSpatMode), nullptr);
    for (int i{}; i < mSources.MAX_NUMBER_OF_SOURCES; ++i) {
        mSources.get(i).setSpatMode(spatMode);
    }

    if (spatMode == SpatMode::dome) {
        // remove cube-specific gadgets
        mElevationSourceLinkEnforcer.setSourceLink(ElevationSourceLink::independent,
                                                   SourceLinkEnforcer::OriginOfChange::automation);
        for (auto& source : mSources) {
            // make sure source position is clipped to Dome limits when switching from Cube to Dome
            source.setPosition(source.getPos(), Source::OriginOfChange::userAnchorMove);
        }
    } else {
        jassert(spatMode == SpatMode::cube);
        mElevationSourceLinkEnforcer.setSourceLink(mElevationTrajectoryManager.getSourceLink(),
                                                   SourceLinkEnforcer::OriginOfChange::automation);
    }

    auto * editor{ dynamic_cast<ControlGrisAudioProcessorEditor *>(getActiveEditor()) };
    if (editor) {
        editor->setSpatMode(spatMode);
    }
}

//==============================================================================
void ControlGrisAudioProcessor::setOscPortNumber(int const oscPortNumber)
{
    mCurrentOscPort = oscPortNumber;
    mAudioProcessorValueTreeState.state.setProperty("oscPortNumber", oscPortNumber, nullptr);
    [[maybe_unused]] auto const success{ createOscConnection(mCurrentOscAddress, oscPortNumber) };
}

//==============================================================================
void ControlGrisAudioProcessor::setOscAddress(juce::String const & address)
{
    mCurrentOscAddress = address;
    mAudioProcessorValueTreeState.state.setProperty("oscAddress", address, nullptr);
    [[maybe_unused]] auto const success{ createOscConnection(address, mCurrentOscPort) };
}

//==============================================================================
void ControlGrisAudioProcessor::setFirstSourceId(SourceId const firstSourceId, bool const propagate)
{
    mFirstSourceId = firstSourceId;
    mAudioProcessorValueTreeState.state.setProperty(PRESET_FIRST_SOURCE_ID_XML_TAG, mFirstSourceId.get(), nullptr);
    for (int i{}; i < mSources.MAX_NUMBER_OF_SOURCES; ++i) {
        mSources.get(i).setId(SourceId{ i + mFirstSourceId.get() });
    }

    if (propagate) {
        sendOscMessage();
    }
}

//==============================================================================
void ControlGrisAudioProcessor::setNumberOfSources(int const numOfSources, bool const propagate)
{
    if (numOfSources == mSources.size()) {
        return;
    }

    mSources.setSize(numOfSources);
    mAudioProcessorValueTreeState.state.setProperty("numberOfSources", mSources.size(), nullptr);

    mPositionSourceLinkEnforcer.numberOfSourcesChanged();
    mElevationSourceLinkEnforcer.numberOfSourcesChanged();

    auto const positionSourceLink{ mPositionTrajectoryManager.getSourceLink() };
    auto const isSymmetricLink{ positionSourceLink == PositionSourceLink::symmetricX
                                || positionSourceLink == PositionSourceLink::symmetricY };
    if (isSymmetricLink && numOfSources != 2) {
        setPositionSourceLink(PositionSourceLink::independent, SourceLinkEnforcer::OriginOfChange::automation);
    }

    if (propagate) {
        sendOscMessage();
    }
}

//==============================================================================
bool ControlGrisAudioProcessor::createOscConnection(juce::String const & address, int const oscPort)
{
    if (!disconnectOsc()) {
        return false;
    }

    mOscConnected = mOscSender.connect(address, oscPort);
    if (!mOscConnected) {
        std::cout << "Error: could not connect to UDP port " << oscPort << " at address " << address << std::endl;
        return false;
    }

    mLastConnectedOscPort = oscPort;

    return true;
}

//==============================================================================
bool ControlGrisAudioProcessor::disconnectOsc()
{
    if (mOscConnected) {
        if (mOscSender.disconnect()) {
            mOscConnected = false;
            mLastConnectedOscPort = -1;
        }
    }
    return !mOscConnected;
}

//==============================================================================
void ControlGrisAudioProcessor::setOscActive(bool const state)
{
    mOscActivated = state;

    if (state) {
        if (mLastConnectedOscPort != mCurrentOscPort || !mOscConnected) {
            mOscConnected = createOscConnection(mCurrentOscAddress, mCurrentOscPort);
        }
    } else {
        mOscConnected = !disconnectOsc();
    }
    mAudioProcessorValueTreeState.state.setProperty("oscConnected", isOscActive(), nullptr);
}

//==============================================================================
void ControlGrisAudioProcessor::sendOscMessage()
{
    if (!mOscConnected || mNeedsInitialization || !mOscActivated) {
        return;
    }

    juce::OSCAddressPattern const oscPattern("/spat/serv");
    juce::OSCMessage message(oscPattern);

    if (mSpatMode == SpatMode::cube) {
        auto constexpr Z_MIN_IN{ 0.0f };
        auto constexpr Z_MAX_IN{ HALF_PI.get() };
        float Z_MIN_OUT{};
        float Z_MAX_OUT{};

        switch (mElevationMode) {
        case gris::ElevationMode::normal:
            Z_MIN_OUT = 1.0f;
            Z_MAX_OUT = 0.0f;
            break;
        case gris::ElevationMode::extendedTop:
            Z_MIN_OUT = LBAP_FAR_FIELD;
            Z_MAX_OUT = 0.0f;
            break;
        case gris::ElevationMode::extendedTopAndBottom:
            Z_MIN_OUT = LBAP_FAR_FIELD;
            Z_MAX_OUT = 1.0f - LBAP_FAR_FIELD;
            break;
        default:
            break;
        }

        for (auto const & source : mSources) {
            auto const cartesianMessage{ juce::String("car") };
            auto const x{ source.getX() * LBAP_FAR_FIELD };
            auto const y{ source.getY() * -LBAP_FAR_FIELD };
            auto const z{ (source.getElevation().getAsRadians() - Z_MIN_IN) * (Z_MAX_OUT - Z_MIN_OUT)
                              / (Z_MAX_IN - Z_MIN_IN)
                          + Z_MIN_OUT };
            auto const azimuthSpan{ source.getAzimuthSpan() };
            auto const elevationSpan{ source.getElevationSpan() };

            message.clear();
            message.addString(cartesianMessage);
            message.addInt32(source.getId().get());
            message.addFloat32(x);
            message.addFloat32(y);
            message.addFloat32(z);
            message.addFloat32(azimuthSpan.get());
            message.addFloat32(elevationSpan.get());

            [[maybe_unused]] auto const success{ mOscSender.send(message) };
            jassert(success);
        }
    } else {
        for (auto const & source : mSources) {
#if DEBUG_COORDINATES
            auto const azim{ source.getAzimuth().getAsRadians() };
            auto const normAzim{ source.getNormalizedAzimuth().get() };

            auto const elev{ source.getElevation().getAsRadians() };
            auto const normElev{ source.getNormalizedElevation().get() };

            DBG("azim: " + juce::String(azim));
            DBG("normAzim: " + juce::String(normAzim));
            DBG("elev: " + juce::String(elev));
            DBG("normElev: " + juce::String(normElev));
#endif
            auto const azimuth{ source.getAzimuth().getAsRadians() };
            auto const elevation{ source.getElevation().getAsRadians() };
            auto const azimuthSpan{ source.getAzimuthSpan() * 2.0f };
            auto const elevationSpan{ source.getElevationSpan() * 0.5f };
            auto const distance{ mSpatMode == SpatMode::cube ? source.getDistance() / 0.6f : source.getDistance() };

            message.clear();
            message.addInt32(source.getId().get() - 1); // osc id starts at 0
            message.addFloat32(azimuth);
            message.addFloat32(elevation);
            message.addFloat32(azimuthSpan.get());
            message.addFloat32(elevationSpan.get());
            message.addFloat32(distance);
            message.addFloat32(0.0); // gain ?

            [[maybe_unused]] auto const success{ mOscSender.send(message) };
            jassert(success);
        }
    }
}

//==============================================================================
bool ControlGrisAudioProcessor::createOscInputConnection(int const oscPort)
{
    [[maybe_unused]] auto const success{ disconnectOscInput(oscPort) };

    mOscInputConnected = mOscInputReceiver.connect(oscPort);
    if (!mOscInputConnected) {
        std::cout << "Error: could not connect to UDP input port " << oscPort << "." << std::endl;
    } else {
        mOscInputReceiver.addListener(this);
        mCurrentOscInputPort = oscPort;
        mAudioProcessorValueTreeState.state.setProperty("oscInputPortNumber", oscPort, nullptr);
    }

    mAudioProcessorValueTreeState.state.setProperty("oscInputConnected", getOscInputConnected(), nullptr);

    return mOscInputConnected;
}

//==============================================================================
bool ControlGrisAudioProcessor::disconnectOscInput(int const oscPort)
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
void ControlGrisAudioProcessor::oscBundleReceived(juce::OSCBundle const & bundle)
{
    for (auto const & element : bundle) {
        if (element.isMessage())
            oscMessageReceived(element.getMessage());
        else if (element.isBundle())
            oscBundleReceived(element.getBundle());
    }
}

//==============================================================================
void ControlGrisAudioProcessor::oscMessageReceived(juce::OSCMessage const & message)
{
    if (!message[0].isFloat32()) {
        return;
    }

    auto positionSourceLinkToProcess{ PositionSourceLink::undefined };
    auto elevationSourceLinkToProcess{ ElevationSourceLink::undefined };
    std::optional<float> x{};
    std::optional<float> y{};
    std::optional<float> z{};
    auto const address{ message.getAddressPattern().toString() };
    auto const pluginInstance{ juce::String{ "/controlgris/" } + juce::String{ getOscOutputPluginId() } };
    auto const positionTrajectory{ mPositionTrajectoryManager.getTrajectoryType() };
    auto const elevationTrajectory{ mElevationTrajectoryManager.getTrajectoryType() };

    if ((address == pluginInstance + "/traj/1/x" || address == pluginInstance + "/traj/1/xyz/1")
        && positionTrajectory == PositionTrajectoryType::realtime) {
        x = message[0].getFloat32();
    } else if ((address == pluginInstance + "/traj/1/y" || address == pluginInstance + "/traj/1/xyz/2")
               && positionTrajectory == PositionTrajectoryType::realtime) {
        y = 1.0f - message[0].getFloat32();
    } else if ((address == pluginInstance + "/traj/1/z" || address == pluginInstance + "/traj/1/xyz/3")
               && elevationTrajectory == ElevationTrajectoryType::realtime) {
        z = 1.0f - message[0].getFloat32();
    } else if (address == pluginInstance + "/traj/1/xy" && positionTrajectory == PositionTrajectoryType::realtime) {
        x = message[0].getFloat32();
        y = 1.0f - message[1].getFloat32();
    } else if (address == pluginInstance + "/traj/1/xyz") {
        if (positionTrajectory == PositionTrajectoryType::realtime) {
            x = message[0].getFloat32();
            y = 1.0f - message[1].getFloat32();
        }
        if (elevationTrajectory == ElevationTrajectoryType::realtime) {
            z = 1.0f - message[2].getFloat32();
        }
    } else if (address == pluginInstance + "/azispan") {
        for (auto & source : mSources) {
            source.setAzimuthSpan(Normalized{ message[0].getFloat32() });
        }
        auto const gestureLock{ mChangeGesturesManager.getScopedLock(Automation::Ids::AZIMUTH_SPAN) };
        mAudioProcessorValueTreeState.getParameter(Automation::Ids::AZIMUTH_SPAN)
            ->setValueNotifyingHost(message[0].getFloat32());
    } else if (address == pluginInstance + "/elespan") {
        for (auto & source : mSources)
            source.setElevationSpan(Normalized{ message[0].getFloat32() });
        auto const gestureLock{ mChangeGesturesManager.getScopedLock(Automation::Ids::ELEVATION_SPAN) };
        mAudioProcessorValueTreeState.getParameter(Automation::Ids::ELEVATION_SPAN)
            ->setValueNotifyingHost(message[0].getFloat32());
    } else if (address == pluginInstance + "/sourcelink/1/1") {
        if (message[0].getFloat32() == 1.0f)
            positionSourceLinkToProcess = PositionSourceLink::independent;
    } else if (address == pluginInstance + "/sourcelink/2/1") {
        if (message[0].getFloat32() == 1.0f)
            positionSourceLinkToProcess = PositionSourceLink::circular;
    } else if (address == pluginInstance + "/sourcelink/3/1") {
        if (message[0].getFloat32() == 1.0f)
            positionSourceLinkToProcess = PositionSourceLink::circularFixedRadius;
    } else if (address == pluginInstance + "/sourcelink/4/1") {
        if (message[0].getFloat32() == 1.0f)
            positionSourceLinkToProcess = PositionSourceLink::circularFixedAngle;
    } else if (address == pluginInstance + "/sourcelink/5/1") {
        if (message[0].getFloat32() == 1.0f)
            positionSourceLinkToProcess = PositionSourceLink::circularFullyFixed;
    } else if (address == pluginInstance + "/sourcelink/6/1") {
        if (message[0].getFloat32() == 1.0f)
            positionSourceLinkToProcess = PositionSourceLink::deltaLock;
    } else if (address == pluginInstance + "/sourcelink") {
        positionSourceLinkToProcess = static_cast<PositionSourceLink>(message[0].getFloat32()); // 1 -> 6
    } else if (address == pluginInstance + "/sourcelinkalt/1/1") {
        if (message[0].getFloat32() == 1.0f)
            elevationSourceLinkToProcess = ElevationSourceLink::independent;
    } else if (address == pluginInstance + "/sourcelinkalt/2/1") {
        if (message[0].getFloat32() == 1.0f)
            elevationSourceLinkToProcess = ElevationSourceLink::fixedElevation;
    } else if (address == pluginInstance + "/sourcelinkalt/3/1") {
        if (message[0].getFloat32() == 1.0f)
            elevationSourceLinkToProcess = ElevationSourceLink::linearMin;
    } else if (address == pluginInstance + "/sourcelinkalt/4/1") {
        if (message[0].getFloat32() == 1.0f)
            elevationSourceLinkToProcess = ElevationSourceLink::linearMax;
    } else if (address == pluginInstance + "/sourcelinkalt/5/1") {
        if (message[0].getFloat32() == 1.0f)
            elevationSourceLinkToProcess = ElevationSourceLink::deltaLock;
    } else if (address == pluginInstance + "/sourcelinkalt") {
        elevationSourceLinkToProcess = static_cast<ElevationSourceLink>(message[0].getFloat32()); // 1 -> 5
    } else if (address == pluginInstance + "/presets") {
        int newPreset = static_cast<int>(message[0].getFloat32()); // 1 -> 50
        auto const loaded{ mPresetManager.loadIfPresetChanged(newPreset) };
        if (loaded) {
            mPositionTrajectoryManager.recomputeTrajectory();
            mElevationTrajectoryManager.recomputeTrajectory();
        }
        auto * ed{ dynamic_cast<ControlGrisAudioProcessorEditor *>(getActiveEditor()) };
        if (ed != nullptr) {
            ed->updatePositionPreset(newPreset);
        }
    } else if (address == pluginInstance + "/elevationmode") {
        auto newElevationModeInt{ static_cast<int>(std::clamp(message[0].getFloat32(), 1.0f, 3.0f)) }; // 1 -> 3
        mElevationMode = static_cast<ElevationMode>(newElevationModeInt - 1);
        auto * ed{ dynamic_cast<ControlGrisAudioProcessorEditor *>(getActiveEditor()) };
        if (ed != nullptr) {
            ed->updateElevationMode(mElevationMode);
        }
    }

    if (x && y) {
        auto const correctedPoint{ juce::Point<float>{ *x, *y } * 2.0f - juce::Point<float>{ 1.0f, 1.0f } };
        mSources.getPrimarySource().setPosition(correctedPoint, Source::OriginOfChange::osc);
        sourcePositionChanged(SourceIndex{ 0 }, 0);
    } else if (x) {
        mSources.getPrimarySource().setX(Normalized{ *x }, Source::OriginOfChange::osc);
        sourcePositionChanged(SourceIndex{ 0 }, 0);
    } else if (y) {
        mSources.getPrimarySource().setY(Normalized{ *y }, Source::OriginOfChange::osc);
        sourcePositionChanged(SourceIndex{ 0 }, 0);
    }

    if (z) {
        mSources.getPrimarySource().setElevation(Normalized{ *z }, Source::OriginOfChange::osc);
        mElevationTrajectoryManager.sendTrajectoryPositionChangedEvent();
        sourcePositionChanged(SourceIndex{ 0 }, 1);
    }

    mPresetManager.loadIfPresetChanged(0);

    if (positionSourceLinkToProcess != PositionSourceLink::undefined) {
        setPositionSourceLink(positionSourceLinkToProcess, SourceLinkEnforcer::OriginOfChange::user);
    }

    if (elevationSourceLinkToProcess != ElevationSourceLink::undefined) {
        setElevationSourceLink(elevationSourceLinkToProcess, SourceLinkEnforcer::OriginOfChange::user);
    }
}

//==============================================================================
bool ControlGrisAudioProcessor::createOscOutputConnection(juce::String const & oscAddress, int const oscPort)
{
    [[maybe_unused]] auto const success{ disconnectOscOutput(oscAddress, oscPort) };

    mOscOutputConnected = mOscOutputSender.connect(oscAddress, oscPort);
    if (!mOscOutputConnected)
        std::cout << "Error: could not connect to UDP output port " << oscPort << " on address " << oscAddress << ".\n";
    else {
        mCurrentOscOutputPort = oscPort;
        mCurrentOscOutputAddress = oscAddress;
        mAudioProcessorValueTreeState.state.setProperty("oscOutputPortNumber", oscPort, nullptr);
        mAudioProcessorValueTreeState.state.setProperty("oscOutputAddress", oscAddress, nullptr);
    }

    mAudioProcessorValueTreeState.state.setProperty("oscOutputConnected", getOscOutputConnected(), nullptr);

    return mOscOutputConnected;
}

//==============================================================================
bool ControlGrisAudioProcessor::disconnectOscOutput(juce::String const & oscAddress, int const oscPort)
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
    if (!mOscOutputConnected) {
        return;
    }

    juce::OSCMessage message(juce::OSCAddressPattern("/tmp"));

    auto const pluginInstance = juce::String{ "/controlgris/" } + juce::String{ getOscOutputPluginId() };

    auto const trajectoryHandlePosition{ (mSources.getPrimarySource().getPos() + juce::Point<float>{ 1.0f, 1.0f })
                                         / 2.0f };
    auto const trajectory1x = trajectoryHandlePosition.getX();
    auto const trajectory1y = 1.0f - trajectoryHandlePosition.getY();
    auto const trajectory1z = 1.0f - mSources.getPrimarySource().getNormalizedElevation().get();

    if (mLastTrajectoryX != trajectory1x) {
        message.setAddressPattern(juce::OSCAddressPattern(pluginInstance + "/traj/1/x"));
        message.addFloat32(trajectory1x);
        mOscOutputSender.send(message);
        message.clear();

        message.setAddressPattern(juce::OSCAddressPattern(pluginInstance + "/traj/1/xyz/1"));
        message.addFloat32(trajectory1x);
        mOscOutputSender.send(message);
        message.clear();
    }

    if (mLastTrajectoryY != trajectory1y) {
        message.setAddressPattern(juce::OSCAddressPattern(pluginInstance + "/traj/1/y"));
        message.addFloat32(trajectory1y);
        mOscOutputSender.send(message);
        message.clear();

        message.setAddressPattern(juce::OSCAddressPattern(pluginInstance + "/traj/1/xyz/2"));
        message.addFloat32(trajectory1y);
        mOscOutputSender.send(message);
        message.clear();
    }

    if (mLastTrajectoryZ != trajectory1z) {
        message.setAddressPattern(juce::OSCAddressPattern(pluginInstance + "/traj/1/z"));
        message.addFloat32(trajectory1z);
        mOscOutputSender.send(message);
        message.clear();

        message.setAddressPattern(juce::OSCAddressPattern(pluginInstance + "/traj/1/xyz/3"));
        message.addFloat32(trajectory1z);
        mOscOutputSender.send(message);
        message.clear();
    }

    if (mLastTrajectoryX != trajectory1x || mLastTrajectoryY != trajectory1y || mLastTrajectoryZ != trajectory1z) {
        message.setAddressPattern(juce::OSCAddressPattern(pluginInstance + "/traj/1/xy"));
        message.addFloat32(trajectory1x);
        message.addFloat32(trajectory1y);
        mOscOutputSender.send(message);
        message.clear();

        message.setAddressPattern(juce::OSCAddressPattern(pluginInstance + "/traj/1/xyz"));
        message.addFloat32(trajectory1x);
        message.addFloat32(trajectory1y);
        message.addFloat32(trajectory1z);
        mOscOutputSender.send(message);
        message.clear();
    }

    mLastTrajectoryX = trajectory1x;
    mLastTrajectoryY = trajectory1y;
    mLastTrajectoryZ = trajectory1z;

    if (mLastAzimuthSpan != mSources.getPrimarySource().getAzimuthSpan()) {
        message.setAddressPattern(juce::OSCAddressPattern(pluginInstance + "/azispan"));
        message.addFloat32(mSources.getPrimarySource().getAzimuthSpan().get());
        mOscOutputSender.send(message);
        message.clear();
        mLastAzimuthSpan = mSources.getPrimarySource().getAzimuthSpan();
    }

    if (mLastElevationSpan != mSources.getPrimarySource().getElevationSpan()) {
        message.setAddressPattern(juce::OSCAddressPattern(pluginInstance + "/elespan"));
        message.addFloat32(mSources.getPrimarySource().getElevationSpan().get());
        mOscOutputSender.send(message);
        message.clear();
        mLastElevationSpan = mSources.getPrimarySource().getElevationSpan();
    }

    if (mPositionTrajectoryManager.getSourceLink() != mLastPositionLink) {
        message.setAddressPattern(juce::OSCAddressPattern(pluginInstance + "/sourcelink"));
        message.addInt32(static_cast<juce::int32>(mPositionTrajectoryManager.getSourceLink()));
        mOscOutputSender.send(message);
        message.clear();

        juce::String pattern = pluginInstance + juce::String("/sourcelink/")
                               + juce::String(static_cast<int>(mPositionTrajectoryManager.getSourceLink()))
                               + juce::String("/1");
        message.setAddressPattern(juce::OSCAddressPattern(pattern));
        message.addInt32(1);
        mOscOutputSender.send(message);
        message.clear();

        mLastPositionLink = mPositionTrajectoryManager.getSourceLink();
    }

    if (static_cast<ElevationSourceLink>(mElevationTrajectoryManager.getSourceLink()) != mLastElevationLink) {
        message.setAddressPattern(juce::OSCAddressPattern(pluginInstance + "/sourcelinkalt"));
        message.addInt32(static_cast<juce::int32>(mElevationTrajectoryManager.getSourceLink()));
        mOscOutputSender.send(message);
        message.clear();

        juce::String patternAlt = pluginInstance + juce::String("/sourcelinkalt/")
                                  + juce::String(static_cast<int>(mElevationTrajectoryManager.getSourceLink()))
                                  + juce::String("/1");
        message.setAddressPattern(juce::OSCAddressPattern(patternAlt));
        message.addInt32(1);
        mOscOutputSender.send(message);
        message.clear();

        mLastElevationLink = static_cast<ElevationSourceLink>(mElevationTrajectoryManager.getSourceLink());
    }

    auto const currentPreset{ mPresetManager.getCurrentPreset() };
    if (currentPreset != mLastPresetNumber) {
        message.setAddressPattern(juce::OSCAddressPattern(pluginInstance + "/presets"));
        message.addInt32(currentPreset);
        mOscOutputSender.send(message);
        message.clear();

        mLastPresetNumber = currentPreset;
    }

    if (mElevationMode != mLastElevationMode) {
        message.setAddressPattern(juce::OSCAddressPattern(pluginInstance + "/elevationmode"));
        message.addInt32(static_cast<juce::int32>(mElevationMode) + 1); // 1 -> 3
        mOscOutputSender.send(message);
        message.clear();
        mLastElevationMode = mElevationMode;
    }
}

//==============================================================================
void ControlGrisAudioProcessor::timerCallback()
{
    auto * editor{ dynamic_cast<ControlGrisAudioProcessorEditor *>(getActiveEditor()) };

    // automation
    if (mLastTimerTime != getCurrentTime() && mSelectedSoundTrajectoriesTabIdx == 1) {
        auto const deltaTime{ getCurrentTime() - getInitTimeOnPlay() };
        if (mPositionTrajectoryManager.getPositionActivateState()) {
            mPositionTrajectoryManager.setTrajectoryDeltaTime(deltaTime);
        }
        if (mSpatMode == SpatMode::cube && mElevationTrajectoryManager.getPositionActivateState()) {
            mElevationTrajectoryManager.setTrajectoryDeltaTime(deltaTime);
        }
    }

    mLastTimerTime = getCurrentTime();

    if (mCanStopActivate && !mIsPlaying) {
        if (mPositionTrajectoryManager.getPositionActivateState())
            mPositionTrajectoryManager.setPositionActivateState(false);
        if (mElevationTrajectoryManager.getPositionActivateState())
            mElevationTrajectoryManager.setPositionActivateState(false);
        mAudioAnalysisActivateState = false;
        mCanStopActivate = false;

        if (editor != nullptr) {
            editor->updateSpanLinkButton(false);
            editor->updateSpeedLinkButton(false);
        }
    }

    if (editor != nullptr) {
        editor->refresh();
    }

    sendOscMessage();
    sendOscOutputMessage();
}

//==============================================================================
void ControlGrisAudioProcessor::setSourcePositionsFromState()
{
    auto & state{ mAudioProcessorValueTreeState.state };
    for (auto & source : mSources) {
        auto const index{ source.getIndex().toString() };

        juce::Identifier const azimuthPropertyString{ juce::String{ "p_azimuth_" } + index };
        juce::Identifier const elevationPropertyString{ juce::String{ "p_elevation_" } + index };
        juce::Identifier const distancePropertyString{ juce::String{ "p_distance_" } + index };

        jassert(state.hasProperty(azimuthPropertyString));
        jassert(state.hasProperty(elevationPropertyString));
        jassert(state.hasProperty(distancePropertyString));

        auto const & rawAzimuth{ state.getProperty(azimuthPropertyString) };
        auto const & rawElevation{ state.getProperty(elevationPropertyString) };
        auto const & rawDistance{ state.getProperty(distancePropertyString) };

        Normalized const azimuth{ rawAzimuth };
        Normalized const elevation{ rawElevation };
        float const distance{ rawDistance };

        source.setAzimuth(azimuth, Source::OriginOfChange::userAnchorMove);
        source.setElevation(elevation, Source::OriginOfChange::userAnchorMove);
        source.setDistance(distance, Source::OriginOfChange::userAnchorMove);
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
            setSourceParameterValue(sourceIndex, SourceParameter::azimuth, source.getNormalizedAzimuth().get());
            setSourceParameterValue(sourceIndex, SourceParameter::elevation, source.getNormalizedElevation().get());
        } else {
            setSourceParameterValue(sourceIndex, SourceParameter::azimuth, source.getNormalizedAzimuth().get());
            setSourceParameterValue(sourceIndex, SourceParameter::distance, source.getDistance());
        }
        if (source.isPrimarySource()) {
            mPositionTrajectoryManager.setTrajectoryType(mPositionTrajectoryManager.getTrajectoryType(),
                                                         mSources.getPrimarySource().getPos());
        }
    } else {
        setSourceParameterValue(sourceIndex, SourceParameter::elevation, source.getNormalizedElevation().get());
        mElevationTrajectoryManager.setTrajectoryType(mElevationTrajectoryManager.getTrajectoryType());
    }
}

//==============================================================================
void ControlGrisAudioProcessor::setSourceParameterValue(SourceIndex const sourceIndex,
                                                        SourceParameter const sourceParameter,
                                                        float const value)
{
    Normalized const normalized{ static_cast<float>(value) };
    auto const paramId{ sourceIndex.toString() };
    switch (sourceParameter) {
    case SourceParameter::azimuth:
        mAudioProcessorValueTreeState.state.setProperty("p_azimuth_" + paramId, value, nullptr);
        break;
    case SourceParameter::elevation:
        mAudioProcessorValueTreeState.state.setProperty(juce::String("p_elevation_") + paramId, value, nullptr);
        break;
    case SourceParameter::distance:
        mAudioProcessorValueTreeState.state.setProperty(juce::String("p_distance_") + paramId, value, nullptr);
        break;
    case SourceParameter::x:
        jassertfalse;
        break;
    case SourceParameter::y:
        jassertfalse;
        break;
    case SourceParameter::azimuthSpan:
        for (auto & source : mSources) {
            source.setAzimuthSpan(normalized);
        }
        mAudioProcessorValueTreeState.getParameter(Automation::Ids::AZIMUTH_SPAN)->setValueNotifyingHost(value);
        break;
    case SourceParameter::elevationSpan:
        for (auto & source : mSources) {
            source.setElevationSpan(normalized);
        }
        mAudioProcessorValueTreeState.getParameter(Automation::Ids::ELEVATION_SPAN)->setValueNotifyingHost(value);
        break;
    }
}

//==============================================================================
juce::String const ControlGrisAudioProcessor::getName() const
{
    return JucePlugin_Name;
}

//==============================================================================
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
    if (!mOscConnected && mOscActivated) {
        // Connect OSC to default socket
        mOscConnected = createOscConnection(mCurrentOscAddress, mCurrentOscPort);
    }

    mNeedsInitialization = true;
    mLastTime = mLastTimerTime = 10000000.0;
    mCanStopActivate = true;
}

//==============================================================================
void ControlGrisAudioProcessor::prepareToPlay([[maybe_unused]] double const sampleRate,
                                              [[maybe_unused]] int const samplesPerBlock)
{
    mSampleRate = sampleRate;
    mBlockSize = samplesPerBlock;

    mPitch.reset();
    mLoudness.reset();
    mStats.reset();
    mShape.reset();
    mCentroid.reset();
    mSpread.reset();
    mFlatness.reset();
    mOnsetDetectionAzimuth.reset(samplesPerBlock);
    mOnsetDetectionElevation.reset(samplesPerBlock);
    mOnsetDetectionHSpan.reset(samplesPerBlock);
    mOnsetDetectionVSpan.reset(samplesPerBlock);
    mOnsetDetectionX.reset(samplesPerBlock);
    mOnsetDetectionY.reset(samplesPerBlock);
    mOnsetDetectionZ.reset(samplesPerBlock);

    mPitch.init();
    mLoudness.init(mSampleRate);
    mStats.init();
    mCentroid.init();
    mSpread.init();
    mFlatness.init();
    mOnsetDetectionAzimuth.init();
    mOnsetDetectionElevation.init();
    mOnsetDetectionHSpan.init();
    mOnsetDetectionVSpan.init();
    mOnsetDetectionX.init();
    mOnsetDetectionY.init();
    mOnsetDetectionZ.init();

    mInLoudness.resize(mBlockSize);
    mPaddedLoudness = mLoudness.calculatePadded(mInLoudness);
    mNFramesLoudness = mLoudness.calculateFrames(mPaddedLoudness);
    mLoudnessMat = fluid::RealMatrix(mNFramesLoudness, 2);
    mLoudnessDesc.resize(2);

    mInPitch.resize(mBlockSize);
    mPaddedPitch = mPitch.calculatePadded(mInPitch);
    mNFramesPitch = mPitch.calculateFrames(mPaddedPitch);
    mPitchMat = fluid::RealMatrix(mNFramesPitch, 2);
    mCalculatedPitchDesc.resize(2);
    mPitch.setFrame(mFramePitch);
    mPitch.setMagnitude(mMagnitudePitch);

    mInSpectral.resize(mBlockSize);
    mPaddedSpectral = mShape.calculatePadded(mInSpectral);
    mNFramesSpectral = mShape.calculateFrames(mPaddedSpectral);
    mShapeMat = fluid::RealMatrix(mNFramesSpectral, 7);
    mCalculatedShapeDesc.resize(7);
    mDescriptorsBuffer.setSize(1, mBlockSize);
    mShape.setFrame(nFrameSpectral);
    mShape.setMagnitude(mMagnitudeSpectral);

    auto * ed{ dynamic_cast<ControlGrisAudioProcessorEditor *>(getActiveEditor()) };
    if (ed != nullptr) {
        ed->updateAudioAnalysisNumInputChannels();
    }

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
void ControlGrisAudioProcessor::processBlock([[maybe_unused]] juce::AudioBuffer<float> & buffer,
                                             [[maybe_unused]] juce::MidiBuffer & midiMessages) [[clang::nonblocking]]
{
    auto const wasPlaying{ mIsPlaying };
    juce::AudioPlayHead * audioPlayHead = getPlayHead();
    if (audioPlayHead != nullptr) {
        auto currentPositionInfo = audioPlayHead->getPosition();
        mIsPlaying = currentPositionInfo->getIsPlaying();
        mBpm = currentPositionInfo->getBpm().orFallback(120.0);
        if (mNeedsInitialization) {
            auto timeInSeconds = currentPositionInfo->getTimeInSeconds().orFallback(0.0);
            mCurrentTime = (timeInSeconds < 0.0) ? 0.0 : timeInSeconds;
            mInitTimeOnPlay = mCurrentTime;
            mNeedsInitialization = false;
        } else {
            mCurrentTime = currentPositionInfo->getTimeInSeconds().orFallback(0.0);
            if (mInitTimeOnPlay > mCurrentTime) {
                mInitTimeOnPlay = mCurrentTime;
            }
        }
    }

    if (!wasPlaying
        && mIsPlaying) { // Initialization here only for Logic, Reaper, Live, Pro Tools and Digital Performer,
        juce::PluginHostType const hostType; // which are not calling prepareToPlay every time the sequence starts.
        if (hostType.isLogic() || hostType.isReaper() || hostType.isAbletonLive() || hostType.isDigitalPerformer()
            || hostType.isProTools()
            || juce::String(hostType.getHostDescription()).compare(juce::String("Unknown")) == 0) {
            initialize();
        }
    }

    // deal with trajectory recording gestures
    bool const isPositionTrajectoryActive{ mPositionTrajectoryManager.getPositionActivateState() };
    bool const isElevationTrajectoryActive{ mElevationTrajectoryManager.getPositionActivateState() };

    if (isPositionTrajectoryActive && mIsPlaying && !mPositionGestureStarted) {
        mPositionGestureStarted = true;
        mChangeGesturesManager.beginGesture(Automation::Ids::X);
        mChangeGesturesManager.beginGesture(Automation::Ids::Y);
    } else if ((!isPositionTrajectoryActive || !mIsPlaying) && mPositionGestureStarted) {
        mPositionGestureStarted = false;
        mChangeGesturesManager.endGesture(Automation::Ids::X);
        mChangeGesturesManager.endGesture(Automation::Ids::Y);
    }
    if (mSpatMode == SpatMode::cube) {
        if (isElevationTrajectoryActive && mIsPlaying && !mElevationGestureStarted) {
            mElevationGestureStarted = true;
            mChangeGesturesManager.beginGesture(Automation::Ids::Z);
        } else if ((!isElevationTrajectoryActive || !mIsPlaying) && mElevationGestureStarted) {
            mElevationGestureStarted = false;
            mChangeGesturesManager.endGesture(Automation::Ids::Z);
        }
    }

    mLastTime = mCurrentTime;

    // Audio Descriptors section
    if (mSelectedSoundTrajectoriesTabIdx == 0) {
        mAzimuthDomeValue = 0.0;
        mElevationDomeValue = 0.0;
        mHspanDomeValue = 0.0;
        mVspanDomeValue = 0.0;

        mXCubeValue = 0.0;
        mYCubeValue = 0.0;
        mZCubeValue = 0.0;
        mHspanCubeValue = 0.0;
        mVspanCubeValue = 0.0;

        juce::ScopedNoDenormals noDenormals;
        auto totalNumInputChannels = getTotalNumInputChannels();
        auto totalNumOutputChannels = getTotalNumOutputChannels();

        for (auto i = totalNumInputChannels; i < totalNumOutputChannels; ++i)
            buffer.clear(i, 0, buffer.getNumSamples());

        mDescriptorsBuffer.clear();
        for (int i{}; i < mNumChannelsToAnalyse; ++i) {
            mDescriptorsBuffer.addFrom(0, 0, buffer, i, 0, buffer.getNumSamples());
        }
        mDescriptorsBuffer.applyGain((1.0f / mNumChannelsToAnalyse)
                                     * static_cast<float>(mAudioAnalysisInputGainMultiplier));

        auto bufferMagnitude = mDescriptorsBuffer.getMagnitude(0, mDescriptorsBuffer.getNumSamples());
        auto * channelData = mDescriptorsBuffer.getReadPointer(0);

        // FLUCOMA
        if (shouldProcessDomeLoudnessAnalysis() || shouldProcessCubeLoudnessAnalysis()) {
            for (int i{}; i < mDescriptorsBuffer.getNumSamples(); ++i) {
                mInLoudness[i] = channelData[i];
            }

            mLoudnessMat.fill(0.0);
            std::fill(mPaddedLoudness.begin(), mPaddedLoudness.end(), 0);
            // Note: this and the other intsances of padding do not pad with a symmetric amount of zeroes. Maybe this is
            // fine ?
            std::copy(mInLoudness.begin(), mInLoudness.end(), mPaddedLoudness.begin() + mLoudness.HALF_WINDOW);
            std::fill(mLoudnessDesc.begin(), mLoudnessDesc.end(), 0); // necessary?
            for (int i{}; i < mNFramesLoudness; i++) {
                fluid::RealVectorView windowLoudness = mLoudness.calculateWindow(mPaddedLoudness, i);
                mLoudness.loudnessProcess(windowLoudness, mLoudnessDesc);
                mLoudnessMat.row(i) <<= mLoudnessDesc;
            }

            mLoudness.process(mLoudnessMat, *mStats.getStats());
            double loudnessValue = mLoudness.getValue();
            loudnessValue = juce::Decibels::decibelsToGain(loudnessValue);

            if (mSpatMode == SpatMode::dome) {
                for (int i{}; i < mSpatParametersDomeRefs.size(); ++i) {
                    if (mSpatParametersDomeRefs[i]->shouldProcessLoudnessAnalysis()) {
                        mSpatParametersDomeRefs[i]->process(mLoudness.getID(), loudnessValue);
                        *mSpatParametersDomeValueRefs[i] = mSpatParametersDomeRefs[i]->getDiffValue();
                    }
                }
            } else {
                for (int i{}; i < mSpatParametersCubeRefs.size(); ++i) {
                    if (mSpatParametersCubeRefs[i]->shouldProcessLoudnessAnalysis()) {
                        mSpatParametersCubeRefs[i]->process(mLoudness.getID(), loudnessValue);
                        *mSpatParametersCubeValueRefs[i] = mSpatParametersCubeRefs[i]->getDiffValue();
                    }
                }
            }
        }

        if (shouldProcessDomePitchAnalysis() || shouldProcessCubePitchAnalysis()) {
            for (int i{}; i < mDescriptorsBuffer.getNumSamples(); ++i) {
                mInPitch[i] = channelData[i];
            }

            mPitchMat.fill(0.0);
            std::fill(mPaddedPitch.begin(), mPaddedPitch.end(), 0);
            std::copy(mInPitch.begin(), mInPitch.end(), mPaddedPitch.begin() + mPitch.HALF_WINDOW);

            for (int i{}; i < mNFramesPitch; i++) {
                std::fill(mCalculatedPitchDesc.begin(), mCalculatedPitchDesc.end(), 0);
                fluid::RealVectorView windowPitch = mPitch.calculateWindow(mPaddedPitch, i);
                windowPitch = mPitch.calculateWindow(mPaddedPitch, i);

                mPitch.stftProcess(windowPitch, mFramePitch);
                mPitch.stftMagnitude(mFramePitch, mMagnitudePitch);
                mPitch.yinProcess(mMagnitudePitch, mCalculatedPitchDesc, mSampleRate);
                mPitchMat.row(i) <<= mCalculatedPitchDesc;
            }

            mPitch.process(mPitchMat, *mStats.getStats());
            double pitchValue = mPitch.getValue();
            pitchValue = mParamFunctions.frequencyToMidiNoteNumber(pitchValue);

            if (mSpatMode == SpatMode::dome) {
                for (int i{}; i < mSpatParametersDomeRefs.size(); ++i) {
                    if (mSpatParametersDomeRefs[i]->shouldProcessPitchAnalysis()) {
                        mSpatParametersDomeRefs[i]->process(mPitch.getID(), pitchValue);
                        *mSpatParametersDomeValueRefs[i] = mSpatParametersDomeRefs[i]->getDiffValue();
                    }
                }
            } else {
                for (int i{}; i < mSpatParametersCubeRefs.size(); ++i) {
                    if (mSpatParametersCubeRefs[i]->shouldProcessPitchAnalysis()) {
                        mSpatParametersCubeRefs[i]->process(mPitch.getID(), pitchValue);
                        *mSpatParametersCubeValueRefs[i] = mSpatParametersCubeRefs[i]->getDiffValue();
                    }
                }
            }
        }

        if (shouldProcessDomeSpectralAnalysis() || shouldProcessCubeSpectralAnalysis()) {
            for (int i{}; i < mDescriptorsBuffer.getNumSamples(); ++i) {
                mInSpectral[i] = channelData[i];
            }

            mShapeMat.fill(0.0);
            std::fill(mPaddedSpectral.begin(), mPaddedSpectral.end(), 0);
            std::copy(mInSpectral.begin(), mInSpectral.end(), mPaddedSpectral.begin() + mShape.HALF_WINDOW);

            for (int i{}; i < mNFramesSpectral; i++) {
                std::fill(mCalculatedShapeDesc.begin(), mCalculatedShapeDesc.end(), 0);
                fluid::RealVectorView windowSpectral = mShape.calculateWindow(mPaddedSpectral, i);
                mShape.stftProcess(windowSpectral, nFrameSpectral);
                mShape.stftMagnitude(nFrameSpectral, mMagnitudeSpectral);
                mShape.shapeProcess(mMagnitudeSpectral, mCalculatedShapeDesc, mSampleRate);
                mShapeMat.row(i) <<= mCalculatedShapeDesc;
            }

            mShapeStats = mShape.process(mShapeMat, *mStats.getStats());

            if (shouldProcessDomeCentroidAnalysis() || shouldProcessCubeCentroidAnalysis()) {
                mCentroid.process(mShapeStats);
                double centroidValue = mCentroid.getValue(); // centroidValue when silence = 118.02870609942256
                if (bufferMagnitude == 0.0f) {
                    centroidValue = 0.0;
                }
                if (mSpatMode == SpatMode::dome) {
                    for (int i{}; i < mSpatParametersDomeRefs.size(); ++i) {
                        if (mSpatParametersDomeRefs[i]->shouldProcessCentroidAnalysis()) {
                            mSpatParametersDomeRefs[i]->process(mCentroid.getID(), centroidValue);
                            *mSpatParametersDomeValueRefs[i] = mSpatParametersDomeRefs[i]->getDiffValue();
                        }
                    }
                } else {
                    for (int i{}; i < mSpatParametersCubeRefs.size(); ++i) {
                        if (mSpatParametersCubeRefs[i]->shouldProcessCentroidAnalysis()) {
                            mSpatParametersCubeRefs[i]->process(mCentroid.getID(), centroidValue);
                            *mSpatParametersCubeValueRefs[i] = mSpatParametersCubeRefs[i]->getDiffValue();
                        }
                    }
                }
            }

            if (shouldProcessDomeSpreadAnalysis() || shouldProcessCubeSpreadAnalysis()) {
                mSpread.process(mShapeStats);
                double spreadValue = mSpread.getValue(); // spreadValue when silence  = 16.520351353896057
                if (bufferMagnitude == 0.0f) {
                    spreadValue = 0.0;
                }
                spreadValue = mParamFunctions.zmap(spreadValue, 0.0, 16.0);

                if (mSpatMode == SpatMode::dome) {
                    for (int i{}; i < mSpatParametersDomeRefs.size(); ++i) {
                        if (mSpatParametersDomeRefs[i]->shouldProcessSpreadAnalysis()) {
                            mSpatParametersDomeRefs[i]->process(mSpread.getID(), spreadValue);
                            *mSpatParametersDomeValueRefs[i] = mSpatParametersDomeRefs[i]->getDiffValue();
                        }
                    }
                } else {
                    for (int i{}; i < mSpatParametersCubeRefs.size(); ++i) {
                        if (mSpatParametersCubeRefs[i]->shouldProcessSpreadAnalysis()) {
                            mSpatParametersCubeRefs[i]->process(mSpread.getID(), spreadValue);
                            *mSpatParametersCubeValueRefs[i] = mSpatParametersCubeRefs[i]->getDiffValue();
                        }
                    }
                }
            }

            if (shouldProcessDomeNoiseAnalysis() || shouldProcessCubeNoiseAnalysis()) {
                mFlatness.process(mShapeStats);
                double flatnessValue = mFlatness.getValue(); // flatnessValue when silence = -6.9624443085150120e-13
                if (bufferMagnitude == 0.0f) {
                    flatnessValue = -160.0;
                }
                flatnessValue = juce::Decibels::decibelsToGain(flatnessValue);
                flatnessValue = mParamFunctions.zmap(flatnessValue, 0.0, 0.5);
                flatnessValue = mParamFunctions.power(flatnessValue);

                if (mSpatMode == SpatMode::dome) {
                    for (int i{}; i < mSpatParametersDomeRefs.size(); ++i) {
                        if (mSpatParametersDomeRefs[i]->shouldProcessNoiseAnalysis()) {
                            mSpatParametersDomeRefs[i]->process(mFlatness.getID(), flatnessValue);
                            *mSpatParametersDomeValueRefs[i] = mSpatParametersDomeRefs[i]->getDiffValue();
                        }
                    }
                } else {
                    for (int i{}; i < mSpatParametersCubeRefs.size(); ++i) {
                        if (mSpatParametersCubeRefs[i]->shouldProcessNoiseAnalysis()) {
                            mSpatParametersCubeRefs[i]->process(mFlatness.getID(), flatnessValue);
                            *mSpatParametersCubeValueRefs[i] = mSpatParametersCubeRefs[i]->getDiffValue();
                        }
                    }
                }
            }
        }

        if (shouldProcessDomeOnsetDetectionAnalysis() || shouldProcessCubeOnsetDetectionAnalysis()) {
            if (mSpatMode == SpatMode::dome) {
                for (int i{}; i < mSpatParametersDomeRefs.size(); ++i) {
                    if (mSpatParametersDomeRefs[i]->shouldProcessOnsetDetectionAnalysis()) {
                        mDomeOnsetDetectionRefs[i]->process(mDescriptorsBuffer, mSampleRate, mBlockSize);
                        mSpatParametersDomeRefs[i]->process(mDomeOnsetDetectionRefs[i]->getID(),
                                                            mDomeOnsetDetectionRefs[i]->getValue());
                        *mSpatParametersDomeValueRefs[i] = mSpatParametersDomeRefs[i]->getDiffValue();
                    }
                }
            } else {
                for (int i{}; i < mSpatParametersCubeRefs.size(); ++i) {
                    if (mSpatParametersCubeRefs[i]->shouldProcessOnsetDetectionAnalysis()) {
                        mCubeOnsetDetectionRefs[i]->process(mDescriptorsBuffer, mSampleRate, mBlockSize);
                        mSpatParametersCubeRefs[i]->process(mCubeOnsetDetectionRefs[i]->getID(),
                                                            mCubeOnsetDetectionRefs[i]->getValue());
                        *mSpatParametersCubeValueRefs[i] = mSpatParametersCubeRefs[i]->getDiffValue();
                    }
                }
            }
        }
        processParameterValues();
    }
}

//==============================================================================
juce::AudioProcessorEditor * ControlGrisAudioProcessor::createEditor()
{
    return new ControlGrisAudioProcessorEditor(*this,
                                               mAudioProcessorValueTreeState,
                                               mPositionTrajectoryManager,
                                               mElevationTrajectoryManager);
}

//==============================================================================
void ControlGrisAudioProcessor::getStateInformation(juce::MemoryBlock & destData)
{
    for (int sourceIndex{}; sourceIndex < mSources.MAX_NUMBER_OF_SOURCES; ++sourceIndex) {
        juce::String const id{ sourceIndex };
        juce::Identifier const azimuthId{ juce::String{ "p_azimuth_" } + id };
        juce::Identifier const elevationId{ juce::String{ "p_elevation_" } + id };
        juce::Identifier const distanceId{ juce::String{ "p_distance_" } + id };
        auto const & source{ mSources[sourceIndex] };
        auto const normalizedAzimuth{ source.getNormalizedAzimuth().get() };
        auto const normalizedElevation{ source.getNormalizedElevation().get() };
        auto const distance{ source.getDistance() };

        mAudioProcessorValueTreeState.state.setProperty(azimuthId, normalizedAzimuth, nullptr);
        mAudioProcessorValueTreeState.state.setProperty(elevationId, normalizedElevation, nullptr);
        mAudioProcessorValueTreeState.state.setProperty(distanceId, distance, nullptr);
    }

    mAudioProcessorValueTreeState.state.setProperty("soundTrajSelTab", mSelectedSoundTrajectoriesTabIdx, nullptr);

    auto const state{ mAudioProcessorValueTreeState.copyState() };

    auto xmlState{ state.createXml() };

    if (xmlState != nullptr) {
        auto * childExist = xmlState->getChildByName(FIXED_POSITION_DATA_TAG);
        if (childExist) {
            xmlState->removeChildElement(childExist, true);
        }
        if (mPresetData.getNumChildElements() > 0) {
            auto * positionData{ new juce::XmlElement{ mPresetData } };
            xmlState->addChildElement(positionData);
        }
        copyXmlToBinary(*xmlState, destData);
    }
}

//==============================================================================
void ControlGrisAudioProcessor::setStateInformation(void const * data, int const sizeInBytes)
{
    // TODO : There used to be a MessageManager lock here, but it is causing a deadlock on ProTools. The
    // JUCE_ASSERT_MESSAGE_MANAGER_IS_LOCKED fails. Removing the lock seems ok on ProTools, but it should be tested on
    // other hosts.

    auto const xmlState{ getXmlFromBinary(data, sizeInBytes) };
    if (xmlState != nullptr) {
        // Set global settings values.
        //----------------------------
        auto const valueTree{ juce::ValueTree::fromXml(*xmlState) };
        auto const spatMode{ static_cast<SpatMode>(static_cast<int>(valueTree.getProperty("oscFormat", 0))) };
        setSpatMode(spatMode);
        setOscPortNumber(valueTree.getProperty("oscPortNumber", 18032));
        setOscAddress(valueTree.getProperty("oscAddress", "127.0.0.1"));
        setOscActive(valueTree.getProperty("oscConnected", true));
        setNumberOfSources(valueTree.getProperty("numberOfSources", 1), false);
        setFirstSourceId(SourceId{ valueTree.getProperty("firstSourceId", 1) });
        setOscOutputPluginId(valueTree.getProperty("oscOutputPluginId", 1));

        if (valueTree.getProperty("oscInputConnected", false)) {
            [[maybe_unused]] auto const success{ createOscInputConnection(
                valueTree.getProperty("oscInputPortNumber", 9000)) };
        }

        if (valueTree.getProperty("oscOutputConnected", false)) {
            [[maybe_unused]] auto const success{ createOscOutputConnection(
                valueTree.getProperty("oscOutputAddress", "192.168.1.100"),
                valueTree.getProperty("oscOutputPortNumber", 8000)) };
        }

        // Load last used Sound Trajectories tab
        auto tabIdx{ valueTree.getProperty("soundTrajSelTab", 0) };
        setSelectedSoundTrajectoriesTab(tabIdx);

        // Load stored sources positions
        for (int sourceIndex{}; sourceIndex < mSources.MAX_NUMBER_OF_SOURCES; ++sourceIndex) {
            juce::String const id{ sourceIndex };
            juce::Identifier const azimuthId{ juce::String{ "p_azimuth_" } + id };
            juce::Identifier const elevationId{ juce::String{ "p_elevation_" } + id };
            juce::Identifier const distanceId{ juce::String{ "p_distance_" } + id };
            auto & source{ mSources[sourceIndex] };

            const Radians azimuth{ valueTree.getProperty(azimuthId) };
            const Radians elevation{ valueTree.getProperty(elevationId) };
            const float distance{ valueTree.getProperty(distanceId) };

            source.setAzimuth(azimuth, Source::OriginOfChange::userAnchorMove);
            source.setElevation(elevation, Source::OriginOfChange::userAnchorMove);
            source.setDistance(distance, Source::OriginOfChange::userAnchorMove);
        }

        // Load saved fixed positions.
        //----------------------------
        auto * presetData{ xmlState->getChildByName(FIXED_POSITION_DATA_TAG) };
        if (presetData) {
            mPresetData.deleteAllChildElements();
            mPresetData = *presetData;
        }

        // Replace the state and call automated parameter current values.
        //---------------------------------------------------------------
        mAudioProcessorValueTreeState.replaceState(juce::ValueTree::fromXml(*xmlState));
        // Load/refresh stored spatial parameters values
        //---------------------------------------------------------------
        for (const auto & spatParam : mSpatParametersDomeRefs) {
            spatParam->updateParameterState();
            setOnsetDetectionMinTime(spatParam->getParameterID(), spatParam->getParamMinTime());
            setOnsetDetectionMaxTime(spatParam->getParameterID(), spatParam->getParamMaxTime());
        }
        for (const auto & spatParam : mSpatParametersCubeRefs) {
            spatParam->updateParameterState();
            setOnsetDetectionMinTime(spatParam->getParameterID(), spatParam->getParamMinTime());
            setOnsetDetectionMaxTime(spatParam->getParameterID(), spatParam->getParamMaxTime());
        }
        setXYParamLink(mAudioProcessorValueTreeState.state.getProperty("XYParamLinked"));
    }

    setSourcePositionsFromState();
}

//==============================================================================
PersistentStorage & ControlGrisAudioProcessor::getPersistentStorage()
{
    return mPersistentStorage;
}

//==============================================================================
void ControlGrisAudioProcessor::sourceChanged(Source & source,
                                              Source::ChangeType changeType,
                                              Source::OriginOfChange origin)
{
    /**
     * There is a try-lock here because when the source position is changed by the user (using the mouse),
     * the host is notified and sometimes calls parameterChanged() in the audio thread to move the source,
     * causing a race condition.
     */
    jassert(changeType == Source::ChangeType::position || changeType == Source::ChangeType::elevation);

    const juce::ScopedTryLock tryLock(mLock);
    if (tryLock.isLocked()) {
        auto & trajectoryManager{ changeType == Source::ChangeType::position
                                      ? static_cast<TrajectoryManager &>(mPositionTrajectoryManager)
                                      : static_cast<TrajectoryManager &>(mElevationTrajectoryManager) };
        auto & sourceLinkEnforcer{ changeType == Source::ChangeType::position ? mPositionSourceLinkEnforcer
                                                                              : mElevationSourceLinkEnforcer };
        // auto const isTrajectoryActive{ mPositionTrajectoryManager.getPositionActivateState()
        //                               || mElevationTrajectoryManager.getPositionActivateState() };
        auto const isTrajectoryActive{ trajectoryManager.getPositionActivateState() };
        auto const isPrimarySource{ source.isPrimarySource() };

        switch (origin) {
        case Source::OriginOfChange::none:
            return;
        case Source::OriginOfChange::userMove:
            sourceLinkEnforcer.sourceMoved(source);
            setSelectedSource(source);
            if (isPrimarySource) {
                trajectoryManager.sourceMoved(source);
                updatePrimarySourceParameters(changeType);
            } else {
                getPresetsManager().loadIfPresetChanged(0);
            }
            return;
        case Source::OriginOfChange::userAnchorMove:
            sourceLinkEnforcer.anchorMoved(source);
            setSelectedSource(source);
            if (isPrimarySource) {
                trajectoryManager.sourceMoved(source);
                updatePrimarySourceParameters(changeType);
            }
            mPresetManager.loadIfPresetChanged(0);
            return;
        case Source::OriginOfChange::presetRecall:
            jassert(isPrimarySource);
            sourceLinkEnforcer.sourceMoved(source);
            trajectoryManager.sourceMoved(source);
            return;
        case Source::OriginOfChange::link:
            if (isPrimarySource) {
                sourceLinkEnforcer.sourceMoved(source);
                trajectoryManager.sourceMoved(source);
                updatePrimarySourceParameters(changeType);
            }
            return;
        case Source::OriginOfChange::trajectory:
            jassert(isPrimarySource);
            sourceLinkEnforcer.sourceMoved(source);
            updatePrimarySourceParameters(changeType);
            return;
        case Source::OriginOfChange::osc:
            jassert(isPrimarySource);
            sourceLinkEnforcer.sourceMoved(source);
            trajectoryManager.sourceMoved(source);
            updatePrimarySourceParameters(changeType);
            return;
        case Source::OriginOfChange::automation:
            sourceLinkEnforcer.sourceMoved(source);
            if (!isTrajectoryActive) {
                trajectoryManager.sourceMoved(source);
            }
            return;
        case Source::OriginOfChange::audioAnalysis:
            jassert(isPrimarySource);
            sourceLinkEnforcer.sourceMoved(source);
            return;
        case Source::OriginOfChange::audioAnalysisRecAutomation:
            jassert(isPrimarySource);
            sourceLinkEnforcer.sourceMoved(source);
            updatePrimarySourceParameters(changeType);
            return;
        }
        jassertfalse;
    }
}

//==============================================================================
void ControlGrisAudioProcessor::setSelectedSource(const Source & source)
{
    auto * editor{ dynamic_cast<ControlGrisAudioProcessorEditor *>(getActiveEditor()) };
    if (editor != nullptr) {
        auto const index{ source.getIndex() };
        juce::MessageManager::callAsync([=] { editor->sourceSelectionChangedCallback(index); });
    }
}

//==============================================================================
void ControlGrisAudioProcessor::updatePrimarySourceParameters(Source::ChangeType const changeType)
{
    auto const & source{ mSources.getPrimarySource() };
    switch (changeType) {
    case Source::ChangeType::position: {
        auto const x_sl{ mChangeGesturesManager.getScopedLock(Automation::Ids::X) };
        auto const y_sl{ mChangeGesturesManager.getScopedLock(Automation::Ids::Y) };
        auto const normalized_x{ (source.getX() + 1.0f) / 2.0f };
        auto const normalized_y{ 1.0f - (source.getY() + 1.0f) / 2.0f };
        auto * x_param{ mAudioProcessorValueTreeState.getParameter(Automation::Ids::X) };
        auto * y_param{ mAudioProcessorValueTreeState.getParameter(Automation::Ids::Y) };
        x_param->setValueNotifyingHost(normalized_x);
        y_param->setValueNotifyingHost(normalized_y);
        break;
    }
    case Source::ChangeType::elevation: {
        jassert(mSpatMode == SpatMode::cube);
        auto const sl{ mChangeGesturesManager.getScopedLock(Automation::Ids::Z) };
        auto const normalized_z{ 1.0f - source.getElevation() / Radians{ MAX_ELEVATION } };
        mAudioProcessorValueTreeState.getParameter(Automation::Ids::Z)->setValueNotifyingHost(normalized_z);
        break;
    }
    default:
        jassertfalse;
    }
}

//==============================================================================
void ControlGrisAudioProcessor::setGainMultiplierForAudioAnalysis(double gainMult)
{
    mAudioAnalysisInputGainMultiplier = gainMult;
}

//==============================================================================
void ControlGrisAudioProcessor::setNumChannelsForAudioAnalysis(int numChannels)
{
    mNumChannelsToAnalyse = numChannels;
}

//==============================================================================
void ControlGrisAudioProcessor::setSelectedSoundTrajectoriesTab(int newCurrentTabIndex)
{
    mSelectedSoundTrajectoriesTabIdx = newCurrentTabIndex;
}

//==============================================================================
void ControlGrisAudioProcessor::processParameterValues()
{
    auto & pSource{ mSources.getPrimarySource() };
    auto originOfChange{ mAudioAnalysisActivateState ? gris::Source::OriginOfChange::audioAnalysisRecAutomation
                                                     : gris::Source::OriginOfChange::audioAnalysis };

    if (mSpatMode == SpatMode::dome) {
        // Azimuth
        auto aziDeg{ pSource.getAzimuth().getAsDegrees() };
        pSource.setAzimuth(Radians{ Degrees{ aziDeg - static_cast<float>(mAzimuthDomeValue) } }, originOfChange);
        // Elevation
        auto eleDeg{ pSource.getElevation().getAsDegrees() };

        auto diffElev = eleDeg + static_cast<float>(mElevationDomeValue);
        if (mSpatParamElevationBuffer + diffElev < 0.0f) {
            pSource.setElevation(Radians{ Degrees{ 0.0f } }, originOfChange);
            mSpatParamElevationBuffer += diffElev;
        } else if (mSpatParamElevationBuffer + diffElev > 90.0f) {
            pSource.setElevation(Radians{ Degrees{ 90.0f } }, originOfChange);
            mSpatParamElevationBuffer += diffElev - 90.0f;
        } else if (mSpatParamElevationBuffer + diffElev >= 0.0f && mSpatParamElevationBuffer + diffElev <= 90.0f) {
            pSource.setElevation(Radians{ Degrees{ mSpatParamElevationBuffer + diffElev } }, originOfChange);
            mSpatParamElevationBuffer = 0.0f;
        }
    } else {
        if (mXYParamLinked) {
            // X param behaves like Azimuth
            auto aziDeg{ pSource.getAzimuth().getAsDegrees() };
            pSource.setDistance(juce::jmin(1.0f, pSource.getDistance()), originOfChange);
            pSource.setAzimuth(Radians{ Degrees{ aziDeg - static_cast<float>(mXCubeValue) } }, originOfChange);
        } else {
            // X, Y
            auto descX = static_cast<float>(mXCubeValue);
            auto descY = static_cast<float>(-1.0 * mYCubeValue); // Y is inverted in GUI
            auto sourceXYPosition{ pSource.getPositionFromAngle(pSource.getAzimuth(), pSource.getDistance()) };
            auto diffX = sourceXYPosition.x - descX;
            auto diffY = sourceXYPosition.y - descY;
            float newX{};
            float newY{};

            if (mSpatParamXBuffer + diffX < -1.0f) {
                newX = -1.0f;
                mSpatParamXBuffer += diffX + 1.0f;
            } else if (mSpatParamXBuffer + diffX > 1.0f) {
                newX = 1.0f;
                mSpatParamXBuffer += diffX - 1.0f;
            } else if (mSpatParamXBuffer + diffX >= -1.0f && mSpatParamXBuffer + diffX <= 1.0f) {
                newX = mSpatParamXBuffer + diffX;
                mSpatParamXBuffer = 0.0f;
            }

            if (mSpatParamYBuffer + diffY < -1.0f) {
                newY = -1.0f;
                mSpatParamYBuffer += diffY + 1.0f;
            } else if (mSpatParamYBuffer + diffY > 1.0f) {
                newY = 1.0f;
                mSpatParamYBuffer += diffY - 1.0f;
            } else if (mSpatParamYBuffer + diffY >= -1.0f && mSpatParamYBuffer + diffY <= 1.0f) {
                newY = mSpatParamYBuffer + diffY;
                mSpatParamYBuffer = 0.0f;
            }

            pSource.setPosition({ newX, newY }, originOfChange);
        }

        // Z
        auto descZ = static_cast<float>(-1.0 * mZCubeValue); // Z is inverted in GUI
        auto sourceZPosition{ pSource.getNormalizedElevation().get() };
        auto diffZ = sourceZPosition - descZ;
        float newZ{};

        if (mSpatParamZBuffer + diffZ < 0.0f) {
            newZ = 0.0f;
            mSpatParamZBuffer += diffZ;
        } else if (mSpatParamZBuffer + diffZ > 1.0f) {
            newZ = 1.0f;
            mSpatParamZBuffer += diffZ - 1.0f;
        } else if (mSpatParamZBuffer + diffZ >= 0.0f && mSpatParamZBuffer + diffZ <= 1.0f) {
            newZ = mSpatParamZBuffer + diffZ;
            mSpatParamZBuffer = 0.0f;
        }
        pSource.setElevation(Normalized{ newZ }, originOfChange);
    }
    // Spans
    double hSpanVal{ mSpatMode == SpatMode::dome ? mHspanDomeValue : mHspanCubeValue };
    double vSpanVal{ mSpatMode == SpatMode::dome ? mVspanDomeValue : mVspanCubeValue };
    // // HSpan
    auto hSpan = pSource.getAzimuthSpan().get();
    auto diffHSpan = hSpan + static_cast<float>(hSpanVal) * -0.01f;
    float newHSpanVal{};

    if (mSpatParamHSpanBuffer + diffHSpan < 0.0f) {
        newHSpanVal = 0.0f;
        mSpatParamHSpanBuffer += diffHSpan;
    } else if (mSpatParamHSpanBuffer + diffHSpan > 1.0f) {
        newHSpanVal = 1.0f;
        mSpatParamHSpanBuffer += diffHSpan - 1.0f;
    } else if (mSpatParamHSpanBuffer + diffHSpan >= 0.0f && mSpatParamHSpanBuffer + diffHSpan <= 1.0f) {
        newHSpanVal = mSpatParamHSpanBuffer + diffHSpan;
        mSpatParamHSpanBuffer = 0.0f;
    }
    for (auto & source : mSources) {
        source.setAzimuthSpan(Normalized{ newHSpanVal });
    }

    // // VSpan
    auto vSpan = pSource.getElevationSpan().get();
    auto diffVSpan = vSpan + static_cast<float>(vSpanVal) * -0.01f;
    float newVSpanVal{};

    if (mSpatParamVSpanBuffer + diffVSpan < 0.0f) {
        newVSpanVal = 0.0f;
        mSpatParamVSpanBuffer += diffVSpan;
    } else if (mSpatParamVSpanBuffer + diffVSpan > 1.0f) {
        newVSpanVal = 1.0f;
        mSpatParamVSpanBuffer += diffVSpan - 1.0f;
    } else if (mSpatParamVSpanBuffer + diffVSpan >= 0.0f && mSpatParamVSpanBuffer + diffVSpan <= 1.0f) {
        newVSpanVal = mSpatParamVSpanBuffer + diffVSpan;
        mSpatParamVSpanBuffer = 0.0f;
    }
    for (auto & source : mSources) {
        source.setElevationSpan(Normalized{ newVSpanVal });
    }

    if (mAudioAnalysisActivateState) {
        // recording automation for spans
        if (mAudioAnalysisAzimuthSpanFlag) {
            auto const gestureLockAzimuth{ mChangeGesturesManager.getScopedLock(Automation::Ids::AZIMUTH_SPAN) };
            mAudioProcessorValueTreeState.getParameter(Automation::Ids::AZIMUTH_SPAN)
                ->setValueNotifyingHost(newHSpanVal);
        }
        if (mAudioAnalysisElevationSpanFlag) {
            auto const gestureLockElevation{ mChangeGesturesManager.getScopedLock(Automation::Ids::ELEVATION_SPAN) };
            mAudioProcessorValueTreeState.getParameter(Automation::Ids::ELEVATION_SPAN)
                ->setValueNotifyingHost(newVSpanVal);
        }
    }
}

//==============================================================================
bool ControlGrisAudioProcessor::getXYParamLink()
{
    return mXYParamLinked;
}

//==============================================================================
void ControlGrisAudioProcessor::setXYParamLink(bool isXYParamLinked)
{
    mXYParamLinked = isXYParamLinked;
    mXCube.setActingLikeAzimuth(mXYParamLinked);
    mAudioProcessorValueTreeState.state.setProperty("XYParamLinked", mXYParamLinked, nullptr);
}

//==============================================================================
bool ControlGrisAudioProcessor::getAudioAnalysisState()
{
    return mAudioAnalysisActivateState;
}

//==============================================================================
void ControlGrisAudioProcessor::setAudioAnalysisState(bool state)
{
    mAudioAnalysisActivateState = state;
    if (state) {
        mPositionTrajectoryManager.setPositionActivateState(false);
        mElevationTrajectoryManager.setPositionActivateState(false);
    }
}

//==============================================================================
AzimuthDome & ControlGrisAudioProcessor::getAzimuthDome()
{
    return mAzimuthDome;
}

//==============================================================================
ElevationDome & ControlGrisAudioProcessor::getElevationDome()
{
    return mElevationDome;
}

//==============================================================================
HspanDome & ControlGrisAudioProcessor::getHSpanDome()
{
    return mHSpanDome;
}

//==============================================================================
VspanDome & ControlGrisAudioProcessor::getVSpanDome()
{
    return mVSpanDome;
}

//==============================================================================
XCube & ControlGrisAudioProcessor::getXCube()
{
    return mXCube;
}

//==============================================================================
YCube & ControlGrisAudioProcessor::getYCube()
{
    return mYCube;
}

//==============================================================================
ZCube & ControlGrisAudioProcessor::getZCube()
{
    return mZCube;
}

//==============================================================================
HspanCube & ControlGrisAudioProcessor::getHSpanCube()
{
    return mHSpanCube;
}

//==============================================================================
VspanCube & ControlGrisAudioProcessor::getVSpanCube()
{
    return mVSpanCube;
}

//==============================================================================
void ControlGrisAudioProcessor::setOnsetDetectionMetric(ParameterID paramID, const int metric)
{
    int transposedMetric{};
    switch (metric) {
    case 1: // Energy
    case 2: // High Frequency Content
    case 3: // Spectral Flux
        transposedMetric = metric - 1;
        break;
    default: // case 4 : // Rectified Complex Phase Deviation
        transposedMetric = 9;
        break;
    }

    switch (paramID) {
    case ParameterID::azimuth:
        mOnsetDetectionAzimuth.setOnesetDetectionMetric(transposedMetric);
        break;
    case ParameterID::elevation:
        mOnsetDetectionElevation.setOnesetDetectionMetric(transposedMetric);
        break;
    case ParameterID::x:
        mOnsetDetectionX.setOnesetDetectionMetric(transposedMetric);
        break;
    case ParameterID::y:
        mOnsetDetectionY.setOnesetDetectionMetric(transposedMetric);
        break;
    case ParameterID::z:
        mOnsetDetectionZ.setOnesetDetectionMetric(transposedMetric);
        break;
    case ParameterID::azimuthspan:
        mOnsetDetectionHSpan.setOnesetDetectionMetric(transposedMetric);
        break;
    case ParameterID::elevationspan:
        mOnsetDetectionVSpan.setOnesetDetectionMetric(transposedMetric);
        break;
    default:
        break;
    }
}

//==============================================================================
void ControlGrisAudioProcessor::setOnsetDetectionThreshold(ParameterID paramID, const float tresh)
{
    switch (paramID) {
    case ParameterID::azimuth:
        mOnsetDetectionAzimuth.setOnsetDetectionThreshold(tresh);
        break;
    case ParameterID::elevation:
        mOnsetDetectionElevation.setOnsetDetectionThreshold(tresh);
        break;
    case ParameterID::x:
        mOnsetDetectionX.setOnsetDetectionThreshold(tresh);
        break;
    case ParameterID::y:
        mOnsetDetectionY.setOnsetDetectionThreshold(tresh);
        break;
    case ParameterID::z:
        mOnsetDetectionZ.setOnsetDetectionThreshold(tresh);
        break;
    case ParameterID::azimuthspan:
        mOnsetDetectionHSpan.setOnsetDetectionThreshold(tresh);
        break;
    case ParameterID::elevationspan:
        mOnsetDetectionVSpan.setOnsetDetectionThreshold(tresh);
        break;
    default:
        break;
    }
}

//==============================================================================
void ControlGrisAudioProcessor::setOnsetDetectionMinTime(ParameterID paramID, const double minTime)
{
    switch (paramID) {
    case ParameterID::azimuth:
        mOnsetDetectionAzimuth.setOnsetDetectionMinTime(minTime);
        break;
    case ParameterID::elevation:
        mOnsetDetectionElevation.setOnsetDetectionMinTime(minTime);
        break;
    case ParameterID::x:
        mOnsetDetectionX.setOnsetDetectionMinTime(minTime);
        break;
    case ParameterID::y:
        mOnsetDetectionY.setOnsetDetectionMinTime(minTime);
        break;
    case ParameterID::z:
        mOnsetDetectionZ.setOnsetDetectionMinTime(minTime);
        break;
    case ParameterID::azimuthspan:
        mOnsetDetectionHSpan.setOnsetDetectionMinTime(minTime);
        break;
    case ParameterID::elevationspan:
        mOnsetDetectionVSpan.setOnsetDetectionMinTime(minTime);
        break;
    default:
        break;
    }
}

//==============================================================================
void ControlGrisAudioProcessor::setOnsetDetectionMaxTime(ParameterID paramID, const double maxTime)
{
    switch (paramID) {
    case ParameterID::azimuth:
        mOnsetDetectionAzimuth.setOnsetDetectionMaxTime(maxTime);
        break;
    case ParameterID::elevation:
        mOnsetDetectionElevation.setOnsetDetectionMaxTime(maxTime);
        break;
    case ParameterID::x:
        mOnsetDetectionX.setOnsetDetectionMaxTime(maxTime);
        break;
    case ParameterID::y:
        mOnsetDetectionY.setOnsetDetectionMaxTime(maxTime);
        break;
    case ParameterID::z:
        mOnsetDetectionZ.setOnsetDetectionMaxTime(maxTime);
        break;
    case ParameterID::azimuthspan:
        mOnsetDetectionHSpan.setOnsetDetectionMaxTime(maxTime);
        break;
    case ParameterID::elevationspan:
        mOnsetDetectionVSpan.setOnsetDetectionMaxTime(maxTime);
        break;
    default:
        break;
    }
}

//==============================================================================
void ControlGrisAudioProcessor::setOnsetDetectionFromClick(ParameterID paramID, const double timeValue)
{
    switch (paramID) {
    case ParameterID::azimuth:
        mOnsetDetectionAzimuth.setOnsetDetectionFromClick(timeValue);
        break;
    case ParameterID::elevation:
        mOnsetDetectionElevation.setOnsetDetectionFromClick(timeValue);
        break;
    case ParameterID::x:
        mOnsetDetectionX.setOnsetDetectionFromClick(timeValue);
        break;
    case ParameterID::y:
        mOnsetDetectionY.setOnsetDetectionFromClick(timeValue);
        break;
    case ParameterID::z:
        mOnsetDetectionZ.setOnsetDetectionFromClick(timeValue);
        break;
    case ParameterID::azimuthspan:
        mOnsetDetectionHSpan.setOnsetDetectionFromClick(timeValue);
        break;
    case ParameterID::elevationspan:
        mOnsetDetectionVSpan.setOnsetDetectionFromClick(timeValue);
        break;
    default:
        break;
    }
}

//==============================================================================
bool ControlGrisAudioProcessor::shouldProcessDomeSpectralAnalysis()
{
    if (mSpatMode == SpatMode::dome) {
        for (const auto & spatParam : mSpatParametersDomeRefs) {
            if (spatParam->needsSpectralAnalysis()) {
                return true;
            }
        }
    }
    return false;
}

//==============================================================================
bool ControlGrisAudioProcessor::shouldProcessDomeLoudnessAnalysis()
{
    if (mSpatMode == SpatMode::dome) {
        for (const auto & spatParam : mSpatParametersDomeRefs) {
            if (spatParam->shouldProcessLoudnessAnalysis()) {
                return true;
            }
        }
    }
    return false;
}

//==============================================================================
bool ControlGrisAudioProcessor::shouldProcessDomePitchAnalysis()
{
    if (mSpatMode == SpatMode::dome) {
        for (const auto & spatParam : mSpatParametersDomeRefs) {
            if (spatParam->shouldProcessPitchAnalysis()) {
                return true;
            }
        }
    }
    return false;
}

//==============================================================================
bool ControlGrisAudioProcessor::shouldProcessDomeCentroidAnalysis()
{
    if (mSpatMode == SpatMode::dome) {
        for (const auto & spatParam : mSpatParametersDomeRefs) {
            if (spatParam->shouldProcessCentroidAnalysis()) {
                return true;
            }
        }
    }
    return false;
}

//==============================================================================
bool ControlGrisAudioProcessor::shouldProcessDomeSpreadAnalysis()
{
    if (mSpatMode == SpatMode::dome) {
        for (const auto & spatParam : mSpatParametersDomeRefs) {
            if (spatParam->shouldProcessSpreadAnalysis()) {
                return true;
            }
        }
    }
    return false;
}

//==============================================================================
bool ControlGrisAudioProcessor::shouldProcessDomeNoiseAnalysis()
{
    if (mSpatMode == SpatMode::dome) {
        for (const auto & spatParam : mSpatParametersDomeRefs) {
            if (spatParam->shouldProcessNoiseAnalysis()) {
                return true;
            }
        }
    }
    return false;
}

//==============================================================================
bool ControlGrisAudioProcessor::shouldProcessDomeOnsetDetectionAnalysis()
{
    if (mSpatMode == SpatMode::dome) {
        for (const auto & spatParam : mSpatParametersDomeRefs) {
            if (spatParam->shouldProcessOnsetDetectionAnalysis()) {
                return true;
            }
        }
    }
    return false;
}

//==============================================================================
bool ControlGrisAudioProcessor::shouldProcessCubeSpectralAnalysis()
{
    if (mSpatMode == SpatMode::cube) {
        for (const auto & spatParam : mSpatParametersCubeRefs) {
            if (spatParam->needsSpectralAnalysis()) {
                return true;
            }
        }
    }
    return false;
}

//==============================================================================
bool ControlGrisAudioProcessor::shouldProcessCubeLoudnessAnalysis()
{
    if (mSpatMode == SpatMode::cube) {
        for (const auto & spatParam : mSpatParametersCubeRefs) {
            if (spatParam->shouldProcessLoudnessAnalysis()) {
                return true;
            }
        }
    }
    return false;
}

//==============================================================================
bool ControlGrisAudioProcessor::shouldProcessCubePitchAnalysis()
{
    if (mSpatMode == SpatMode::cube) {
        for (const auto & spatParam : mSpatParametersCubeRefs) {
            if (spatParam->shouldProcessPitchAnalysis()) {
                return true;
            }
        }
    }
    return false;
}

//==============================================================================
bool ControlGrisAudioProcessor::shouldProcessCubeCentroidAnalysis()
{
    if (mSpatMode == SpatMode::cube) {
        for (const auto & spatParam : mSpatParametersCubeRefs) {
            if (spatParam->shouldProcessCentroidAnalysis()) {
                return true;
            }
        }
    }
    return false;
}

//==============================================================================
bool ControlGrisAudioProcessor::shouldProcessCubeSpreadAnalysis()
{
    if (mSpatMode == SpatMode::cube) {
        for (const auto & spatParam : mSpatParametersCubeRefs) {
            if (spatParam->shouldProcessSpreadAnalysis()) {
                return true;
            }
        }
    }
    return false;
}

//==============================================================================
bool ControlGrisAudioProcessor::shouldProcessCubeNoiseAnalysis()
{
    if (mSpatMode == SpatMode::cube) {
        for (const auto & spatParam : mSpatParametersCubeRefs) {
            if (spatParam->shouldProcessNoiseAnalysis()) {
                return true;
            }
        }
    }
    return false;
}

//==============================================================================
bool ControlGrisAudioProcessor::shouldProcessCubeOnsetDetectionAnalysis()
{
    if (mSpatMode == SpatMode::cube) {
        for (const auto & spatParam : mSpatParametersCubeRefs) {
            if (spatParam->shouldProcessOnsetDetectionAnalysis()) {
                return true;
            }
        }
    }
    return false;
}

} // namespace gris

//==============================================================================
// This creates new instances of the plugin..
juce::AudioProcessor * JUCE_CALLTYPE createPluginFilter()
{
    return new gris::ControlGrisAudioProcessor{};
}
