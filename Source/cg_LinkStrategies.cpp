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

#include "cg_LinkStrategies.hpp"

#include <cmath> // TEMP

#include "cg_Source.hpp"

namespace gris
{
namespace source_link_strategies
{
//==============================================================================
void Base::computeParameters(Sources const & finalStates, SourcesSnapshots const & initialStates)
{
    computeParameters_implementation(finalStates, initialStates);
    mInitialized = true;
}

//==============================================================================
void Base::enforce(Sources & finalStates, SourcesSnapshots const & initialState, SourceIndex const sourceIndex) const
{
    jassert(mInitialized);
    enforce_implementation(finalStates, initialState, sourceIndex);
}

//==============================================================================
void Base::enforce(Sources & finalStates, SourcesSnapshots const & initialState) const
{
    for (auto & source : finalStates) {
        if (source.isPrimarySource()) {
            continue; // do not enforce primary source
        }
        enforce(finalStates, initialState, source.getIndex());
    }
}

//==============================================================================
SourceSnapshot Base::computeInitialStateFromFinalState(Sources const & finalStates,
                                                       SourcesSnapshots const & initialStates,
                                                       SourceIndex const sourceIndex) const
{
    jassert(mInitialized);
    return computeInitialStateFromFinalState_implementation(finalStates, initialStates, sourceIndex);
}

//==============================================================================
std::unique_ptr<Base> Base::make(PositionSourceLink const sourceLink)
{
    switch (sourceLink) {
    case PositionSourceLink::independent:
        return std::make_unique<PositionIndependent>();
    case PositionSourceLink::circular:
        return std::make_unique<Circular>();
    case PositionSourceLink::circularFixedRadius:
        return std::make_unique<CircularFixedRadius>();
    case PositionSourceLink::circularFixedAngle:
        return std::make_unique<CircularFixedAngle>();
    case PositionSourceLink::circularFullyFixed:
        return std::make_unique<CircularFullyFixed>();
    case PositionSourceLink::symmetricX:
        return std::make_unique<SymmetricX>();
    case PositionSourceLink::symmetricY:
        return std::make_unique<SymmetricY>();
    case PositionSourceLink::deltaLock:
        return std::make_unique<PositionDeltaLock>();
    case PositionSourceLink::undefined:
        break;
    }
    jassertfalse;
    return nullptr;
}

//==============================================================================
std::unique_ptr<Base> Base::make(ElevationSourceLink const sourceLink)
{
    switch (sourceLink) {
    case ElevationSourceLink::independent:
        return std::make_unique<ElevationIndependent>();
    case ElevationSourceLink::fixedElevation:
        return std::make_unique<FixedElevation>();
    case ElevationSourceLink::linearMin:
        return std::make_unique<LinearMin>();
    case ElevationSourceLink::linearMax:
        return std::make_unique<LinearMax>();
    case ElevationSourceLink::deltaLock:
        return std::make_unique<ElevationDeltaLock>();
    case ElevationSourceLink::undefined:
    default:
        jassertfalse;
    }
    jassertfalse;
    return nullptr;
}

//==============================================================================
void PositionIndependent::enforce_implementation(Sources & finalStates,
                                                 SourcesSnapshots const & initialStates,
                                                 SourceIndex const sourceIndex) const
{
    finalStates[sourceIndex].setPosition(initialStates[sourceIndex].position, Source::OriginOfChange::link);
}

//==============================================================================
SourceSnapshot
    PositionIndependent::computeInitialStateFromFinalState_implementation(Sources const & finalStates,
                                                                          SourcesSnapshots const & /*initialStates*/,
                                                                          SourceIndex const sourceIndex) const
{
    return SourceSnapshot{ finalStates[sourceIndex] };
}

//==============================================================================
void Circular::computeParameters_implementation(Sources const & finalState, SourcesSnapshots const & initialStates)
{
    auto const & primarySourceFinalState{ finalState.getPrimarySource() };
    auto const & primarySourceInitialState{ initialStates.primary };

    auto constexpr notQuiteZero{ 0.01f };
    auto const primarySourceInitialAngle{ Radians::angleOf(primarySourceInitialState.position) };
    auto const primarySourceFinalAngle{ Radians::angleOf(primarySourceFinalState.getPos()) };
    mRotation = primarySourceFinalAngle - primarySourceInitialAngle;
    auto const primarySourceInitialRadius{ primarySourceInitialState.position.getDistanceFromOrigin() };
    auto const primarySourceFinalRadius{ primarySourceFinalState.getPos().getDistanceFromOrigin() };
    auto const radiusRatio{ primarySourceInitialRadius == 0.0f
                                ? notQuiteZero
                                : primarySourceFinalRadius / primarySourceInitialRadius };
    mRadiusRatio = radiusRatio == 0.0f ? notQuiteZero : radiusRatio;
}

//==============================================================================
void Circular::enforce_implementation(Sources & finalStates,
                                      SourcesSnapshots const & initialStates,
                                      SourceIndex const sourceIndex) const
{
    auto const finalPosition{ initialStates[sourceIndex].position.rotatedAboutOrigin(mRotation.getAsRadians())
                              * mRadiusRatio };
    finalStates[sourceIndex].setPosition(finalPosition, Source::OriginOfChange::link);
}

//==============================================================================
SourceSnapshot Circular::computeInitialStateFromFinalState_implementation(Sources const & finalStates,
                                                                          SourcesSnapshots const & initialStates,
                                                                          SourceIndex const sourceIndex) const
{
    SourceSnapshot newInitialState{ initialStates[sourceIndex] };
    auto const newInitialPosition{
        (finalStates[sourceIndex].getPos() / mRadiusRatio).rotatedAboutOrigin(-mRotation.getAsRadians())
    };
    newInitialState.position = newInitialPosition;
    return newInitialState;
}

//==============================================================================
void CircularFixedRadius::computeParameters_implementation(Sources const & finalStates,
                                                           SourcesSnapshots const & initialStates)
{
    auto const & primarySourceFinalState{ finalStates.getPrimarySource() };

    auto const primarySourceInitialAngle{ Radians::angleOf(initialStates.primary.position) };
    auto const primarySourceFinalAngle{ Radians::angleOf(primarySourceFinalState.getPos()) };
    mRotation = primarySourceFinalAngle - primarySourceInitialAngle;
    mRadius = primarySourceFinalState.getPos().getDistanceFromOrigin();
}

//==============================================================================
void CircularFixedRadius::enforce_implementation(Sources & finalStates,
                                                 SourcesSnapshots const & initialStates,
                                                 SourceIndex const sourceIndex) const
{
    auto const initialAngle{ Radians::angleOf(initialStates[sourceIndex].position) };
    auto const finalAngle{ (mRotation + initialAngle).getAsRadians() };
    juce::Point<float> const finalPosition{ std::cos(finalAngle) * mRadius, std::sin(finalAngle) * mRadius };

    finalStates[sourceIndex].setPosition(finalPosition, Source::OriginOfChange::link);
}

//==============================================================================
SourceSnapshot
    CircularFixedRadius::computeInitialStateFromFinalState_implementation(Sources const & finalStates,
                                                                          SourcesSnapshots const & initialStates,
                                                                          SourceIndex const sourceIndex) const
{
    auto const finalPosition{ finalStates[sourceIndex].getPos() };
    auto newInitialState{ initialStates[sourceIndex] };

    auto const finalAngle{ Radians::angleOf(finalPosition) };
    auto const inverseFinalAngle{ (finalAngle - mRotation).getAsRadians() };
    juce::Point<float> const newInitialPosition{ std::cos(inverseFinalAngle) * mRadius,
                                                 std::sin(inverseFinalAngle) * mRadius };

    newInitialState.position = newInitialPosition;
    return newInitialState;
}

//==============================================================================
void CircularFixedAngle::computeParameters_implementation(Sources const & finalStates,
                                                          SourcesSnapshots const & initialStates)
{
    mSecSourcesLengthRatio.resize(finalStates.MAX_NUMBER_OF_SOURCES);
    mOrdering.resize(finalStates.MAX_NUMBER_OF_SOURCES);

    auto const & primarySourceInitialState{ initialStates.primary };
    auto const & primarySourceFinalState{ finalStates.getPrimarySource() };

    // auto const notQuiteZero{ std::nextafter(0.0f, 1.0f) }; // dont divide by zero!
    auto constexpr notQuiteZero{ 0.001f };

    // initialize distance ratios of secondary sources
    if (!mSecSourcesLengthRatioInitialized) {
        std::fill(mSecSourcesLengthRatio.begin(), mSecSourcesLengthRatio.end(), 0.f);
        for (auto const & finalState : finalStates) {
            auto const sourceIndex{ finalState.getIndex() };
            auto const primaryDistFromOrig{ primarySourceInitialState.position.getDistanceFromOrigin() };
            auto const secDistFromOrig{ initialStates[sourceIndex].position.getDistanceFromOrigin() };
            float distPrimSecRatio{};
            if (primaryDistFromOrig == 0.0f && secDistFromOrig == 0.0f && sourceIndex.get() != 0) {
                distPrimSecRatio = 1;
            } else if (primaryDistFromOrig == 0.0f) {
                distPrimSecRatio = secDistFromOrig / notQuiteZero;
            } else if (secDistFromOrig == 0.0f) {
                distPrimSecRatio = notQuiteZero / primaryDistFromOrig;
            } else {
                distPrimSecRatio = secDistFromOrig / primaryDistFromOrig;
            }
            mSecSourcesLengthRatio[sourceIndex.get()] = distPrimSecRatio;
        }
        mSecSourcesLengthRatioInitialized = true;
    }

    auto const primarySourceFinalPosition{ primarySourceFinalState.getPos() };
    mPrimarySourceFinalAngle = Radians::angleOf(primarySourceFinalPosition);
    mDeviationPerSource = Degrees{ 360.0f } / static_cast<float>(finalStates.size());
    auto const primarySourceInitialAngle{ Radians::angleOf(primarySourceInitialState.position) };
    mRotation = mPrimarySourceFinalAngle - primarySourceInitialAngle;

    // copy initialAngles
    std::vector<std::pair<Degrees, SourceIndex>> initialAngles{};
    initialAngles.resize(finalStates.MAX_NUMBER_OF_SOURCES);
    for (auto const & finalState : finalStates) {
        auto const sourceIndex{ finalState.getIndex() };

        auto const & initialState{ initialStates[sourceIndex] };
        auto const initialAngle{ Radians::angleOf(initialState.position) };
        jassert(!std::isnan(initialAngle.getAsRadians()));

        initialAngles[sourceIndex.get()] = std::make_pair(initialAngle, sourceIndex);
    }
    // make all initialAngles bigger than the primary source's
    auto const minAngle{ Radians::angleOf(initialStates.primary.position) };
    auto const maxAngle{ minAngle + TWO_PI };
    std::for_each(std::begin(initialAngles),
                  std::begin(initialAngles) + finalStates.size(),
                  [&](std::pair<Degrees, SourceIndex> & data) {
                      while (data.first < minAngle) {
                          data.first += TWO_PI;
                      }
                      jassert(data.first >= minAngle);
                      jassert(data.first < maxAngle);
                  });
    // sort
    std::stable_sort(std::begin(initialAngles),
                     std::begin(initialAngles) + finalStates.size(),
                     [](auto const & a, auto const & b) -> bool { return a.first < b.first; });
    // store ordering
    for (int i{}; i < finalStates.size(); ++i) {
        auto const sourceIndex{ initialAngles[i].second };
        mOrdering[sourceIndex.get()] = i;
    }
    jassert(mOrdering[0ull] == 0);
}

//==============================================================================
void CircularFixedAngle::enforce_implementation(Sources & finalStates,
                                                SourcesSnapshots const & /*initialStates*/,
                                                SourceIndex const sourceIndex) const
{
    auto constexpr notQuiteZero{ 0.0000001f };
    auto const ordering{ mOrdering[sourceIndex.get()] };

    auto const finalAngle{ mPrimarySourceFinalAngle + mDeviationPerSource * static_cast<float>(ordering) };
    auto const primaryDistFromOrig{ finalStates.getPrimarySource().getPos().getDistanceFromOrigin() == 0.0f
                                        ? notQuiteZero
                                        : finalStates.getPrimarySource().getPos().getDistanceFromOrigin() };
    auto const radiusRatio = mSecSourcesLengthRatio[sourceIndex.get()];
    auto const finalRadius{ primaryDistFromOrig * radiusRatio };
    juce::Point<float> const finalPosition{ std::cos(finalAngle.getAsRadians()) * finalRadius,
                                            std::sin(finalAngle.getAsRadians()) * finalRadius };

    finalStates[sourceIndex].setPosition(finalPosition, Source::OriginOfChange::link);
}

SourceSnapshot
    CircularFixedAngle::computeInitialStateFromFinalState_implementation(Sources const & finalStates,
                                                                         SourcesSnapshots const & initialStates,
                                                                         SourceIndex const sourceIndex) const
{
    SourceSnapshot newInitialState{ initialStates[sourceIndex] };

    static const float notQuiteZero{ std::nextafter(0.0f, 1.0f) };
    auto const radiusRatio = mSecSourcesLengthRatio[sourceIndex.get()];
    auto const divisor{ std::max(notQuiteZero, radiusRatio) };
    auto const newInitialRadius{ finalStates[sourceIndex].getPos().getDistanceFromOrigin() / divisor };

    Radians const finalAngle{ std::atan2(finalStates[sourceIndex].getY(), finalStates[sourceIndex].getX()) };

    auto const newInitialAngle{ finalAngle - mRotation };

    juce::Point<float> const newInitialPosition{ std::cos(newInitialAngle.getAsRadians()) * newInitialRadius,
                                                 std::sin(newInitialAngle.getAsRadians()) * newInitialRadius };

    newInitialState.position = newInitialPosition;

    return newInitialState;
}

//==============================================================================
std::vector<float> CircularFixedAngle::getSecSourcesLengthRatio()
{
    return mSecSourcesLengthRatio;
}

//==============================================================================
void CircularFixedAngle::setSecSourcesLengthRatio(std::vector<float> & secSourcesLengthRatio)
{
    mSecSourcesLengthRatio = secSourcesLengthRatio;
}

//==============================================================================
void CircularFixedAngle::setSecSourcesLengthRatioInitialized()
{
    mSecSourcesLengthRatioInitialized = true;
}

//==============================================================================
void CircularFullyFixed::computeParameters_implementation(Sources const & finalStates,
                                                          SourcesSnapshots const & initialStates)
{
    mOrdering.resize(finalStates.MAX_NUMBER_OF_SOURCES);

    auto const & primarySourceInitialState{ initialStates.primary };
    auto const & primarySourceFinalState{ finalStates.getPrimarySource() };

    mRadius = primarySourceFinalState.getPos().getDistanceFromOrigin();

    auto const primarySourceFinalPosition{ primarySourceFinalState.getPos() };
    mPrimarySourceFinalAngle = Radians::angleOf(primarySourceFinalPosition);
    mDeviationPerSource = Degrees{ 360.0f } / static_cast<float>(finalStates.size());
    auto const primarySourceInitialAngle{ Radians::angleOf(primarySourceInitialState.position) };
    mRotation = mPrimarySourceFinalAngle - primarySourceInitialAngle;

    if (!mOrderingInitialized) {
        // copy initialAngles
        std::vector<std::pair<Degrees, SourceIndex>> initialAngles{};
        initialAngles.resize(finalStates.MAX_NUMBER_OF_SOURCES);
        for (auto const & finalState : finalStates) {
            auto const sourceIndex{ finalState.getIndex() };

            auto const & initialState{ initialStates[sourceIndex] };
            auto const initialAngle{ Radians::angleOf(initialState.position) };

            initialAngles[sourceIndex.get()] = std::make_pair(initialAngle, sourceIndex);
        }
        // make all initialAngles bigger than the primary source's
        auto const minAngle{ Radians::angleOf(initialStates.primary.position) };
        auto const maxAngle{ minAngle + TWO_PI };
        std::for_each(std::begin(initialAngles),
                      std::begin(initialAngles) + finalStates.size(),
                      [&](std::pair<Degrees, SourceIndex> & data) {
                          if (data.first < minAngle) {
                              data.first += TWO_PI;
                          }
                          jassert(data.first >= minAngle);
                          jassert(data.first < maxAngle);
                      });
        // sort
        std::stable_sort(std::begin(initialAngles),
                         std::begin(initialAngles) + finalStates.size(),
                         [](auto const & a, auto const & b) -> bool { return a.first < b.first; });
        // store ordering
        for (int i{}; i < finalStates.size(); ++i) {
            auto const sourceIndex{ initialAngles[i].second };
            mOrdering[sourceIndex.get()] = i;
        }
        mOrderingInitialized = true;
    }
    jassert(mOrdering[0ull] == 0);
}

//==============================================================================
void CircularFullyFixed::enforce_implementation(Sources & finalStates,
                                                SourcesSnapshots const & /*initialStates*/,
                                                SourceIndex const sourceIndex) const
{
    auto const ordering{ mOrdering[sourceIndex.get()] };

    auto const finalAngle{ mPrimarySourceFinalAngle + mDeviationPerSource * static_cast<float>(ordering) };
    juce::Point<float> const finalPosition{ std::cos(finalAngle.getAsRadians()) * mRadius,
                                            std::sin(finalAngle.getAsRadians()) * mRadius };

    finalStates[sourceIndex].setPosition(finalPosition, Source::OriginOfChange::link);
}

//==============================================================================
SourceSnapshot
    CircularFullyFixed::computeInitialStateFromFinalState_implementation(Sources const & finalStates,
                                                                         SourcesSnapshots const & initialStates,
                                                                         SourceIndex const sourceIndex) const
{
    SourceSnapshot newInitialState{ initialStates[sourceIndex] };

    Radians const finalAngle{ std::atan2(finalStates[sourceIndex].getY(), finalStates[sourceIndex].getX()) };
    auto const newInitialAngle{ finalAngle - mRotation };

    juce::Point<float> const newInitialPosition{ std::cos(newInitialAngle.getAsRadians()) * mRadius,
                                                 std::sin(newInitialAngle.getAsRadians()) * mRadius };

    newInitialState.position = newInitialPosition;

    return newInitialState;
}

//==============================================================================
std::vector<int> CircularFullyFixed::getOrdering()
{
    return mOrdering;
}

//==============================================================================
void CircularFullyFixed::setOrdering(std::vector<int> & ordering)
{
    mOrdering = ordering;
}

//==============================================================================
void CircularFullyFixed::setOrderingInitialized()
{
    mOrderingInitialized = true;
}

//==============================================================================
void SymmetricX::computeParameters_implementation(Sources const & finalStates,
                                                  SourcesSnapshots const & /*initialStates*/)
{
    mPrimarySourceFinalPosition = finalStates.getPrimarySource().getPos();
}

//==============================================================================
void SymmetricX::enforce_implementation(Sources & finalStates,
                                        SourcesSnapshots const & /*initialStates*/,
                                        SourceIndex const sourceIndex) const
{
    juce::Point<float> const finalPosition{ mPrimarySourceFinalPosition.getX(), -mPrimarySourceFinalPosition.getY() };
    finalStates[sourceIndex].setPosition(finalPosition, Source::OriginOfChange::link);
}

//==============================================================================
SourceSnapshot SymmetricX::computeInitialStateFromFinalState_implementation(Sources const & finalStates,
                                                                            SourcesSnapshots const & /*initialStates*/,
                                                                            SourceIndex const sourceIndex) const
{
    // nothing to do here!
    return SourceSnapshot{ finalStates[sourceIndex] };
}

//==============================================================================
void SymmetricY::computeParameters_implementation(Sources const & finalStates,
                                                  SourcesSnapshots const & /*initialStates*/)
{
    mPrimarySourceFinalPosition = finalStates.getPrimarySource().getPos();
}

//==============================================================================
void SymmetricY::enforce_implementation(Sources & finalStates,
                                        SourcesSnapshots const & /*initialStates*/,
                                        SourceIndex const sourceIndex) const
{
    juce::Point<float> const finalPosition{ -mPrimarySourceFinalPosition.getX(), mPrimarySourceFinalPosition.getY() };
    finalStates[sourceIndex].setPosition(finalPosition, Source::OriginOfChange::link);
}

//==============================================================================
SourceSnapshot SymmetricY::computeInitialStateFromFinalState_implementation(Sources const & finalStates,
                                                                            SourcesSnapshots const & /*initialStates*/,
                                                                            SourceIndex const sourceIndex) const
{
    // nothing to do here!
    return SourceSnapshot{ finalStates[sourceIndex] };
}

//==============================================================================
void PositionDeltaLock::computeParameters_implementation(Sources const & finalStates,
                                                         SourcesSnapshots const & initialStates)
{
    mDelta = finalStates.getPrimarySource().getPos() - initialStates.primary.position;
}

//==============================================================================
void PositionDeltaLock::enforce_implementation(Sources & finalStates,
                                               SourcesSnapshots const & initialStates,
                                               SourceIndex const sourceIndex) const
{
    auto const finalPosition{ initialStates[sourceIndex].position + mDelta };
    finalStates[sourceIndex].setPosition(finalPosition, Source::OriginOfChange::link);
}

//==============================================================================
SourceSnapshot
    PositionDeltaLock::computeInitialStateFromFinalState_implementation(Sources const & finalStates,
                                                                        SourcesSnapshots const & initialStates,
                                                                        SourceIndex const sourceIndex) const
{
    SourceSnapshot newInitialState{ initialStates[sourceIndex] };

    auto const newInitialPosition{ finalStates[sourceIndex].getPos() - mDelta };
    newInitialState.position = newInitialPosition;

    return newInitialState;
}

//==============================================================================
void ElevationIndependent::enforce_implementation(Sources & finalStates,
                                                  SourcesSnapshots const & initialStates,
                                                  SourceIndex const sourceIndex) const
{
    finalStates[sourceIndex].setElevation(initialStates[sourceIndex].z, Source::OriginOfChange::link);
}

//==============================================================================
SourceSnapshot
    ElevationIndependent::computeInitialStateFromFinalState_implementation(Sources const & finalStates,
                                                                           SourcesSnapshots const & /*initialStates*/,
                                                                           SourceIndex const sourceIndex) const
{
    return SourceSnapshot{ finalStates[sourceIndex] };
}

//==============================================================================
void FixedElevation::computeParameters_implementation(Sources const & finalStates,
                                                      SourcesSnapshots const & /*initialStates*/)
{
    mElevation = finalStates.getPrimarySource().getElevation();
}

//==============================================================================
void FixedElevation::enforce_implementation(Sources & finalStates,
                                            SourcesSnapshots const & /*initialStates*/,
                                            SourceIndex const sourceIndex) const
{
    finalStates[sourceIndex].setElevation(mElevation, Source::OriginOfChange::link);
}

//==============================================================================
SourceSnapshot FixedElevation::computeInitialStateFromFinalState_implementation(Sources const & /*finalStates*/,
                                                                                SourcesSnapshots const & initialStates,
                                                                                SourceIndex const sourceIndex) const
{
    return initialStates[sourceIndex];
}

//==============================================================================
void LinearMin::computeParameters_implementation(Sources const & sources, SourcesSnapshots const & /*snapshots*/)
{
    mBaseElevation = sources.getPrimarySource().getElevation();
    mElevationPerSource = ELEVATION_DIFF / static_cast<float>((sources.size() - 1));
}

//==============================================================================
void LinearMin::enforce_implementation(Sources & finalStates,
                                       SourcesSnapshots const & /*initialStates*/,
                                       SourceIndex const sourceIndex) const
{
    auto const newElevation{ mBaseElevation + mElevationPerSource * static_cast<float>(sourceIndex.get()) };
    finalStates[sourceIndex].setElevation(newElevation, Source::OriginOfChange::link);
}

//==============================================================================
SourceSnapshot LinearMin::computeInitialStateFromFinalState_implementation([[maybe_unused]] Sources const & finalStates,
                                                                           SourcesSnapshots const & /*initialStates*/,
                                                                           SourceIndex const sourceIndex) const
{
    SourceSnapshot result{};
    result.z = mBaseElevation - mElevationPerSource * static_cast<float>(sourceIndex.get());
    return result;
}

//==============================================================================
void LinearMax::computeParameters_implementation(Sources const & sources, SourcesSnapshots const & /*snapshots*/)
{
    mBaseElevation = sources.getPrimarySource().getElevation();
    mElevationPerSource = ELEVATION_DIFF / static_cast<float>((sources.size() - 1));
}

//==============================================================================
void LinearMax::enforce_implementation(Sources & finalStates,
                                       SourcesSnapshots const & /*initialStates*/,
                                       SourceIndex const sourceIndex) const
{
    auto const newElevation{ mBaseElevation + mElevationPerSource * static_cast<float>(sourceIndex.get()) };
    finalStates[sourceIndex].setElevation(newElevation, Source::OriginOfChange::link);
}

//==============================================================================
SourceSnapshot LinearMax::computeInitialStateFromFinalState_implementation(Sources const & /*finalStates*/,
                                                                           SourcesSnapshots const & initialStates,
                                                                           SourceIndex const sourceIndex) const
{
    return initialStates[sourceIndex];
}

//==============================================================================
void ElevationDeltaLock::computeParameters_implementation(Sources const & sources, SourcesSnapshots const & snapshots)
{
    mDelta = sources.getPrimarySource().getElevation() - snapshots.primary.z;
}

//==============================================================================
void ElevationDeltaLock::enforce_implementation(Sources & finalStates,
                                                SourcesSnapshots const & initialStates,
                                                SourceIndex const sourceIndex) const
{
    auto const newElevation{ initialStates[sourceIndex].z + mDelta };
    finalStates[sourceIndex].setElevation(newElevation, Source::OriginOfChange::link);
}

//==============================================================================
SourceSnapshot
    ElevationDeltaLock::computeInitialStateFromFinalState_implementation(Sources const & finalStates,
                                                                         SourcesSnapshots const & initialStates,
                                                                         SourceIndex const sourceIndex) const
{
    SourceSnapshot result{ initialStates[sourceIndex] };

    auto const initialElevation{ finalStates[sourceIndex].getElevation() - mDelta };
    result.z = initialElevation;

    return result;
}

} // namespace source_link_strategies

} // namespace gris
