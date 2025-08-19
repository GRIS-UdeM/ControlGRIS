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

#include "cg_Source.hpp"
#include "cg_SourceComponent.hpp"

namespace gris
{
class ElevationFieldComponent;
class ElevationTrajectoryManager;

//==============================================================================
class ElevationSourceComponent final
    : public SourceComponent
    , public Source::Listener
{
    ElevationFieldComponent & mFieldComponent;
    ElevationTrajectoryManager & mTrajectoryManager;
    Source & mSource;
    DisplacementMode mDisplacementMode;
    bool mCanDrag{};

public:
    //==============================================================================
    ElevationSourceComponent() = delete;
    ~ElevationSourceComponent() noexcept override;

    ElevationSourceComponent(ElevationSourceComponent const &) = delete;
    ElevationSourceComponent(ElevationSourceComponent &&) = delete;

    ElevationSourceComponent & operator=(ElevationSourceComponent const &) = delete;
    ElevationSourceComponent & operator=(ElevationSourceComponent &&) = delete;
    //==============================================================================
    ElevationSourceComponent(ElevationFieldComponent & fieldComponent, Source & source) noexcept;
    //==============================================================================
    [[nodiscard]] SourceIndex getSourceIndex() const;

    void mouseDown(juce::MouseEvent const & event) override;
    void mouseDrag(juce::MouseEvent const & event) override;
    void mouseUp(juce::MouseEvent const & event) override;
    void updatePositionInParent() override;

protected:
    //==============================================================================
    void sourceMovedCallback() override;

private:
    //==============================================================================
    void setSourcePosition(juce::MouseEvent const & event) const;
    //==============================================================================
    JUCE_LEAK_DETECTOR(ElevationSourceComponent)

}; // Class ElevationSourceComponent

} // namespace gris
