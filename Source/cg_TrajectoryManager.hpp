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

#include <optional>

#include <JuceHeader.h>

#include "cg_Source.hpp"
#include "cg_Trajectory.hpp"
#include "cg_constants.hpp"

namespace gris
{
class ControlGrisAudioProcessor;

//==============================================================================
class TrajectoryManager
{
public:
    //==============================================================================
    class Listener
    {
    public:
        Listener() noexcept = default;
        virtual ~Listener() noexcept = default;

        Listener(Listener const &) = delete;
        Listener(Listener &&) = delete;

        Listener & operator=(Listener const &) = delete;
        Listener & operator=(Listener &&) = delete;
        //==============================================================================
        virtual void
            trajectoryPositionChanged(TrajectoryManager * manager, juce::Point<float> position, Radians elevation)
            = 0;

    private:
        //==============================================================================
        JUCE_LEAK_DETECTOR(Listener)

    }; // class AutomationManager::Listener

    //==============================================================================
    enum class Direction { forward, backward };

protected:
    //==============================================================================
    ControlGrisAudioProcessor & mProcessor;

    juce::ListenerList<Listener> mListeners;

    bool mIsBackAndForth{ false };
    Direction mBackAndForthDirection{ Direction::forward };

    int mDampeningCycles{};
    int mDampeningCycleCount{};
    double mDampeningLastDelta{};

    bool mActivateState{ false };
    double mPlaybackDuration{ 5.0 };
    double mCurrentPlaybackDuration{ 5.0 };

    double mTrajectoryDeltaTime{};
    double mLastTrajectoryDeltaTime{};
    std::optional<Trajectory> mTrajectory{};
    juce::Point<float> mCurrentTrajectoryPoint{};
    juce::Point<float> mLastRecordingPoint{};

    std::atomic<double> mTrajectoryCurrentSpeed{ 1.0 };
    std::atomic<double> mTrajectoryLastSpeed{ 1.0 };
    double mNormalizedTimeBufferAdjustment{};
    double mRandomTimeAdjustment{};
    bool mTrajectoryRandomEnabled{};
    double mTrajectoryRandomProximity{};
    double mTrajectoryRandomTimeMin{ 0.03 };
    double mTrajectoryRandomTimeMax{ 5.0 };
    double mCurrentRandomTime{};
    double mTrajectoryRandomTimeFromPlaySinceLastPosChange{};

    juce::Random mRandomTrajectoryDeviation{};

    Degrees mDegreeOfDeviationPerCycle{};
    Degrees mCurrentDegreeOfDeviation{};
    int mDeviationCycleCount{};

    Source & mPrimarySource;

public:
    //==============================================================================
    TrajectoryManager(ControlGrisAudioProcessor & processor, Source & principalSource) noexcept;
    virtual ~TrajectoryManager() = default;

    TrajectoryManager(TrajectoryManager const &) = delete;
    TrajectoryManager(TrajectoryManager &&) = delete;

    TrajectoryManager & operator=(TrajectoryManager const &) = delete;
    TrajectoryManager & operator=(TrajectoryManager &&) = delete;
    //==============================================================================
    [[nodiscard]] ControlGrisAudioProcessor & getProcessor() const { return mProcessor; }

    void setPositionActivateState(bool state);
    [[nodiscard]] bool getPositionActivateState() const { return mActivateState; }

    void setPlaybackDuration(double const value) { mPlaybackDuration = value; }

    void resetRecordingTrajectory(juce::Point<float> currentPosition);
    void addRecordingPoint(juce::Point<float> const & pos);
    [[nodiscard]] juce::Point<float> getCurrentTrajectoryPoint() const;

    void setTrajectoryDeltaTime(double relativeTimeFromPlay);
    void setTrajectoryCurrentSpeed(double speed);
    void setTrajectoryRandomEnabled(bool isEnabled);
    void setTrajectoryRandomProximity(double proximity);
    void setTrajectoryRandomTimeMin(double timeMin);
    void setTrajectoryRandomTimeMax(double timeMax);
    [[nodiscard]] std::optional<Trajectory> const & getTrajectory() const { return mTrajectory; }

    void setPositionBackAndForth(bool const newState) { mIsBackAndForth = newState; }

    void setPositionDampeningCycles(int const value) { this->mDampeningCycles = value; }
    void setDeviationPerCycle(Degrees const value) { this->mDegreeOfDeviationPerCycle = value; }
    void addListener(Listener * l) { mListeners.add(l); }

