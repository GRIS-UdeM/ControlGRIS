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

#include "cg_SourceLinkEnforcer.hpp"

#include "cg_constants.hpp"

namespace gris
{
//==============================================================================
SourceLinkEnforcer::SourceLinkEnforcer(Sources & sources, PositionSourceLink const sourceLink) : mSources(sources)
{
    setSourceLink(sourceLink, OriginOfChange::user);
}

//==============================================================================
SourceLinkEnforcer::SourceLinkEnforcer(Sources & sources, ElevationSourceLink const sourceLink) : mSources(sources)
{
    setSourceLink(sourceLink, OriginOfChange::user);
}

//==============================================================================
void SourceLinkEnforcer::setSourceLink(PositionSourceLink const sourceLink, OriginOfChange const originOfChange)
{
    // If it's the user who changed the source link, we need to bake the positions before making the change. Otherwise,
    // we use the current positions to that time jumps in the DAW always result in the same position regardless of where
    // the sources were.
    jassert(sourceLink != PositionSourceLink::undefined);
    jassert(originOfChange == OriginOfChange::automation || originOfChange == OriginOfChange::user);

    if (originOfChange == OriginOfChange::user) {
        saveCurrentPositionsToInitialStates();
    }

    if (sourceLink != mPositionSourceLink) {
        mPositionSourceLink = sourceLink;
        mElevationSourceLink = ElevationSourceLink::undefined;
        mLinkStrategy = source_link_strategies::Base::make(sourceLink);
        enforceSourceLink();
    }
}

//==============================================================================
void SourceLinkEnforcer::setSourceLink(ElevationSourceLink const sourceLink, OriginOfChange const originOfChange)
{
    // If it's the user who changed the source link, we need to bake the positions before making the change. Otherwise,
    // we use the current positions to that time jumps in the DAW always result in the same position regardless of where
    // the sources were.
    jassert(sourceLink != ElevationSourceLink::undefined);
    jassert(originOfChange == OriginOfChange::automation || originOfChange == OriginOfChange::user);

    if (originOfChange == OriginOfChange::user) {
        saveCurrentPositionsToInitialStates();
    }

    if (sourceLink != mElevationSourceLink) {
        mElevationSourceLink = sourceLink;
        mPositionSourceLink = PositionSourceLink::undefined;
        mLinkStrategy = source_link_strategies::Base::make(sourceLink);
        enforceSourceLink();
    }
}

//==============================================================================
void SourceLinkEnforcer::enforceSourceLink()
{
    mLinkStrategy->computeParameters(mSources, mSnapshots);
    mLinkStrategy->enforce(mSources, mSnapshots);
    if (mPositionSourceLink == PositionSourceLink::circularFixedAngle
        || mPositionSourceLink == PositionSourceLink::circularFullyFixed) {
        // circularFixedAngle & circularFullyFixed links require the snapshots to be up-to-date or else moving the
        // relative ordering with the mouse won't make any sense.
        saveCurrentPositionsToInitialStates();
    }
}

//==============================================================================
void SourceLinkEnforcer::sourceMoved(Source & source)
{
    if (source.isPrimarySource()) {
        primarySourceMoved();
    } else {
        secondarySourceMoved(source.getIndex());
    }
}

//==============================================================================
void SourceLinkEnforcer::anchorMoved(Source & source)
{
    if (source.isPrimarySource()) {
        primaryAnchorMoved();
    } else {
        secondaryAnchorMoved(source.getIndex());
    }
}

//==============================================================================
void SourceLinkEnforcer::numberOfSourcesChanged()
{
    saveCurrentPositionsToInitialStates();
}

//==============================================================================
void SourceLinkEnforcer::primarySourceMoved()
{
    enforceSourceLink();
}

//==============================================================================
void SourceLinkEnforcer::secondarySourceMoved(SourceIndex const sourceIndex)
{
    jassert(sourceIndex.get() > 0 && sourceIndex.get() < mSources.MAX_NUMBER_OF_SOURCES);

    auto const spatMode{ mSources.getPrimarySource().getSpatMode() };
    auto const isElevationSourceLink{ mElevationSourceLink != ElevationSourceLink::undefined };
    if (spatMode == SpatMode::dome && isElevationSourceLink) {
        return;
    }

    if (mElevationSourceLink == ElevationSourceLink::independent
        || mPositionSourceLink == PositionSourceLink::independent) {
        secondaryAnchorMoved(sourceIndex);
        return;
    }

    if (isElevationSourceLink) {
        // get expected elevation
        auto const currentElevation{ mSources[sourceIndex].getElevation() };
        mLinkStrategy->computeParameters(mSources, mSnapshots);
        mLinkStrategy->enforce_implementation(mSources, mSnapshots, sourceIndex);
        auto const expectedElevation{ mSources[sourceIndex].getElevation() };

        // get motion
        auto const motion{ currentElevation - expectedElevation };

        // apply motion to primary source
        mSources.getPrimarySource().setElevation(mSources.getPrimarySource().getElevation() + motion,
                                                 Source::OriginOfChange::link);
    } else {
        auto const primaryStart{ mSnapshots.primary };
        SourceSnapshot const primaryEnd{ mSources.getPrimarySource() };
        auto const secondaryStart{ mSnapshots[sourceIndex] };
        SourceSnapshot const secondaryEnd{ mSources[sourceIndex] };

        // take a snapshot of sources length ratios if mLinkStrategy is circularFixedAngle
        auto tmpFixedAngleSecSourcesLengthRatio = std::vector<float>(mSources.MAX_NUMBER_OF_SOURCES);
        if (mLinkStrategy.get()->isInitialized()) {
            if (mPositionSourceLink == PositionSourceLink::circularFixedAngle) {
                source_link_strategies::CircularFixedAngle * circularFixedLinkStrategy
                    = static_cast<source_link_strategies::CircularFixedAngle *>(mLinkStrategy.get());
                tmpFixedAngleSecSourcesLengthRatio = circularFixedLinkStrategy->getSecSourcesLengthRatio();
            }
        }

        // take a snapshot of source ordering if mLinkStrategy is circularFullyFixed
        auto tmpCircularFullyFixedOrdering = std::vector<int>(mSources.MAX_NUMBER_OF_SOURCES);
        if (mLinkStrategy.get()->isInitialized()) {
            if (mPositionSourceLink == PositionSourceLink::circularFullyFixed) {
                source_link_strategies::CircularFullyFixed * circularFullyFixedLinkStrategy
                    = static_cast<source_link_strategies::CircularFullyFixed *>(mLinkStrategy.get());
                tmpCircularFullyFixedOrdering = circularFullyFixedLinkStrategy->getOrdering();
            }
        }

        if (mPositionSourceLink == PositionSourceLink::circularFixedAngle
            || mPositionSourceLink == PositionSourceLink::circularFullyFixed) {
            mLinkStrategy = source_link_strategies::Base::make(PositionSourceLink::circular);
        }
        mLinkStrategy->computeParameters(mSources, mSnapshots);

        // get motion start and end
        mLinkStrategy->enforce_implementation(mSources, mSnapshots, sourceIndex);
        SourceSnapshot const motionStart{ mSources[sourceIndex] };
        auto const motionEnd{ secondaryEnd };

        // train motion
        mSnapshots.primary = motionStart;
        mSources.getPrimarySource().setPosition(motionEnd.position, Source::OriginOfChange::none);
        mLinkStrategy->computeParameters(mSources, mSnapshots);

        // apply motion to secondary source (temp)
        mSnapshots.primary = primaryEnd;
        mSnapshots[sourceIndex] = primaryEnd;
        mLinkStrategy->enforce_implementation(mSources, mSnapshots, sourceIndex);
        SourceSnapshot const target{ mSources[sourceIndex] };

        // rebuild source snapshot
        mSnapshots[sourceIndex] = secondaryStart;

        // enforce link
        if (mPositionSourceLink == PositionSourceLink::circularFixedAngle
            || mPositionSourceLink == PositionSourceLink::circularFullyFixed) {
            if (mPositionSourceLink == PositionSourceLink::undefined) {
                mLinkStrategy = source_link_strategies::Base::make(mElevationSourceLink); // how can we reach this ?
            } else {
                mLinkStrategy = source_link_strategies::Base::make(mPositionSourceLink);
                if (mPositionSourceLink == PositionSourceLink::circularFixedAngle) {
                    source_link_strategies::CircularFixedAngle * circularFixedLinkStrategy
                        = static_cast<source_link_strategies::CircularFixedAngle *>(mLinkStrategy.get());
                    circularFixedLinkStrategy->setSecSourcesLengthRatio(tmpFixedAngleSecSourcesLengthRatio);
                    circularFixedLinkStrategy->setSecSourcesLengthRatioInitialized();
                } else if (mPositionSourceLink == PositionSourceLink::circularFullyFixed) {
                    source_link_strategies::CircularFullyFixed * circularFullyFixedStrategy
                        = static_cast<source_link_strategies::CircularFullyFixed *>(mLinkStrategy.get());
                    circularFullyFixedStrategy->setOrdering(tmpCircularFullyFixedOrdering);
                    circularFullyFixedStrategy->setOrderingInitialized();
                }
            }
        }
        mSnapshots.primary = primaryStart;
        mSources.getPrimarySource().setPosition(target.position, Source::OriginOfChange::link);
    }
}

//==============================================================================
void SourceLinkEnforcer::primaryAnchorMoved()
{
    // mLinkStrategy->computeParameters(mSources, mSnapshots);
    auto const newPrimarySnapshot{
        mLinkStrategy->computeInitialStateFromFinalState(mSources, mSnapshots, mSources.getPrimarySource().getIndex())
    };
    mSnapshots.primary = newPrimarySnapshot;
    enforceSourceLink();
}

//==============================================================================
void SourceLinkEnforcer::secondaryAnchorMoved(SourceIndex const sourceIndex)
{
    jassert(sourceIndex.get() > 0 && sourceIndex.get() < mSources.MAX_NUMBER_OF_SOURCES);

    auto const secondaryIndex{ sourceIndex.get() - 1 };
    auto & snapshot{ mSnapshots.secondaries.getReference(secondaryIndex) };
    mLinkStrategy->computeParameters(mSources, mSnapshots);
    auto const newSecondaryAnchor{
        mLinkStrategy->computeInitialStateFromFinalState(mSources, mSnapshots, sourceIndex)
    };
    snapshot = newSecondaryAnchor;
    primarySourceMoved(); // some positions are invalid - fix them right away
}

//==============================================================================
void SourceLinkEnforcer::saveCurrentPositionsToInitialStates()
{
    mSnapshots.primary = SourceSnapshot{ mSources.getPrimarySource() };
    mSnapshots.secondaries.clearQuick();
    for (unsigned i{}; i < mSources.getSecondarySources().size(); ++i) {
        mSnapshots.secondaries.add(SourceSnapshot{ mSources.getSecondarySources()[i] });
    }
}

} // namespace gris
