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

#include "cg_ElevationDrawingHandle.hpp"

#include "cg_FieldComponent.hpp"

namespace gris
{
//==============================================================================
void ElevationDrawingHandle::mouseDown(juce::MouseEvent const & event)
{
    constexpr float DUMMY{ 0.0f };

    auto const relativePosition{ event.getEventRelativeTo(getParentComponent()).getPosition().toFloat() };
    auto const elevation{ mFieldComponent.componentPositionToSourceElevation(relativePosition) };
    mCurrentElevation = elevation;
    juce::Point<float> const position{ DUMMY, elevation / Radians{ MAX_ELEVATION } * 2.0f - 1.0f };

    auto & automationManager{ mFieldComponent.getAutomationManager() };
    automationManager.resetRecordingTrajectory(position);
    automationManager.addRecordingPoint(position); // two points minimum are needed for elevation drawing

    setCentrePosition(mFieldComponent.sourceElevationToComponentPosition(elevation, SourceIndex{ -1 }).toInt());
    mFieldComponent.repaint();
}

//==============================================================================
void ElevationDrawingHandle::mouseDrag(juce::MouseEvent const & event)
{
    auto & automationManager{ mFieldComponent.getAutomationManager() };
    auto const relativePosition{ event.getEventRelativeTo(getParentComponent()).getPosition().toFloat() };
    auto const elevation{ mFieldComponent.componentPositionToSourceElevation(relativePosition) };
    mCurrentElevation = elevation;
    constexpr auto DUMMY{ 0.0f };
    juce::Point<float> const position{ DUMMY, elevation / Radians{ MAX_ELEVATION } * 2.0f - 1.0f };

    automationManager.addRecordingPoint(position);

    setCentrePosition(mFieldComponent.sourceElevationToComponentPosition(elevation, SourceIndex{ -1 }).toInt());
    mFieldComponent.repaint();
}

//==============================================================================
void ElevationDrawingHandle::updatePositionInParent()
{
    auto const newSourcePosition{
        mFieldComponent.sourceElevationToComponentPosition(mCurrentElevation, SourceIndex{ -1 }).toInt()
    };
    setCentrePosition(newSourcePosition);
}

//==============================================================================
ElevationDrawingHandle::ElevationDrawingHandle(ElevationFieldComponent & fieldComponent) noexcept
    : SourceComponent(juce::Colour::fromRGB(176, 176, 228), "X")
    , mFieldComponent(fieldComponent)
{
    setSelected(true);
}

} // namespace gris
