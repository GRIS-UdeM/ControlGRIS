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

#include "cg_ChangeGesturesManager.hpp"
#include "cg_PersistentStorage.h"
#include "cg_PresetsManager.hpp"
#include "cg_Source.hpp"
#include "cg_SourceLinkEnforcer.hpp"
#include "cg_TrajectoryManager.hpp"
#include "cg_constants.hpp"

#include "FluidVersion.hpp"

#include "Descriptors/cg_Centroid.hpp"
#include "Descriptors/cg_Flatness.hpp"
#include "Descriptors/cg_Loudness.hpp"
#include "Descriptors/cg_OnsetDetection.hpp"
#include "Descriptors/cg_Pitch.hpp"
#include "Descriptors/cg_Shape.hpp"
#include "Descriptors/cg_Spread.hpp"
#include "Descriptors/cg_Stats.hpp"

#include "SpatialParameters/cg_SpatParamHelperFunctions.h"

#include "SpatialParameters/cg_AzimuthDome.hpp"
#include "SpatialParameters/cg_ElevationDome.hpp"
#include "SpatialParameters/cg_HspanCube.hpp"
#include "SpatialParameters/cg_HspanDome.hpp"
#include "SpatialParameters/cg_VspanCube.hpp"
#include "SpatialParameters/cg_VspanDome.hpp"
#include "SpatialParameters/cg_XCube.hpp"
#include "SpatialParameters/cg_YCube.hpp"
#include "SpatialParameters/cg_ZCube.hpp"

