/*
 This file is part of ControlGris.

 Developers: Olivier BELANGER, Ga�l LANE L�PINE

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

#include "cg_constants.hpp"

namespace gris
{
juce::String const Automation::Ids::X{ "recordingTrajectory_x" };
juce::String const Automation::Ids::Y{ "recordingTrajectory_y" };
juce::String const Automation::Ids::Z{ "recordingTrajectory_z" };
juce::String const Automation::Ids::POSITION_SOURCE_LINK{ "sourceLink" };
juce::String const Automation::Ids::ELEVATION_SOURCE_LINK{ "sourceLinkAlt" };
juce::String const Automation::Ids::AZIMUTH_SPAN{ "azimuthSpan" };
juce::String const Automation::Ids::ELEVATION_SPAN{ "elevationSpan" };
juce::String const Automation::Ids::POSITION_PRESET{ "positionPreset" };
juce::String const Automation::Ids::ELEVATION_MODE{ "elevationMode" };

Automation::Enum Automation::idToEnum(const juce::String & name)
{
    if (name.compare(Ids::X) == 0) {
        return Enum::x;
    }
    if (name.compare(Ids::Y) == 0) {
        return Enum::y;
    }
    if (name.compare(Ids::Z) == 0) {
        return Enum::z;
    }
    if (name.compare(Ids::POSITION_SOURCE_LINK) == 0) {
        return Enum::positionSourceLink;
    }
    if (name.compare(Ids::ELEVATION_SOURCE_LINK) == 0) {
        return Enum::elevationSourceLink;
    }
    if (name.compare(Ids::AZIMUTH_SPAN) == 0) {
        return Enum::azimuthSpan;
    }
    if (name.compare(Ids::ELEVATION_SPAN) == 0) {
        return Enum::elevationSpan;
    }
    if (name.compare(Ids::POSITION_PRESET) == 0) {
        return Enum::positionPreset;
    }
    if (name.compare(Ids::ELEVATION_MODE) == 0) {
        return Enum::elevationMode;
    }

    jassertfalse;
    return {};
}

juce::String const SOURCE_SELECTION_WARNING{ "This source link does not allow individual moves." };

juce::StringArray const SOURCE_PLACEMENT_SKETCH{
    juce::String("Left Alternate"),  juce::String("Right Alternate"),
    juce::String("Left Clockwise"),  juce::String("Left Counter Clockwise"),
    juce::String("Right Clockwise"), juce::String("Right Counter Clockwise"),
    juce::String("Top Clockwise"),   juce::String("Top Counter Clockwise")
};

juce::StringArray const POSITION_SOURCE_LINK_TYPES{
    juce::String("Independent"),           juce::String("Circular"),
    juce::String("Circular Fixed Radius"), juce::String("Circular Fixed Angle"),
    juce::String("Circular Fully Fixed"),  juce::String("Delta Lock"),
    juce::String("Symmetric X"),           juce::String("Symmetric Y")
};

juce::StringArray const ELEVATION_SOURCE_LINK_TYPES{ juce::String("Independent"),
                                                     juce::String("Equal Elevation"),
                                                     juce::String("Bottom-Top"),
                                                     juce::String("Top-Bottom"),
                                                     juce::String("Delta Lock") };

juce::StringArray const POSITION_TRAJECTORY_TYPE_TYPES{ juce::String("Realtime"),
                                                        juce::String("Drawing"),
                                                        juce::String("Circle Clockwise"),
                                                        juce::String("Circle Counter Clockwise"),
                                                        juce::String("Ellipse Clockwise"),
                                                        juce::String("Ellipse Counter Clockwise"),
                                                        juce::String("Spiral Clockwise Out In"),
                                                        juce::String("Spiral Counter Clockwise Out In"),
                                                        juce::String("Spiral Clockwise In Out"),
                                                        juce::String("Spiral Counter Clockwise In Out"),
                                                        juce::String("Square Clockwise"),
                                                        juce::String("Square Counter Clockwise"),
                                                        juce::String("Triangle Clockwise"),
                                                        juce::String("Triangle Counter Clockwise") };

juce::StringArray const ELEVATION_TRAJECTORY_TYPE_TYPES{ juce::String("Realtime"),
                                                         juce::String("Drawing"),
                                                         juce::String("Up Down"),
                                                         juce::String("Down Up") };

juce::StringArray const ELEVATION_MODE_TYPES{ juce::String("Normal"),
                                              juce::String("Extended Top"),
                                              juce::String("Extended Top and Bottom") };

juce::StringArray const FIXED_POSITION_DATA_HEADERS{
    // TODO: is this useful?
    juce::String("ID"),   juce::String("S1_X"), juce::String("S1_Y"), juce::String("S1_Z"), juce::String("S2_X"),
    juce::String("S2_Y"), juce::String("S2_Z"), juce::String("S3_X"), juce::String("S3_Y"), juce::String("S3_Z"),
    juce::String("S4_X"), juce::String("S4_Y"), juce::String("S4_Z"), juce::String("S5_X"), juce::String("S5_Y"),
    juce::String("S5_Z"), juce::String("S6_X"), juce::String("S6_Y"), juce::String("S6_Z"), juce::String("S7_X"),
    juce::String("S7_Y"), juce::String("S7_Z"), juce::String("S8_X"), juce::String("S8_Y"), juce::String("S8_Z"),
    juce::String("T1_X"), juce::String("T1_Y"), juce::String("T1_Z"), juce::String("T2_X"), juce::String("T2_Y"),
    juce::String("T2_Z")
};

juce::String const FIXED_POSITION_DATA_TAG("Fix_Position_Data");

juce::StringArray const AUDIO_DESCRIPTOR_TYPES{
    juce::String("-"),      juce::String("Loudness"), juce::String("Pitch"),           juce::String("Centroid"),
    juce::String("Spread"), juce::String("Noise"),    juce::String("Iterations Speed")
};

juce::StringArray const ONSET_DETECTION_METRIC_TYPES{ juce::String("Energy"),
                                                      juce::String("High Frequency Content"),
                                                      juce::String("Spectral Flux"),
                                                      juce::String("Rectified Complex Domain") };

} // namespace gris
