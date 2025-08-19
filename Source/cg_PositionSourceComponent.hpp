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

#include "cg_PersistentStorage.h"
#include "cg_Source.hpp"
#include "cg_SourceComponent.hpp"
//#include "cg_StrongTypes.hpp"

namespace gris
{
class PositionFieldComponent;
class PositionTrajectoryManager;

//==============================================================================
class PositionSourceComponent final
    : public SourceComponent
    , public Source::Listener
{
    PositionFieldComponent & mFieldComponent;
    PositionTrajectoryManager & mTrajectoryManager;
    Source & mSource;
    PersistentStorage & mStorage;
    DisplacementMode mDisplacementMode{};
    bool mCanDrag{};

public:
    //==============================================================================
    PositionSourceComponent() = delete;
    ~PositionSourceComponent() override;

    PositionSourceComponent(PositionSourceComponent const &) = delete;
    PositionSourceComponent(PositionSourceComponent &&) = delete;

    PositionSourceComponent & operator=(PositionSourceComponent const &) = delete;
    PositionSourceComponent & operator=(PositionSourceComponent &&) = delete;
    //==============================================================================
    PositionSourceComponent(PositionFieldComponent & fieldComponent, Source & source, PersistentStorage & storage);
    //==============================================================================
    [[nodiscard]] SourceIndex getSourceIndex() const;

    void mouseDown(juce::MouseEvent const & event) override;
    void mouseDrag(juce::MouseEvent const & event) override;
    void mouseUp(juce::MouseEvent const & event) override;
    void updatePositionInParent() override;

private:
    //==============================================================================
    void setSourcePosition(juce::MouseEvent const & event) const;
    //==============================================================================
    void sourceMovedCallback() override;
    //==============================================================================
    JUCE_LEAK_DETECTOR(PositionSourceComponent)

}; // class PositionSourceComponent

} // namespace gris