    void sourceMoved(Source & source);
    //==============================================================================
    virtual void sendTrajectoryPositionChangedEvent() = 0;
    virtual void recomputeTrajectory() = 0;
    virtual void applyCurrentTrajectoryPointToPrimarySource() = 0;

protected:
    //==============================================================================
    [[nodiscard]] virtual juce::Point<float> extractTrajectoryPointFromPrimarySource() const = 0;

private:
    //==============================================================================
    void invertBackAndForthDirection();
    void computeCurrentTrajectoryPoint();
    [[nodiscard]] juce::Point<float> smoothRecordingPosition(juce::Point<float> const & pos);
    void calculateCurrentRandomTime();
    //==============================================================================
    JUCE_LEAK_DETECTOR(TrajectoryManager)

}; // TrajectoryManager

//==============================================================================
class PositionTrajectoryManager final : public TrajectoryManager
{
    PositionTrajectoryType mTrajectoryType{ PositionTrajectoryType::drawing };
    PositionSourceLink mSourceLink{ PositionSourceLink::independent };

public:
    //==============================================================================
    PositionTrajectoryManager() = delete;
    ~PositionTrajectoryManager() noexcept override = default;

    PositionTrajectoryManager(PositionTrajectoryManager const &) = delete;
    PositionTrajectoryManager(PositionTrajectoryManager &&) = delete;

    PositionTrajectoryManager & operator=(PositionTrajectoryManager const &) = delete;
    PositionTrajectoryManager & operator=(PositionTrajectoryManager &&) = delete;
    //==============================================================================
    PositionTrajectoryManager(ControlGrisAudioProcessor & processor, Source & principalSource) noexcept
        : TrajectoryManager(processor, principalSource)
    {
    }
    //==============================================================================
    [[nodiscard]] PositionTrajectoryType getTrajectoryType() const { return mTrajectoryType; }
    void setTrajectoryType(PositionTrajectoryType type, juce::Point<float> const & startPos);

    void setSourceLink(PositionSourceLink const sourceLink) { mSourceLink = sourceLink; }
    [[nodiscard]] PositionSourceLink getSourceLink() const { return mSourceLink; }
    //==============================================================================
    void applyCurrentTrajectoryPointToPrimarySource() override;
    void sendTrajectoryPositionChangedEvent() override;
    void recomputeTrajectory() override;

private:
    //==============================================================================
    [[nodiscard]] juce::Point<float> extractTrajectoryPointFromPrimarySource() const override;
    //==============================================================================
    JUCE_LEAK_DETECTOR(PositionTrajectoryManager)

}; // PositionTrajectoryManager

//==============================================================================
class ElevationTrajectoryManager final : public TrajectoryManager
{
    ElevationTrajectoryType mTrajectoryType{ ElevationTrajectoryType::drawing };
    ElevationSourceLink mSourceLink{ ElevationSourceLink::independent };

public:
    //==============================================================================
    ElevationTrajectoryManager() = delete;
    ~ElevationTrajectoryManager() noexcept override = default;

    ElevationTrajectoryManager(ElevationTrajectoryManager const &) = delete;
    ElevationTrajectoryManager(ElevationTrajectoryManager &&) = delete;

    ElevationTrajectoryManager & operator=(ElevationTrajectoryManager const &) = delete;
    ElevationTrajectoryManager & operator=(ElevationTrajectoryManager &&) = delete;
    //==============================================================================
    ElevationTrajectoryManager(ControlGrisAudioProcessor & processor, Source & principalSource) noexcept
        : TrajectoryManager(processor, principalSource)
    {
    }
    //==============================================================================
    void setTrajectoryType(ElevationTrajectoryType type);
    [[nodiscard]] ElevationTrajectoryType getTrajectoryType() const { return mTrajectoryType; }

    void setSourceLink(ElevationSourceLink const sourceLink) { mSourceLink = sourceLink; }
    [[nodiscard]] ElevationSourceLink getSourceLink() const { return mSourceLink; }

    void sendTrajectoryPositionChangedEvent() override;
    void applyCurrentTrajectoryPointToPrimarySource() override;
    void recomputeTrajectory() override;

private:
    //==============================================================================
    [[nodiscard]] juce::Point<float> extractTrajectoryPointFromPrimarySource() const override;
    //==============================================================================
    JUCE_LEAK_DETECTOR(ElevationTrajectoryManager)

}; // ElevationTrajectoryManager

} // namespace gris
