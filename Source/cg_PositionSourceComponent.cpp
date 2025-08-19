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

#include "cg_PositionSourceComponent.hpp"

#include "cg_ControlGrisAudioProcessor.hpp"
#include "cg_FieldComponent.hpp"
#include "cg_Source.hpp"

namespace gris
{
//==============================================================================
PositionSourceComponent::PositionSourceComponent(PositionFieldComponent & fieldComponent,
                                                 Source & source,
                                                 PersistentStorage & storage)
    : SourceComponent(source.getColour(), source.getId().toString())
    , mFieldComponent(fieldComponent)
    , mTrajectoryManager(fieldComponent.getAutomationManager())
    , mSource(source)
    , mStorage(storage)
{
    source.addGuiListener(this);
    updatePositionInParent();
}

//==============================================================================
PositionSourceComponent::~PositionSourceComponent()
{
    mSource.removeGuiListener(this);
}

//==============================================================================
void PositionSourceComponent::updatePositionInParent()
{
    auto const newCenter{ mFieldComponent.sourcePositionToComponentPosition(mSource.getPos()).roundToInt() };
    setCentrePosition(newCenter.getX(), newCenter.getY());
}

//==============================================================================
void PositionSourceComponent::sourceMovedCallback()
{
    updatePositionInParent();
}

//==============================================================================
void PositionSourceComponent::mouseDown(juce::MouseEvent const & event)
{
    static auto tempHideError{ !mStorage.getShowSecondarySourceDragErrorMessage() };

    auto const isPrimarySource{ mSource.isPrimarySource() };
    if (mFieldComponent.isPlaying() && !isPrimarySource && !tempHideError) {
        mCanDrag = false;
        juce::String const error{ "You are moving a secondary source during playback : please note that secondary "
                                  "sources cannot be automated in ControlGRIS. Automation on more than one source can "
                                  "be done using source links or with multiple ControlGRIS instances." };
        juce::AlertWindow alert{ "Warning", error, juce::AlertWindow::InfoIcon, this };
        enum class Button { closed = -1, ok = 0, hideForNow, hideForever };
        alert.addButton("Ok", static_cast<int>(Button::ok));
        alert.addButton("Hide for now", static_cast<int>(Button::hideForNow));
        alert.addButton("Hide forever", static_cast<int>(Button::hideForever));
        alert.setDropShadowEnabled(true);
        auto const userAction{ static_cast<Button>(alert.runModalLoop()) };
        switch (userAction) {
        case Button::hideForever:
            tempHideError = true;
            mStorage.setShowSecondarySourceDragErrorMessage(false);
            break;
        case Button::hideForNow:
            tempHideError = true;
            break;
        case Button::closed:
        case Button::ok:
            break;
        default:
            jassertfalse;
        }
        return;
    }

    mDisplacementMode = getDisplacementMode(event);
    mCanDrag = isMoveAllowed(mDisplacementMode, isPrimarySource, mTrajectoryManager.getSourceLink());
    if (mCanDrag) {
        if (isPrimarySource || mDisplacementMode == DisplacementMode::all) {
            mTrajectoryManager.getProcessor().getChangeGestureManager().beginGesture(Automation::Ids::X);
            mTrajectoryManager.getProcessor().getChangeGestureManager().beginGesture(Automation::Ids::Y);
        }
        setSourcePosition(event);
    } else {
        mFieldComponent.displayInvalidSourceMoveWarning(true);
    }
}

//==============================================================================
void PositionSourceComponent::setSourcePosition(juce::MouseEvent const & event) const
{
    jassert(mFieldComponent.getWidth() == mFieldComponent.getHeight());

    auto const origin{ mDisplacementMode == DisplacementMode::selectedSourceOnly
                           ? Source::OriginOfChange::userAnchorMove
                           : Source::OriginOfChange::userMove };
    auto const eventRelativeToFieldComponent{ event.getEventRelativeTo(&mFieldComponent) };
    auto const newPosition{ mFieldComponent.componentPositionToSourcePosition(
        eventRelativeToFieldComponent.getPosition().toFloat()) };
    mSource.setPosition(newPosition, origin);
}

//==============================================================================
void PositionSourceComponent::mouseDrag(juce::MouseEvent const & event)
{
    if (mCanDrag && mFieldComponent.getSelectedSourceIndex() == mSource.getIndex()) {
        jassert(mFieldComponent.getWidth() == mFieldComponent.getHeight());
        setSourcePosition(event);
    }
}

//==============================================================================
void PositionSourceComponent::mouseUp(juce::MouseEvent const & event)
{
    if (mCanDrag) {
        mouseDrag(event);
        if (mSource.isPrimarySource() || mDisplacementMode == DisplacementMode::all) {
            mTrajectoryManager.getProcessor().getChangeGestureManager().endGesture(Automation::Ids::X);
            mTrajectoryManager.getProcessor().getChangeGestureManager().endGesture(Automation::Ids::Y);
        }
    } else {
        mFieldComponent.displayInvalidSourceMoveWarning(false);
    }
}

//==============================================================================
SourceIndex PositionSourceComponent::getSourceIndex() const
{
    return mSource.getIndex();
}

} // namespace gris
