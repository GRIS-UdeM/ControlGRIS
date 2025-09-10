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

#include "cg_LinkStrategies.hpp"
#include "cg_Source.hpp"
#include "cg_SourceSnapshot.hpp"
#include "cg_constants.hpp"

namespace gris
{
//==============================================================================
class SourceLinkEnforcer
{
public:
    enum class OriginOfChange { user, automation };
    //==============================================================================
private:
    Sources & mSources;
    SourcesSnapshots mSnapshots{};
    PositionSourceLink mPositionSourceLink{ PositionSourceLink::undefined };
    ElevationSourceLink mElevationSourceLink{ ElevationSourceLink::undefined };
    std::unique_ptr<source_link_strategies::Base> mLinkStrategy;
    double mElevationSourceLinkScale{ 1.0 };

public:
    //==============================================================================
    SourceLinkEnforcer() = delete;
    ~SourceLinkEnforcer() noexcept = default;

    SourceLinkEnforcer(SourceLinkEnforcer const &) = delete;
    SourceLinkEnforcer(SourceLinkEnforcer &&) = delete;

    SourceLinkEnforcer & operator=(SourceLinkEnforcer const &) = delete;
    SourceLinkEnforcer & operator=(SourceLinkEnforcer &&) = delete;
    //==============================================================================
    SourceLinkEnforcer(Sources & sources, PositionSourceLink sourceLink);
    SourceLinkEnforcer(Sources & sources, ElevationSourceLink sourceLink);
    //==============================================================================
    void setSourceLink(ElevationSourceLink sourceLink, OriginOfChange originOfChange);
    void setSourceLink(PositionSourceLink sourceLink, OriginOfChange originOfChange);
    void numberOfSourcesChanged();
    void enforceSourceLink();
    void setElevationSourceLinkScale(double scale);

    [[nodiscard]] auto const & getSnapshots() const { return mSnapshots; }

    //==============================================================================
    void sourceMoved(Source & source);
    void anchorMoved(Source & source);

    void loadSnapshots(SourcesSnapshots const & snapshots) { mSnapshots = snapshots; }

private:
    //==============================================================================
    void primarySourceMoved();
    void secondarySourceMoved(SourceIndex sourceIndex);
    void primaryAnchorMoved();
    void secondaryAnchorMoved(SourceIndex sourceIndex);
    void saveCurrentPositionsToInitialStates();
    //==============================================================================
    JUCE_LEAK_DETECTOR(SourceLinkEnforcer)
}; // class SourceLinkEnforcer

} // namespace gris
