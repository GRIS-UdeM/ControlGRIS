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

#include <algorithm>

#include "cg_Source.hpp"

#include "cg_ControlGrisAudioProcessor.hpp"

namespace gris
{
bool Source::shouldForceNotifications(Source::OriginOfChange const origin) const
{
    switch (origin) {
    case Source::OriginOfChange::none:
    case Source::OriginOfChange::userMove:
    case Source::OriginOfChange::userAnchorMove:
    case Source::OriginOfChange::automation:
    case Source::OriginOfChange::osc:
        return false;
    case Source::OriginOfChange::trajectory:
    case Source::OriginOfChange::link:
    case Source::OriginOfChange::presetRecall:
        return isPrimarySource();
    }
    jassertfalse;
    return false;
}

//==============================================================================
void Source::setAzimuth(Radians const azimuth, OriginOfChange const origin)
{
    auto const balancedAzimuth{ azimuth.centered() };
    if (balancedAzimuth != mAzimuth || shouldForceNotifications(origin)) {
        mAzimuth = balancedAzimuth;
        computeXY();
        notify(ChangeType::position, origin);
    }
}

//==============================================================================
void Source::setAzimuth(Normalized const normalizedAzimuth, OriginOfChange const origin)
{
    setAzimuth(TWO_PI * normalizedAzimuth.get() - PI, origin);
}

//==============================================================================
Normalized Source::getNormalizedAzimuth() const
{
    return Normalized{ (mAzimuth + PI) / TWO_PI };
}

//==============================================================================
void Source::setElevation(Radians const elevation, OriginOfChange const origin)
{
    auto const clippedElevation{ clipElevation(elevation) };
    if (clippedElevation != mElevation || shouldForceNotifications(origin)) {
        mElevation = clippedElevation;
        computeXY();
        notify(ChangeType::elevation, origin);
    }
}

//==============================================================================
void Source::setElevation(Normalized const normalizedElevation, OriginOfChange const origin)
{
    auto const newValue{ HALF_PI * normalizedElevation.get() };
    setElevation(newValue, origin);
}

//==============================================================================
void Source::setDistance(float const distance, OriginOfChange const origin)
{
    jassert(distance >= 0.0f);

    if (distance != mDistance || shouldForceNotifications(origin)) {
        mDistance = distance;
        computeXY();
        notify(ChangeType::position, origin);
    }
}

//==============================================================================
void Source::setCoordinates(Radians const azimuth,
                            Radians const elevation,
                            float const distance,
                            OriginOfChange const origin)
{
    auto const balancedAzimuth{ azimuth.centered() };
    auto const clippedElevation{ clipElevation(elevation) };
    jassert(distance >= 0.0f);

    if (balancedAzimuth != mAzimuth || clippedElevation != mElevation || distance != mDistance
        || shouldForceNotifications(origin)) {
        mAzimuth = azimuth;
        mElevation = elevation;
        mDistance = distance;
        computeXY();
        notify(ChangeType::position, origin);
        if (mSpatMode == SpatMode::cube) {
            notify(ChangeType::elevation, origin);
        }
    }
}

//==============================================================================
void Source::setAzimuthSpan(Normalized const azimuthSpan)
{
    if (mAzimuthSpan != azimuthSpan) {
        mAzimuthSpan = azimuthSpan;
        notifyGuiListeners();
    }
}

//==============================================================================
void Source::setElevationSpan(Normalized const elevationSpan)
{
    if (mElevationSpan != elevationSpan) {
        mElevationSpan = elevationSpan;
        notifyGuiListeners();
    }
}

//==============================================================================
void Source::setX(float const x, OriginOfChange const origin)
{
    auto const clippedX{ clipCoordinate(x) };
    if (clippedX != mPosition.getX() || shouldForceNotifications(origin)) {
        mPosition.setX(clippedX);
        computeAzimuthElevation();
        notify(ChangeType::position, origin);
    }
}

//==============================================================================
void Source::setX(Normalized const x, OriginOfChange const origin)
{
    setX(x.get() * 2.0f - 1.0f, origin);
}

//==============================================================================
void Source::setY(Normalized const y, OriginOfChange const origin)
{
    setY(y.get() * 2.0f - 1.0f, origin);
}

//==============================================================================
void Source::setY(float const y, OriginOfChange const origin)
{
    auto const clippedY{ clipCoordinate(y) };
    if (y != mPosition.getY() || shouldForceNotifications(origin)) {
        mPosition.setY(clippedY);
        computeAzimuthElevation();
        notify(ChangeType::position, origin);
    }
}

//==============================================================================
void Source::setPosition(juce::Point<float> const & position, OriginOfChange const origin)
{
    auto const clippedPosition{ clipPosition(position, mSpatMode) };
    if (mPosition != clippedPosition || shouldForceNotifications(origin)) {
        mPosition = clippedPosition;
        computeAzimuthElevation();
        notify(ChangeType::position, origin);
    }
}

//==============================================================================
void Source::computeXY()
{
    float const radius{ [&] {
        if (mSpatMode == SpatMode::dome) { // azimuth - elevation
            jassert(!std::isnan(mElevation.getAsRadians()));
            auto const result{ mElevation / MAX_ELEVATION };
            jassert(result >= 0.0f && result <= 1.0f);
            return result;
        }
        jassert(!std::isnan(mDistance));
        return mDistance;
    }() };

    jassert(!std::isnan(mAzimuth.getAsRadians()));
    auto const newPosition{ getPositionFromAngle(mAzimuth, radius) };
    mPosition = newPosition;
}

//==============================================================================
void Source::computeAzimuthElevation()
{
    jassert(!std::isnan(mPosition.getX()) && !std::isnan(mPosition.getY()));

    // update the azimuth only if we're not exactly at origin.
    if (! mPosition.isOrigin()) {
        // TODO : when the position converges to the origin via an automation, one of the dimension is going to get to
        // zero before the other. This is going to drastically change the angle. We need to insulate the real automation
        // from a listener callback initiated by some other source.

#if DEBUG_COORDINATES
        DBG("angle: " + getAngleFromPosition(mPosition).toString());
        DBG("centered angle: " + getAngleFromPosition(mPosition).centered().toString());
#endif
        mAzimuth = getAngleFromPosition(mPosition).centered();
    }

    // update both the elevation and distance in dome mode, otherwise only set the distance to the origin
    auto const radius{ mPosition.getDistanceFromOrigin() };
#if DEBUG_COORDINATES
    DBG("OG Radius: " + juce::String (radius));
#endif

    if (mSpatMode == SpatMode::dome) {
        auto const clippedRadius{ std::min(radius, 1.0f) };
#if DEBUG_COORDINATES
        DBG("clippedRadius: " + juce::String(clippedRadius));
#endif

        if (clippedRadius < radius) {
            //when does this actually happen? and shouldn't we also update mAzimuth with the clipped radius here??
            jassertfalse;

            jassert(!std::isnan(mAzimuth.getAsRadians()));
            mPosition = getPositionFromAngle(mAzimuth, clippedRadius);
#if DEBUG_COORDINATES
            DBG("mPosition: " + mPosition.toString());
#endif
        }

        mElevation = HALF_PI * clippedRadius;
#if DEBUG_COORDINATES
        DBG("mElevation : " + juce::String (mElevation.get()));
#endif

        mDistance = clippedRadius;
    } else {
        mDistance = radius;
    }
#if DEBUG_COORDINATES
    DBG("mDistance: " + juce::String(mDistance));
#endif
}

//==============================================================================
Normalized Source::getNormalizedElevation() const
{
    return Normalized{ mElevation / HALF_PI };
}

//==============================================================================
juce::Point<float> Source::getPositionFromAngle(Radians const angle, float const radius)
{
    auto const rotatedAngle{ angle - HALF_PI };
    juce::Point<float> const result{ std::cos(rotatedAngle.getAsRadians()) * radius,
                                     std::sin(rotatedAngle.getAsRadians()) * radius };
    return result;
}

//==============================================================================
Radians Source::getAngleFromPosition(juce::Point<float> const & position)
{
    auto const getAngle = [](juce::Point<float> const & pos) {
        if (pos.getX() == 0.0f && pos.getY() == 0.0f) {
            return 0.0f;
        }
        return std::atan2(pos.getY(), pos.getX());
    };
    Radians const angle{ getAngle(position) };
    auto const rotatedAngle{ angle + HALF_PI };
    return rotatedAngle;
}

//==============================================================================
juce::Point<float> Source::clipDomePosition(juce::Point<float> const & position)
{
    juce::Point<float> result{};
    auto const radius{ position.getDistanceFromOrigin() };
    if (radius > 1.0f) {
        auto const angle{ std::atan2(position.getY(), position.getX()) };
        result = juce::Point<float>{ std::cos(angle), std::sin(angle) };
    } else {
        result = position;
    }
    return result;
}

//==============================================================================
juce::Point<float> Source::clipCubePosition(juce::Point<float> const & position)
{
    juce::Point<float> const result{ std::clamp(position.getX(), -1.0f, 1.0f),
                                     std::clamp(position.getY(), -1.0f, 1.0f) };
    return result;
}

//==============================================================================
void Source::notify(ChangeType type, OriginOfChange const origin)
{
    if (mSpatMode == SpatMode::dome) {
        type = ChangeType::position;
    }

    mProcessor->sourceChanged(*this, type, origin);

    notifyGuiListeners();
}

//==============================================================================
juce::Point<float> Source::clipPosition(juce::Point<float> const & position, SpatMode const spatMode)
{
    juce::Point<float> result{};
    if (spatMode == SpatMode::dome) {
        result = clipDomePosition(position);
    } else {
        jassert(spatMode == SpatMode::cube);
        result = clipCubePosition(position);
    }
    return result;
}

//==============================================================================
Radians Source::clipElevation(Radians const elevation)
{
    return Radians{ std::clamp(elevation, Radians{ 0.0f }, HALF_PI) };
}

//==============================================================================
float Source::clipCoordinate(float const coord)
{
    return std::clamp(coord, -1.0f, 1.0f);
}

//==============================================================================
void Source::setColorFromIndex(int const numTotalSources)
{
    auto hue{ static_cast<float>(mIndex.get()) / static_cast<float>(numTotalSources) + 0.577251f };
    if (hue > 1.0f) {
        hue -= 1.0f;
    }
    mColour = juce::Colour::fromHSV(hue, 1.0f, 1.0f, 0.85f);
    notifyGuiListeners();
}

//==============================================================================
void Source::notifyGuiListeners()
{
    auto callback = [=](Source::Listener & listener) { listener.update(); };
    mGuiListeners.call(callback);
}

//==============================================================================
void Sources::setSize(int const size)
{
    jassert(size >= 1 && size <= MAX_NUMBER_OF_SOURCES);
    mSize = size;
    auto const azimuthSpan{ mPrimarySource.getAzimuthSpan() };
    auto const elevationSpan{ mPrimarySource.getElevationSpan() };
    for (auto & source : *this) {
        source.setColorFromIndex(size);
        source.setAzimuthSpan(azimuthSpan);
        source.setElevationSpan(elevationSpan);
    }
}

} // namespace gris
