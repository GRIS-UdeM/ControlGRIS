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

#include "cg_SourceComponent.hpp"

#include "cg_constants.hpp"

namespace gris
{
//==============================================================================
SourceComponent::SourceComponent(juce::Colour const colour, juce::String const & icon) : mColour(colour), mIcon(icon)
{
    if (colour == juce::Colours::black && colour.getAlpha() == 0.0f) {
        jassertfalse;
    }
    constexpr auto dimension = SOURCE_FIELD_COMPONENT_DIAMETER + MAX_LINE_THICKNESS * 2.0f;
    constexpr auto rounded{ static_cast<int>(dimension + 0.5f) };
    setSize(rounded, rounded);
}

//==============================================================================
void SourceComponent::setSelected(bool const selected)
{
    mSelected = selected;
    repaint();
}

//==============================================================================
void SourceComponent::paint(juce::Graphics & g)
{
    auto const lineThickness{ static_cast<float>(mSelected ? MAX_LINE_THICKNESS : 1) };
    auto const saturation{ (mSelected) ? 1.0f : 0.8f };
    auto const colour{ mColour.withMultipliedSaturation(saturation) };
    auto const center{ getLocalBounds().getCentre().toFloat() };
    auto const area{
        juce::Rectangle<float>{ SOURCE_FIELD_COMPONENT_DIAMETER, SOURCE_FIELD_COMPONENT_DIAMETER }.withCentre(center)
    };

    g.setColour(colour);
    g.drawEllipse(area, lineThickness);
    g.setGradientFill(juce::ColourGradient(colour.darker(0.8f), center, colour, center / 5.0f, true));
    g.fillEllipse(area);
    g.setColour(juce::Colours::white);
    g.drawFittedText(mIcon, area.getSmallestIntegerContainer(), juce::Justification::centred, 1);
}

//==============================================================================
SourceComponent::DisplacementMode SourceComponent::getDisplacementMode(juce::MouseEvent const & event)
{
    auto const pressedModifierKey{ event.mods.getRawFlags() & DISPLACEMENT_MODIFIER };
    return pressedModifierKey ? DisplacementMode::selectedSourceOnly : DisplacementMode::all;
}

//==============================================================================
bool SourceComponent::isMoveAllowed(SourceComponent::DisplacementMode const displacementMode,
                                    bool const isPrimarySource,
                                    PositionSourceLink const sourceLink)
{
    if (isPrimarySource || displacementMode == DisplacementMode::all) {
        return true;
    }
    switch (sourceLink) {
    case PositionSourceLink::independent:
    case PositionSourceLink::circular:
    case PositionSourceLink::circularFixedAngle:
    case PositionSourceLink::circularFixedRadius:
    case PositionSourceLink::deltaLock:
    case PositionSourceLink::circularFullyFixed:
        return true;
    case PositionSourceLink::symmetricX:
    case PositionSourceLink::symmetricY:
        return false;
    case PositionSourceLink::undefined:
        break;
    }
    jassertfalse;
    return false;
}

//==============================================================================
bool SourceComponent::isMoveAllowed(SourceComponent::DisplacementMode displacementMode,
                                    bool isPrimarySource,
                                    ElevationSourceLink sourceLink)
{
    if (isPrimarySource || displacementMode == DisplacementMode::all) {
        return true;
    }
    switch (sourceLink) {
    case ElevationSourceLink::independent:
    case ElevationSourceLink::deltaLock:
        return true;
    case ElevationSourceLink::fixedElevation:
    case ElevationSourceLink::linearMax:
    case ElevationSourceLink::linearMin:
        return false;
    case ElevationSourceLink::undefined:
        break;
    }
    jassertfalse;
    return false;
}

} // namespace gris
