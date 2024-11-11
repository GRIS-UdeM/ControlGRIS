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

#include "cg_TrajectoryManager.hpp"

#include <algorithm>

namespace gris
{
//==============================================================================
TrajectoryManager::TrajectoryManager(ControlGrisAudioProcessor & processor, Source & principalSource) noexcept
    : mProcessor(processor)
    , mPrimarySource(principalSource)
{
}

//==============================================================================
void TrajectoryManager::setPositionActivateState(bool const state)
{
    mActivateState = state;
    if (state) {
        mTrajectoryDeltaTime = 0.0;
        mLastTrajectoryDeltaTime = 0.0;
        mBackAndForthDirection = Direction::forward;
        mDampeningCycleCount = 0;
        mDampeningLastDelta = 0.0;
        mCurrentPlaybackDuration = mPlaybackDuration;
        mCurrentDegreeOfDeviation = Degrees{ 0.0f };
        mDeviationCycleCount = 0;
        mNormalizedTimeBufferAdjustment = 0.0;
        mTrajectoryLastSpeed.store(mTrajectoryCurrentSpeed.load());
        calculateCurrentRandomTime();
        mRandomTimeAdjustment = 0.0;
        mTrajectoryRandomTimeFromPlaySinceLastPosChange = 0.0;
    }
}

//==============================================================================
void TrajectoryManager::resetRecordingTrajectory(juce::Point<float> const currentPosition)
{
    jassert(currentPosition.getX() >= -1.0f && currentPosition.getX() <= 1.0f && currentPosition.getY() >= -1.0f
            && currentPosition.getY() <= 1.0f);
    jassert(mTrajectory.has_value());

    mTrajectory->clear();
    mTrajectory->addPoint(currentPosition);
    mLastRecordingPoint = currentPosition;
    mBackAndForthDirection = Direction::forward;
}

//==============================================================================
juce::Point<float> TrajectoryManager::smoothRecordingPosition(juce::Point<float> const & pos)
{
    constexpr auto smoothingFactor = 0.8f;

    mLastRecordingPoint = (mLastRecordingPoint - pos) * smoothingFactor + pos;
    return mLastRecordingPoint;
}

//==============================================================================
void TrajectoryManager::calculateCurrentRandomTime()
{
    if (mTrajectoryRandomTimeMin != mTrajectoryRandomTimeMax) {
        mCurrentRandomTime = mRandomTrajectoryDeviation.nextDouble() * (mTrajectoryRandomTimeMax - mTrajectoryRandomTimeMin) + mTrajectoryRandomTimeMin;
    } else {
        mCurrentRandomTime = mTrajectoryRandomTimeMin;
    }
}

//==============================================================================
void TrajectoryManager::setTrajectoryCurrentSpeed(double speed)
{
    mTrajectoryCurrentSpeed.store(speed);
}

//==============================================================================
void TrajectoryManager::setTrajectoryRandomEnabled(bool isEnabled)
{
    mTrajectoryRandomEnabled = isEnabled;

    if (!isEnabled) {
        mRandomTimeAdjustment = 0.0;
    }
}

//==============================================================================
void TrajectoryManager::setTrajectoryRandomType(TrajectoryRandomType type)
{
    mTrajectoryRandomType = type;
}

//==============================================================================
void TrajectoryManager::setTrajectoryRandomProximity(double proximity)
{
    mTrajectoryRandomProximity = proximity;
}

//==============================================================================
void TrajectoryManager::setTrajectoryRandomTimeMin(double timeMin)
{
    mTrajectoryRandomTimeMin = timeMin;
}

//==============================================================================
void TrajectoryManager::setTrajectoryRandomTimeMax(double timeMax)
{
    mTrajectoryRandomTimeMax = timeMax;
}

//==============================================================================
void TrajectoryManager::setTrajectoryDeltaTime(double const relativeTimeFromPlay)
{
    auto trajectoryCurrentSpeed{ mTrajectoryCurrentSpeed.load() };
    auto trajectoryLastSpeed{ mTrajectoryLastSpeed.load() };

    // Random logic
    if (mTrajectoryRandomEnabled) {
        auto timeSinceLastPosChange{ relativeTimeFromPlay - mTrajectoryRandomTimeFromPlaySinceLastPosChange };

        if (timeSinceLastPosChange >= mCurrentRandomTime && relativeTimeFromPlay != 0.0) {
            calculateCurrentRandomTime();
            mTrajectoryRandomTimeFromPlaySinceLastPosChange = relativeTimeFromPlay;

            // find random adjustment
            auto randRange{ juce::Range<double>(0.0, mTrajectoryRandomProximity) };
            auto nextRandomVal{ mRandomTrajectoryDeviation.nextDouble() };
            mRandomTimeAdjustment = nextRandomVal * randRange.getLength() - (randRange.getLength() / 2);
        } else if (relativeTimeFromPlay < mTrajectoryRandomTimeFromPlaySinceLastPosChange) {
            // if the playhead of the DAW is in loop mode
            mTrajectoryRandomTimeFromPlaySinceLastPosChange = relativeTimeFromPlay;
        }
    }

    // Speed logic
    double normalizedTime = relativeTimeFromPlay * trajectoryLastSpeed / mCurrentPlaybackDuration;
    normalizedTime = std::fmod(normalizedTime, 1.0);

    double newNormalizedTime = relativeTimeFromPlay * trajectoryCurrentSpeed / mCurrentPlaybackDuration;
    newNormalizedTime = std::fmod(newNormalizedTime, 1.0);

    if (trajectoryCurrentSpeed != trajectoryLastSpeed) {
        mNormalizedTimeBufferAdjustment += normalizedTime - newNormalizedTime;
        mNormalizedTimeBufferAdjustment = std::fmod(mNormalizedTimeBufferAdjustment, 1.0);
    }
    auto deltaTimeAdjustement{ newNormalizedTime + mNormalizedTimeBufferAdjustment + mRandomTimeAdjustment};
    if (deltaTimeAdjustement < 0) {
        deltaTimeAdjustement = 1.0 + deltaTimeAdjustement;
    }
    mTrajectoryDeltaTime = std::fmod(deltaTimeAdjustement, 1.0);
    mTrajectoryLastSpeed.store(mTrajectoryCurrentSpeed.load());

    computeCurrentTrajectoryPoint();
    applyCurrentTrajectoryPointToPrimarySource();
}

//==============================================================================
void TrajectoryManager::computeCurrentTrajectoryPoint()
{
    if (!mTrajectory.has_value()) {
        mCurrentTrajectoryPoint = extractTrajectoryPointFromPrimarySource();
        return;
    }

    auto const dampeningCyclesTimes2{ mDampeningCycles * 2 };
    double currentScaleMin{};
    double currentScaleMax{};

    if (mTrajectory->size() > 0) {
        if (mTrajectoryDeltaTime < mLastTrajectoryDeltaTime) {
            if (mIsBackAndForth) {
                this->invertBackAndForthDirection();
                mDampeningCycleCount++;
                if (mDampeningCycleCount >= dampeningCyclesTimes2) {
                    mDampeningCycleCount = dampeningCyclesTimes2;
                }
            }
            mDeviationCycleCount++;
        }
        mLastTrajectoryDeltaTime = mTrajectoryDeltaTime;

        double trajectoryPhase;
        if (mIsBackAndForth && mDampeningCycles > 0) {
            if (mTrajectoryDeltaTime <= 0.5) {
                trajectoryPhase = std::pow(mTrajectoryDeltaTime * 2.0, 2.0) * 0.5;
            } else {
                trajectoryPhase = 1.0 - std::pow(1.0 - ((mTrajectoryDeltaTime - 0.5) * 2.0), 2.0) * 0.5;
            }
        } else {
            trajectoryPhase = mTrajectoryDeltaTime;
        }

        double delta{ trajectoryPhase * mTrajectory->size() };

        if (mBackAndForthDirection == Direction::backward) {
            delta = mTrajectory->size() - delta;
        }

        delta = std::clamp(delta, 0.0, static_cast<double>(mTrajectory->size()));

        if (mIsBackAndForth && mDampeningCycles > 0) {
            if (mDampeningCycleCount < dampeningCyclesTimes2) {
                double const relativeDeltaTime{ (mDampeningCycleCount + mTrajectoryDeltaTime) / dampeningCyclesTimes2 };
                mCurrentPlaybackDuration
                    = mPlaybackDuration - (std::pow(relativeDeltaTime, 2.0) * mPlaybackDuration * 0.25);
                currentScaleMin = relativeDeltaTime * mTrajectory->size() * 0.5;
                currentScaleMax = mTrajectory->size() - currentScaleMin;
                double const currentScale{ (currentScaleMax - currentScaleMin) / mTrajectory->size() };
                delta = delta * currentScale + currentScaleMin;
                mDampeningLastDelta = delta;
            } else {
                delta = mDampeningLastDelta;
            }
        } else {
            mDampeningLastDelta = delta;
        }

        auto const deltaRatio{ static_cast<double>(mTrajectory->size() - 1) / mTrajectory->size() };
        delta *= deltaRatio;
        auto const index{ static_cast<int>(delta) };
        if (index + 1 < mTrajectory->size()) {
            Normalized const progression{ static_cast<float>(delta / mTrajectory->size()) };
            mCurrentTrajectoryPoint = mTrajectory->getPosition(progression);
        } else {
            mCurrentTrajectoryPoint = mTrajectory->getEndPosition();
        }
    }

    if (mDegreeOfDeviationPerCycle != Degrees{ 0.0f }) {
        auto deviationFlag{ true };
        if (mIsBackAndForth && mDampeningCycles > 0) {
            if (juce::approximatelyEqual(currentScaleMin, currentScaleMax)) {
                deviationFlag = false;
            }
        }
        if (deviationFlag) {
            mCurrentDegreeOfDeviation
                = mDegreeOfDeviationPerCycle * static_cast<float>(mDeviationCycleCount + mTrajectoryDeltaTime);
            if (mCurrentDegreeOfDeviation >= Degrees{ 360.0f }) {
                mCurrentDegreeOfDeviation -= Degrees{ 360.0f };
            }
        }

        mCurrentTrajectoryPoint = mCurrentTrajectoryPoint.rotatedAboutOrigin(mCurrentDegreeOfDeviation.getAsRadians());
    }
}

//==============================================================================
juce::Point<float> TrajectoryManager::getCurrentTrajectoryPoint() const
{
    if (mActivateState) {
        return mCurrentTrajectoryPoint;
    }
    return mPrimarySource.getPos();
}

//==============================================================================
void TrajectoryManager::sourceMoved([[maybe_unused]] Source & source)
{
    jassert(source.isPrimarySource());
    recomputeTrajectory();
}

//==============================================================================
void PositionTrajectoryManager::sendTrajectoryPositionChangedEvent()
{
    mListeners.call([&](Listener & l) {
        l.trajectoryPositionChanged(this, mPrimarySource.getPos(), mPrimarySource.getElevation());
    });
}

//==============================================================================
void PositionTrajectoryManager::recomputeTrajectory()
{
    this->setTrajectoryType(mTrajectoryType, mPrimarySource.getPos());
    mBackAndForthDirection = Direction::forward;
}

//==============================================================================
juce::Point<float> PositionTrajectoryManager::extractTrajectoryPointFromPrimarySource() const
{
    return mPrimarySource.getPos();
}

//==============================================================================
void ElevationTrajectoryManager::sendTrajectoryPositionChangedEvent()
{
    mListeners.call([&](Listener & l) {
        l.trajectoryPositionChanged(this, mPrimarySource.getPos(), mPrimarySource.getElevation());
    });
}

//==============================================================================
void PositionTrajectoryManager::setTrajectoryType(PositionTrajectoryType const type,
                                                  juce::Point<float> const & startPos)
{
    mTrajectoryType = type;
    if (type == PositionTrajectoryType::realtime) {
        mTrajectory.reset();
    } else {
        mTrajectory = Trajectory{ type, startPos };
    }
    mBackAndForthDirection = Direction::forward;
}

//==============================================================================
void TrajectoryManager::addRecordingPoint(juce::Point<float> const & pos)
{
    jassert(mTrajectory.has_value());
    mTrajectory->addPoint(smoothRecordingPosition(pos));
}

//==============================================================================
void TrajectoryManager::invertBackAndForthDirection()
{
    mBackAndForthDirection = mBackAndForthDirection == Direction::forward ? Direction::backward : Direction::forward;
}

//==============================================================================
void ElevationTrajectoryManager::setTrajectoryType(ElevationTrajectoryType const type)
{
    mTrajectoryType = type;

    if (type != ElevationTrajectoryType::realtime) {
        mTrajectory = Trajectory{ type };
    } else {
        mTrajectory.reset();
    }
    mBackAndForthDirection = Direction::forward;
}

//==============================================================================
void PositionTrajectoryManager::applyCurrentTrajectoryPointToPrimarySource()
{
    if (mActivateState) {
        mPrimarySource.setPosition(mCurrentTrajectoryPoint, Source::OriginOfChange::trajectory);
        sendTrajectoryPositionChangedEvent();
    }
}

//==============================================================================
void ElevationTrajectoryManager::applyCurrentTrajectoryPointToPrimarySource()
{
    if (mActivateState) {
        auto const currentElevation{ MAX_ELEVATION * (mCurrentTrajectoryPoint.getY() + 1.0f) / 2.0f };
        mPrimarySource.setElevation(currentElevation, Source::OriginOfChange::trajectory);
        sendTrajectoryPositionChangedEvent();
    }
}

//==============================================================================
void ElevationTrajectoryManager::recomputeTrajectory()
{
    this->setTrajectoryType(mTrajectoryType);
}

//==============================================================================
juce::Point<float> ElevationTrajectoryManager::extractTrajectoryPointFromPrimarySource() const
{
    return juce::Point<float>{ 0.0f, mPrimarySource.getNormalizedElevation().get() * 2.0f - 1.0f };
}
} // namespace gris
