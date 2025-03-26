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

#include "cg_StrongTypes.hpp"

namespace gris
{
//==============================================================================
// Global variables.
constexpr int MIN_FIELD_WIDTH = 300;

constexpr int NUMBER_OF_POSITION_PRESETS = 50;
constexpr float SOURCE_FIELD_COMPONENT_RADIUS = 12.0f;
constexpr float SOURCE_FIELD_COMPONENT_DIAMETER = SOURCE_FIELD_COMPONENT_RADIUS * 2.0f;
constexpr auto LBAP_FAR_FIELD = 1.666666667f;

constexpr Radians MIN_ELEVATION = Degrees{ 0.0f };
constexpr Radians MAX_ELEVATION = Degrees{ 90.0f };

//==============================================================================
// Spatialisation modes.
enum class SpatMode { dome, cube };

//==============================================================================
// Elevation modes.
enum class ElevationMode { normal, extendedTop, extendedTopAndBottom };
extern juce::StringArray const ELEVATION_MODE_TYPES;

extern juce::String const SOURCE_SELECTION_WARNING;

//==============================================================================
// Configuration -> Source tab -> Source Placement popup choices.
enum class SourcePlacement {
    undefined,
    leftAlternate,
    rightAlternate,
    leftClockwise,
    leftCounterClockwise,
    rightClockwise,
    rightCounterClockwise,
    topClockwise,
    topCounterClockwise
};

extern juce::StringArray const SOURCE_PLACEMENT_SKETCH;

//==============================================================================
// Trajectories -> Source Link popup choices.
enum class PositionSourceLink {
    undefined,
    independent,
    circular,
    circularFixedRadius,
    circularFixedAngle,
    circularFullyFixed,
    deltaLock,
    symmetricX,
    symmetricY
};

extern juce::StringArray const POSITION_SOURCE_LINK_TYPES;

enum class ElevationSourceLink { undefined, independent, fixedElevation, linearMin, linearMax, deltaLock };
extern juce::StringArray const ELEVATION_SOURCE_LINK_TYPES;

//==============================================================================
// Trajectories -> Trajectory Type popup choices.
enum class PositionTrajectoryType {
    undefined,
    realtime,
    drawing,
    circleClockwise,
    circleCounterClockwise,
    ellipseClockwise,
    ellipseCounterClockwise,
    spiralClockwiseOutIn,
    spiralCounterClockwiseOutIn,
    spiralClockwiseInOut,
    spiralCounterClockwiseInOut,
    squareClockwise,
    squareCounterClockwise,
    triangleClockwise,
    triangleCounterClockwise
};

extern juce::StringArray const POSITION_TRAJECTORY_TYPE_TYPES;

enum class ElevationTrajectoryType { undefined, realtime, drawing, downUp, upDown };
extern juce::StringArray const ELEVATION_TRAJECTORY_TYPE_TYPES;

//==============================================================================
extern juce::String const FIXED_POSITION_DATA_TAG;

constexpr auto SPEAKER_SETUP_XML_TAG = "SPEAKER_SETUP";
constexpr auto SPEAKER_SETUP_POS_PREFIX = "SPEAKER_";
constexpr auto SPEAKER_SETUP_POSITION_XML_TAG = "POSITION";

constexpr auto PRESET_XML_TAG = "ITEM";
constexpr auto PRESET_ID_XML_TAG = "ID";
constexpr auto PRESET_FIRST_SOURCE_ID_XML_TAG = "firstSourceId";
constexpr auto PRESET_SPAT_MODE_XML_TAG = "SPAT_MODE";

// taken from sg_SpatMode.cpp in SpatGRIS
constexpr auto SPAT_MODE_STRINGS = std::array<const char *, 2>{ "Dome", "Cube" };

namespace Automation
{
namespace Ids
{
juce::String extern const X;
juce::String extern const Y;
juce::String extern const Z;
juce::String extern const POSITION_SOURCE_LINK;
juce::String extern const ELEVATION_SOURCE_LINK;
juce::String extern const AZIMUTH_SPAN;
juce::String extern const ELEVATION_SPAN;
juce::String extern const POSITION_PRESET;
juce::String extern const ELEVATION_MODE;
} // namespace Ids

enum class Enum {
    x,
    y,
    z,
    positionSourceLink,
    elevationSourceLink,
    azimuthSpan,
    elevationSpan,
    positionPreset,
    elevationMode
};

Enum idToEnum(juce::String const & name);

} // namespace Automation

} // namespace gris
