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

#include "cg_SourceComponent.hpp"
#include <Data/StrongTypes/sg_Radians.hpp>

namespace gris
{
class ElevationFieldComponent;

//==============================================================================
class ElevationDrawingHandle final : public SourceComponent
{
    ElevationFieldComponent & mFieldComponent;
    Radians mCurrentElevation;

public:
    //==============================================================================
    ElevationDrawingHandle() = delete;
    ~ElevationDrawingHandle() noexcept override = default;

    ElevationDrawingHandle(ElevationDrawingHandle const &) = delete;
    ElevationDrawingHandle(ElevationDrawingHandle &&) = delete;

    ElevationDrawingHandle & operator=(ElevationDrawingHandle const &) = delete;
    ElevationDrawingHandle & operator=(ElevationDrawingHandle &&) = delete;
    //==============================================================================
    explicit ElevationDrawingHandle(ElevationFieldComponent & fieldComponent) noexcept;
    //==============================================================================
    void mouseDown(juce::MouseEvent const & event) override;
    void mouseDrag(juce::MouseEvent const & event) override;
    void updatePositionInParent() override;

private:
    //==============================================================================
    JUCE_LEAK_DETECTOR(ElevationDrawingHandle)

}; // class ElevationDrawingHandle

} // namespace gris