namespace gris
{
juce::AudioProcessorValueTreeState::ParameterLayout createParameterLayout();
//==============================================================================
class ControlGrisAudioProcessor final
    : public juce::AudioProcessor
    , public juce::AudioProcessorValueTreeState::Listener
    , public juce::Timer
    , private juce::OSCReceiver::Listener<juce::OSCReceiver::RealtimeCallback>
{
    //==============================================================================
    SpatMode mSpatMode{ SpatMode::dome };
    bool mOscActivated{ true };
    bool mOscConnected{ false };
    bool mOscInputConnected{ false };
    bool mOscOutputConnected{ false };
    SourceId mFirstSourceId{ 1 };
    int mCurrentOscPort{ 18032 };
    juce::String mCurrentOscAddress{ "127.0.0.1" };
    int mLastConnectedOscPort{ -1 };
    int mCurrentOscInputPort{ 8000 };
    int mCurrentOscOutputPort{ 9000 };
    juce::String mCurrentOscOutputAddress{ "192.168.1.100" };
    bool mNeedsInitialization{ true };
    PersistentStorage mPersistentStorage;

    double mInitTimeOnPlay{ 0.0 };
    double mCurrentTime{ 0.0 };
    double mLastTime{ 10000000.0 };
    double mLastTimerTime{ 10000000.0 };

    bool mIsPlaying{ false };
    bool mCanStopActivate{ false };
    double mBpm{ 120 };

    bool mPositionGestureStarted{};
    bool mElevationGestureStarted{};

    // juce::Uuid uniqueID{}; // for debugging purposes

    // OSC stuff
    const float IMPOSSIBLE_NUMBER{ std::numeric_limits<float>::min() };
    float mLastTrajectoryX{ IMPOSSIBLE_NUMBER };
    float mLastTrajectoryY{ IMPOSSIBLE_NUMBER };
    float mLastTrajectoryZ{ IMPOSSIBLE_NUMBER };
    Normalized mLastAzimuthSpan{ IMPOSSIBLE_NUMBER };
    Normalized mLastElevationSpan{ IMPOSSIBLE_NUMBER };
    PositionSourceLink mLastPositionLink{ PositionSourceLink::undefined };
    ElevationSourceLink mLastElevationLink{ ElevationSourceLink::undefined };
    int mLastPresetNumber{ std::numeric_limits<int>::min() };
    ElevationMode mLastElevationMode{};

    juce::OSCSender mOscSender;
    juce::OSCSender mOscOutputSender;
    juce::OSCReceiver mOscInputReceiver;

    juce::XmlElement mFixPositionData{ FIXED_POSITION_DATA_TAG };

    Sources mSources{};
    SourceLinkEnforcer mPositionSourceLinkEnforcer{ mSources, PositionSourceLink::independent };
    SourceLinkEnforcer mElevationSourceLinkEnforcer{ mSources, ElevationSourceLink::independent };

    juce::AudioProcessorValueTreeState mAudioProcessorValueTreeState;

    ChangeGesturesManager mChangeGesturesManager{ mAudioProcessorValueTreeState };
    PresetsManager mPresetManager{ mFixPositionData,
                                   mSources,
                                   mPositionSourceLinkEnforcer,
                                   mElevationSourceLinkEnforcer };

    PositionTrajectoryManager mPositionTrajectoryManager{ *this, mSources.getPrimarySource() };
    ElevationTrajectoryManager mElevationTrajectoryManager{ *this, mSources.getPrimarySource() };

    ElevationMode mElevationMode{};

    int mSelectedSoundSpatializationTabIdx{};

    //==============================================================================
    // Audio Descriptors
    double mSampleRate{};
    int mBlockSize{};
    juce::AudioBuffer<float> mDescriptorsBuffer;
    double mAudioAnalysisInputGain{};
    int mNumChannelsToAnalyse{};
    bool mXYParamLinked{};
    bool mAudioAnalysisActivateState{};
    bool mAudioAnalysisAzimuthSpanFlag{};
    bool mAudioAnalysisElevationSpanFlag{};

    float mSpatParamElevationBuffer{};
    float mSpatParamHSpanBuffer{};
    float mSpatParamVSpanBuffer{};
    float mSpatParamXBuffer{};
    float mSpatParamYBuffer{};
    float mSpatParamZBuffer{};

    double mAzimuthDomeValue{};
    double mElevationDomeValue{};
    double mHspanDomeValue{};
    double mVspanDomeValue{};

    double mXCubeValue{};
    double mYCubeValue{};
    double mZCubeValue{};
    double mHspanCubeValue{};
    double mVspanCubeValue{};

    StatsD mStats;
    ShapeD mShape;
    PitchD mPitch;
    LoudnessD mLoudness;
    CentroidD mCentroid;
    SpreadD mSpread;
    FlatnessD mFlatness;
    OnsetDetectionD mOnsetDetectionAzimuth;
    OnsetDetectionD mOnsetDetectionElevation;
    OnsetDetectionD mOnsetDetectionHSpan;
    OnsetDetectionD mOnsetDetectionVSpan;
    OnsetDetectionD mOnsetDetectionX;
    OnsetDetectionD mOnsetDetectionY;
    OnsetDetectionD mOnsetDetectionZ;

    SpatParamHelperFunctions mParamFunctions;

    AzimuthDome mAzimuthDome;
    ElevationDome mElevationDome;
    HspanDome mHSpanDome;
    VspanDome mVSpanDome;
    XCube mXCube;
    YCube mYCube;
    ZCube mZCube;
    HspanCube mHSpanCube;
    VspanCube mVSpanCube;

    std::array<SpatialParameter *, 4> mSpatParametersDomeRefs; // just an array of references to dome spatial parameters
    std::array<SpatialParameter *, 5> mSpatParametersCubeRefs; // just an array of references to cube spatial parameters
    std::array<double *, 4>
        mSpatParametersDomeValueRefs; // just an array of references to dome spatial parameters values
    std::array<double *, 5>
        mSpatParametersCubeValueRefs; // just an array of references to cube spatial parameters values
    // just an arrays of references to OnsetDetection objs. We use the same span OnsetDetection
    // objs in both dome and cube modes
    std::array<OnsetDetectionD *, 4> mDomeOnsetDetectionRefs;
    std::array<OnsetDetectionD *, 5> mCubeOnsetDetectionRefs;

public:
    //==============================================================================
    ControlGrisAudioProcessor();
    ~ControlGrisAudioProcessor() override;

    ControlGrisAudioProcessor(ControlGrisAudioProcessor const &) = delete;
    ControlGrisAudioProcessor(ControlGrisAudioProcessor &&) = delete;

    ControlGrisAudioProcessor & operator=(ControlGrisAudioProcessor const &) = delete;
    ControlGrisAudioProcessor & operator=(ControlGrisAudioProcessor &&) = delete;
    //==============================================================================
    void prepareToPlay(double sampleRate, int samplesPerBlock) override;
    void releaseResources() override {}

#ifndef JucePlugin_PreferredChannelConfigurations
    bool isBusesLayoutSupported(BusesLayout const & layouts) const override;
#endif

    void processBlock(juce::AudioBuffer<float> &, juce::MidiBuffer &) override;

    //==============================================================================
    juce::AudioProcessorEditor * createEditor() override;
    bool hasEditor() const override { return true; } // (change this to false if you choose to not supply an editor)

    //==============================================================================
    juce::String const getName() const override;

    bool acceptsMidi() const override;
    bool producesMidi() const override;
    bool isMidiEffect() const override;
    double getTailLengthSeconds() const override { return 0.0; }

    //==============================================================================
    int getNumPrograms() override
    {
        return 1;
    } // NB: some hosts don't cope very well if you tell them there are 0 programs,
      // so this should be at least 1, even if you're not really implementing programs.
    int getCurrentProgram() override { return 0; }
    void setCurrentProgram([[maybe_unused]] int index) override {}
    juce::String const getProgramName([[maybe_unused]] int index) override { return {}; }
    void changeProgramName([[maybe_unused]] int index, [[maybe_unused]] juce::String const & newName) override {}

    //==============================================================================
    void getStateInformation(juce::MemoryBlock & destData) override;
    void setStateInformation(const void * data, int sizeInBytes) override;

    //==============================================================================
    PersistentStorage & getPersistentStorage();

    //==============================================================================
    void parameterChanged(juce::String const & parameterId, float newValue) override;

    //==============================================================================
    void setSpatMode(SpatMode spatMode);
    SpatMode getSpatMode() const { return mSpatMode; }

    void setOscPortNumber(int oscPortNumber);
    void setOscAddress(juce::String const & address);
    int getOscPortNumber() const { return mCurrentOscPort; }
    juce::String const & getOscAddress() const { return mCurrentOscAddress; }

    void setFirstSourceId(SourceId firstSourceId, bool propagate = true);
    auto getFirstSourceId() const { return mFirstSourceId; }

    void setNumberOfSources(int numOfSources, bool propagate = true);

    auto & getSources() { return mSources; }
    auto const & getSources() const { return mSources; }

    //==============================================================================
    juce::AudioProcessorValueTreeState const & getValueTreeState() const { return mAudioProcessorValueTreeState; }
    juce::AudioProcessorValueTreeState & getValueTreeState() { return mAudioProcessorValueTreeState; }
    //==============================================================================
    [[nodiscard]] bool createOscConnection(juce::String const &, int oscPort);
    [[nodiscard]] bool disconnectOsc();
    [[nodiscard]] bool isOscConnected() const { return mOscConnected; }
    [[nodiscard]] bool isOscActive() const { return mOscActivated; }
    void setOscActive(bool state);
    void sendOscMessage();

    [[nodiscard]] bool createOscInputConnection(int oscPort);
    [[nodiscard]] bool disconnectOscInput(int oscPort);
    [[nodiscard]] bool getOscInputConnected() const { return mOscInputConnected; }
    void oscMessageReceived(const juce::OSCMessage & message) override;
    void oscBundleReceived(const juce::OSCBundle & bundle) override;

    [[nodiscard]] bool createOscOutputConnection(juce::String const & oscAddress, int oscPort);
    [[nodiscard]] bool disconnectOscOutput(juce::String const & oscAddress, int oscPort);
    [[nodiscard]] bool getOscOutputConnected() const { return mOscOutputConnected; }
    void sendOscOutputMessage();
    void setOscOutputPluginId(int pluginId);
    [[nodiscard]] int getOscOutputPluginId() const;

    void timerCallback() override;

    //==============================================================================
    void setPluginState();

    void sourcePositionChanged(SourceIndex sourceIndex, int whichField);
    void setSourceParameterValue(SourceIndex sourceIndex, SourceParameter sourceParameter, float value);

    void initialize();

    double getInitTimeOnPlay() const { return std::max(mInitTimeOnPlay, 0.0); }
    double getCurrentTime() const { return std::max(mCurrentTime, 0.0); }

    bool isPlaying() const { return mIsPlaying; }
    double getBpm() const { return mBpm; }

    ChangeGesturesManager & getChangeGestureManager() { return mChangeGesturesManager; }

    PositionSourceLink getPositionSourceLink() const { return mPositionTrajectoryManager.getSourceLink(); }

    void setPositionSourceLink(PositionSourceLink newSourceLink, SourceLinkEnforcer::OriginOfChange originOfChange);
    void setElevationSourceLink(ElevationSourceLink newSourceLink, SourceLinkEnforcer::OriginOfChange originOfChange);
    void setElevationSourceLinkScale(double scale);

    PresetsManager & getPresetsManager() { return mPresetManager; }
    PresetsManager const & getPresetsManager() const { return mPresetManager; }

    void sourceChanged(Source & source, Source::ChangeType changeType, Source::OriginOfChange origin);
    void setSelectedSource(Source const & source);
    void updatePrimarySourceParameters(Source::ChangeType changeType);
    void setGainForAudioAnalysis(double gain);
    void setNumChannelsForAudioAnalysis(int numChannels);
    void setSelectedSoundSpatializationTab(int newCurrentTabIndex);
    void processParameterValues();
    bool getXYParamLink();
    void setXYParamLink(bool isXYParamLinked);
    bool getAudioAnalysisState();
    void setAudioAnalysisState(bool state);

    //=================================================================================
    // Audio Descriptors

    AzimuthDome & getAzimuthDome();
    ElevationDome & getElevationDome();
    HspanDome & getHSpanDome();
    VspanDome & getVSpanDome();
    XCube & getXCube();
    YCube & getYCube();
    ZCube & getZCube();
    HspanCube & getHSpanCube();
    VspanCube & getVSpanCube();

    void setOnsetDetectionMetric(ParameterID paramID, const int metric);
    void setOnsetDetectionThreshold(ParameterID paramID, const float tresh);
    void setOnsetDetectionMinTime(ParameterID paramID, const double minTime);
    void setOnsetDetectionMaxTime(ParameterID paramID, const double maxTime);
    void setOnsetDetectionFromClick(ParameterID paramID, const double timeValue);

    void setAudioAnalysisAzimuthSpanFlag(bool flag) { mAudioAnalysisAzimuthSpanFlag = flag; }
    void setAudioAnalysisElevationSpanFlag(bool flag) { mAudioAnalysisElevationSpanFlag = flag; }

    bool shouldProcessDomeSpectralAnalysis();
    bool shouldProcessDomeLoudnessAnalysis();
    bool shouldProcessDomePitchAnalysis();
    bool shouldProcessDomeCentroidAnalysis();
    bool shouldProcessDomeSpreadAnalysis();
    bool shouldProcessDomeNoiseAnalysis();
    bool shouldProcessDomeOnsetDetectionAnalysis();
    bool shouldProcessCubeSpectralAnalysis();
    bool shouldProcessCubeLoudnessAnalysis();
    bool shouldProcessCubePitchAnalysis();
    bool shouldProcessCubeCentroidAnalysis();
    bool shouldProcessCubeSpreadAnalysis();
    bool shouldProcessCubeNoiseAnalysis();
    bool shouldProcessCubeOnsetDetectionAnalysis();

private:
    //==============================================================================
    JUCE_LEAK_DETECTOR(ControlGrisAudioProcessor)
};

} // namespace gris
