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

#include "cg_FieldComponent.hpp"

#include "cg_ElevationSourceComponent.hpp"
#include "cg_constants.hpp"

namespace gris
{
//==============================================================================
FieldComponent::FieldComponent(Sources & sources) noexcept : mSources(sources)
{
    setSize(MIN_FIELD_WIDTH, MIN_FIELD_WIDTH);
}

//==============================================================================
FieldComponent::~FieldComponent() noexcept
{
    for (SourceIndex i{}; i < SourceIndex{ mSources.MAX_NUMBER_OF_SOURCES }; ++i) {
        mSources[i].removeGuiListener(this);
    }
}

//==============================================================================
void FieldComponent::setSelectedSource(std::optional<SourceIndex> const selectedSource)
{
    mOldSelectedSource = mSelectedSource;
    mSelectedSource = selectedSource;

    applySourceSelectionToComponents();
}

//==============================================================================
void FieldComponent::refreshSources()
{
    mSelectedSource.reset();
    mOldSelectedSource.reset();

    for (auto & source : mSources) {
        source.addGuiListener(this);
    }

    rebuildSourceComponents(mSources.size());
}

//==============================================================================
void FieldComponent::drawBackgroundGrid(juce::Graphics & g) const
{
    auto const fieldComponentSize{ getWidth() };
    auto const fieldComponentSize_f{ static_cast<float>(fieldComponentSize) };

    auto * grisLookAndFeel{ dynamic_cast<GrisLookAndFeel *>(&getLookAndFeel()) };
    jassert(grisLookAndFeel != nullptr);

    if (grisLookAndFeel != nullptr) {
        // Draw the background.
        g.setColour(grisLookAndFeel->getFieldColor());
        g.fillRect(0, 0, fieldComponentSize, fieldComponentSize);
        g.setColour(juce::Colours::black);
        g.drawRect(0, 0, fieldComponentSize, fieldComponentSize);

        // Draw the grid.
        g.setColour(juce::Colour::fromRGB(55, 56, 57));
        constexpr int gridCount = 8;
        for (int i{ 1 }; i < gridCount; ++i) {
            g.drawLine(fieldComponentSize_f * i / gridCount,
                       0,
                       fieldComponentSize_f * i / gridCount,
                       fieldComponentSize_f);
            g.drawLine(0,
                       fieldComponentSize_f * i / gridCount,
                       fieldComponentSize_f,
                       fieldComponentSize_f * i / gridCount);
        }
        g.drawLine(0, 0, fieldComponentSize_f, fieldComponentSize_f);
        g.drawLine(0, fieldComponentSize_f, fieldComponentSize_f, 0);
    }
}

//==============================================================================
void FieldComponent::sourceMovedCallback()
{
    repaint();
}

//==============================================================================
void FieldComponent::paint(juce::Graphics & g)
{
    Component::paint(g);
    drawBackground(g);
    drawSpans(g);
    if (mDisplayInvalidSourceMoveWarning) {
        static constexpr auto padding{ 25 };
        static constexpr auto fontSize{ 16 };
        juce::Rectangle<int> const textArea{ padding, padding, getWidth() - padding * 2, getHeight() - padding * 2 };
        juce::Font const font{ juce::FontOptions(fontSize, juce::Font::plain) };
        g.setFont(font);
        g.setColour(juce::Colours::antiquewhite);
        g.drawFittedText(SOURCE_SELECTION_WARNING, textArea, juce::Justification::centredTop, 2);
    }
}

//==============================================================================
void FieldComponent::displayInvalidSourceMoveWarning(bool const state)
{
    mDisplayInvalidSourceMoveWarning = state;
    repaint();
}

//==============================================================================
void PositionFieldComponent::applySourceSelectionToComponents()
{
    if (mSelectedSource.has_value()) {
        for (auto component : mSourceComponents) {
            bool const selected{ *mSelectedSource == component->getSourceIndex() };
            component->setSelected(selected);
        }
    } else {
        for (auto component : mSourceComponents) {
            component->setSelected(false);
        }
    }
}

//==============================================================================
void ElevationFieldComponent::applySourceSelectionToComponents()
{
    // TODO: this is a dupe of PositionFieldComponent::applySourceSelectionToComponents()
    if (mSelectedSource.has_value()) {
        for (auto component : mSourceComponents) {
            bool const selected{ mSelectedSource == component->getSourceIndex() };
            component->setSelected(selected);
        }
    } else {
        for (auto component : mSourceComponents) {
            component->setSelected(false);
        }
    }
}

//==============================================================================
void PositionFieldComponent::drawBackground(juce::Graphics & g) const
{
    auto const fieldComponentSize{ getWidth() };
    auto const fieldComponentSize_f{ static_cast<float>(fieldComponentSize) };
    auto * grisLookAndFeel{ dynamic_cast<GrisLookAndFeel *>(&getLookAndFeel()) };
    jassert(grisLookAndFeel != nullptr);
    auto const fieldCenter{ fieldComponentSize / 2.0f };

    drawBackgroundGrid(g);

    g.setColour(grisLookAndFeel->getLightColor());
    if (mSpatMode == SpatMode::dome) {
        // Draw big background circles.
        for (int i{ 1 }; i < 3; ++i) {
            float const w{ i / 2.0f * (fieldComponentSize - SOURCE_FIELD_COMPONENT_DIAMETER) };
            float const x{ (fieldComponentSize - w) / 2.0f };
            g.drawEllipse(x, x, w, w, 1);
        }

        // Draw center dot.
        float const w{ 0.0125f * (fieldComponentSize - SOURCE_FIELD_COMPONENT_DIAMETER) };
        float const x{ (fieldComponentSize - w) / 2.0f };
        g.drawEllipse(x, x, w, w, 1);
    } else {
        // Draw big background squares.
        static constexpr auto BASE_OFFSET{ SOURCE_FIELD_COMPONENT_RADIUS };
        auto const usableLength{ fieldComponentSize - SOURCE_FIELD_COMPONENT_DIAMETER };

        auto const getCenteredRect = [usableLength](float const size) -> juce::Rectangle<float> {
            jassert(usableLength >= size);
            auto const offset{ BASE_OFFSET + (usableLength - size) / 2.0f };
            juce::Rectangle<float> const result{ offset, offset, size, size };
            return result;
        };

        auto const noAttenuationRect{ getCenteredRect(usableLength / LBAP_FAR_FIELD) };
        auto const maxAttenuationRect{ getCenteredRect(usableLength) };
        g.setColour(juce::Colour::fromRGB(55, 56, 57));
        g.drawEllipse(noAttenuationRect, 1.0f);
        g.drawEllipse(maxAttenuationRect, 1.0f);
        g.setColour(grisLookAndFeel->getLightColor());
        g.drawRect(noAttenuationRect);
        g.drawRect(maxAttenuationRect);
    }

    // Draw cross.
    g.setColour(grisLookAndFeel->getLightColor());
    g.drawLine(fieldCenter,
               SOURCE_FIELD_COMPONENT_RADIUS,
               fieldCenter,
               fieldComponentSize - SOURCE_FIELD_COMPONENT_RADIUS);
    g.drawLine(SOURCE_FIELD_COMPONENT_RADIUS,
               fieldComponentSize_f / 2.0f,
               fieldComponentSize - SOURCE_FIELD_COMPONENT_RADIUS,
               fieldComponentSize_f / 2.0f);
}

//==============================================================================
void ElevationFieldComponent::drawBackground(juce::Graphics & g) const
{
    auto const fieldComponentSize{ getWidth() };
    auto const fieldComponentSize_f{ static_cast<float>(fieldComponentSize) };
    auto * grisLookAndFeel{ dynamic_cast<GrisLookAndFeel *>(&getLookAndFeel()) };
    jassert(grisLookAndFeel != nullptr);

    if (grisLookAndFeel != nullptr) {
        drawBackgroundGrid(g);

        g.setColour(grisLookAndFeel->getLightColor());
        g.drawVerticalLine(5, 5, fieldComponentSize_f - 5);
        g.drawHorizontalLine(fieldComponentSize - 5, 5, fieldComponentSize_f - 5);

        // Draw horizontal lines.
        if (mElevationMode == ElevationMode::extendedTop) {
            auto const lbapAttenuationHeight{ (fieldComponentSize_f * (LBAP_FAR_FIELD - 1.0f) / LBAP_FAR_FIELD) };
            auto const domeTopLimit{ lbapAttenuationHeight + SOURCE_FIELD_COMPONENT_RADIUS / 2.0f };

            g.setColour(grisLookAndFeel->getLightColor());
            g.drawHorizontalLine(static_cast<int>(domeTopLimit), 10.0f, fieldComponentSize_f - 10.0f);
        } else if (mElevationMode == ElevationMode::extendedTopAndBottom) {
            auto const lbapAttenuationHeight{ (fieldComponentSize_f * (LBAP_FAR_FIELD - 1.0f)
                                               / (LBAP_FAR_FIELD + LBAP_FAR_FIELD - 1.0f))
                                              + SOURCE_FIELD_COMPONENT_RADIUS };
            auto const domeTopLimit{ lbapAttenuationHeight };
            auto const domeBottomLimit{ fieldComponentSize_f - lbapAttenuationHeight };

            g.setColour(grisLookAndFeel->getLightColor());
            g.drawHorizontalLine(static_cast<int>(domeTopLimit), 10.0f, fieldComponentSize_f - 10.0f);
            g.drawHorizontalLine(static_cast<int>(domeBottomLimit), 10.0f, fieldComponentSize_f - 10.0f);
        }
    }
}

//==============================================================================
juce::Point<float>
    PositionFieldComponent::sourcePositionToComponentPosition(juce::Point<float> const & sourcePosition) const
{
    auto const effectiveArea{ getEffectiveArea() };
    auto const normalizedPosition{ (sourcePosition + juce::Point<float>{ 1.0f, 1.0f }) / 2.0f };
    auto const result{ normalizedPosition * effectiveArea.getWidth() + effectiveArea.getPosition() };
    return result;
}

//==============================================================================
juce::Line<float>
    PositionFieldComponent::sourcePositionToComponentPosition(juce::Line<float> const & sourcePosition) const
{
    juce::Line<float> result{ sourcePositionToComponentPosition(sourcePosition.getStart()),
                              sourcePositionToComponentPosition(sourcePosition.getEnd()) };
    return result;
}

//==============================================================================
juce::Point<float>
    PositionFieldComponent::componentPositionToSourcePosition(juce::Point<float> const & componentPosition) const
{
    auto const effectiveArea{ getEffectiveArea() };
    auto const normalizedPosition{ (componentPosition - effectiveArea.getPosition()) / effectiveArea.getWidth() };
    auto const result{ normalizedPosition * 2.0f - juce::Point<float>{ 1.0f, 1.0f } };
    return result;
}

//==============================================================================
juce::Rectangle<float> PositionFieldComponent::getEffectiveArea() const
{
    jassert(getWidth() == getHeight());

    auto const componentSize{ static_cast<float>(getWidth()) };
    auto const effectiveSize{ componentSize - SOURCE_FIELD_COMPONENT_RADIUS * 2.0f };

    juce::Rectangle<float> const result{ SOURCE_FIELD_COMPONENT_RADIUS,
                                         SOURCE_FIELD_COMPONENT_RADIUS,
                                         effectiveSize,
                                         effectiveSize };
    return result;
}

//==============================================================================
void PositionFieldComponent::notifySourcePositionChanged(SourceIndex const sourceIndex)
{
    mListeners.call([&](Listener & l) { l.fieldSourcePositionChangedCallback(sourceIndex, 0); });
}

//==============================================================================
void PositionFieldComponent::setShowTrajectory(bool shouldShowTrajectory)
{
    mShowTrajectory = shouldShowTrajectory;
}

//==============================================================================
void PositionFieldComponent::rebuildSourceComponents(int const numberOfSources)
{
    mSourceComponents.clearQuick(true);
    for (int i{ numberOfSources - 1 }; i >= 0; --i) {
        auto & source{ mSources[i] };
        mSourceComponents.add(new PositionSourceComponent{ *this, source, mStorage });
        addAndMakeVisible(mSourceComponents.getLast());
    }
}

//==============================================================================
PositionFieldComponent::PositionFieldComponent(Sources & sources,
                                               PositionTrajectoryManager & positionAutomationManager,
                                               PersistentStorage & storage) noexcept
    : FieldComponent(sources)
    , mAutomationManager(positionAutomationManager)
    , mStorage(storage)
{
    mDrawingHandleComponent.setInterceptsMouseClicks(false, false);
    mDrawingHandleComponent.setCentrePosition(getWidth() / 2, getHeight() / 2);
    mDrawingHandleComponent.setSelected(true);
    addAndMakeVisible(mDrawingHandleComponent);
}

//==============================================================================
void PositionFieldComponent::setSpatMode(SpatMode const spatMode)
{
    mSpatMode = spatMode;
    repaint();
}

//==============================================================================
void PositionFieldComponent::drawDomeSpans(juce::Graphics & g) const
{
    auto const width{ getWidth() };
    auto const halfWidth{ static_cast<float>(width) / 2.0f };
    juce::Point<float> const fieldCenter{ halfWidth, halfWidth };
    auto const magnitude{ (width - SOURCE_FIELD_COMPONENT_DIAMETER) / 2.0f };

    for (auto const & source : mSources) {
        auto const azimuth{ source.getAzimuth() };
        auto const elevation{ source.getNormalizedElevation() };
        auto const azimuthSpan{ (Degrees{ 180.0f } * source.getAzimuthSpan().get()).getAsRadians() };
        auto const elevationSpan{ source.getElevationSpan() };

        // Calculate min and max elevation in degrees.
        juce::Range<float> const elevationLimits{ 0.0f, 1.0f };
        juce::Range<float> const elevationRange{ (elevation - elevationSpan).get(), (elevation + elevationSpan).get() };
        auto const clippedElevationRange{ elevationRange.getIntersectionWith(elevationLimits) };

        juce::Point<float> const lower_corner_a{ std::cos(azimuthSpan) * clippedElevationRange.getStart(),
                                                 std::sin(azimuthSpan) * clippedElevationRange.getStart() };
        juce::Point<float> const upper_corner_b{ std::cos(-azimuthSpan) * clippedElevationRange.getEnd(),
                                                 std::sin(-azimuthSpan) * clippedElevationRange.getEnd() };

        // Draw the path
        juce::Path path{};
        path.startNewSubPath(lower_corner_a);
        path.addCentredArc(
            0.0f,
            0.0f,
            clippedElevationRange.getStart(),
            clippedElevationRange.getStart(),
            0.0f,
            azimuthSpan
                + juce::MathConstants<float>::halfPi, // addCentredArc counts radians from the top-center of the ellipse
            -azimuthSpan + juce::MathConstants<float>::halfPi);
        path.lineTo(upper_corner_b); // lower right corner
        path.addCentredArc(0.0f,
                           0.0f,
                           clippedElevationRange.getEnd(),
                           clippedElevationRange.getEnd(),
                           0.0f,
                           -azimuthSpan + juce::MathConstants<float>::halfPi,
                           azimuthSpan + juce::MathConstants<float>::halfPi); // upper right corner
        path.closeSubPath();

        // rotate, scale and translate path
        auto const rotation{ azimuth - Radians { Degrees{ 90.0f }} }; // correction for the way addCentredArc counts angles
        auto const transform{
            juce::AffineTransform::rotation(rotation.getAsRadians()).scaled(magnitude).translated(fieldCenter)
        };
        path.applyTransform(transform);

        // draw
        g.setColour(source.getColour().withAlpha(0.1f));
        g.fillPath(path);
        g.setColour(source.getColour().withAlpha(0.5f));
        juce::PathStrokeType strokeType = juce::PathStrokeType(1.5);
        g.strokePath(path, strokeType);
    }
}

//==============================================================================
void PositionFieldComponent::drawCubeSpans(juce::Graphics & g) const
{
    constexpr float MIN_SPAN_WIDTH = SOURCE_FIELD_COMPONENT_DIAMETER;
    constexpr float MAGIC_MAX_SPAN_RATIO
        = 0.8f; // TODO : there is probably some reasonning behind this value in SpatGRIS2 source.

    auto const effectiveWidth{ getEffectiveArea().getWidth() };

    for (auto const & source : mSources) {
        float const azimuthSpan{ effectiveWidth * source.getAzimuthSpan().get() * MAGIC_MAX_SPAN_RATIO
                                 + MIN_SPAN_WIDTH };
        float const halfAzimuthSpan{ azimuthSpan / 2.0f };
        float const saturation{ (mSelectedSource == source.getIndex()) ? 1.0f : 0.5f };
        juce::Point<float> const center{ sourcePositionToComponentPosition(source.getPos()) };
        juce::Rectangle<float> const area{ center.getX() - halfAzimuthSpan,
                                           center.getY() - halfAzimuthSpan,
                                           azimuthSpan,
                                           azimuthSpan };

        g.setColour(source.getColour().withSaturation(saturation).withAlpha(0.5f));
        g.drawEllipse(area, 1.5f);
        g.setColour(source.getColour().withSaturation(saturation).withAlpha(0.1f));
        g.fillEllipse(area);
    }
}

//==============================================================================
void PositionFieldComponent::paint(juce::Graphics & g)
{
    FieldComponent::paint(g);

    mDrawingHandleComponent.setVisible(mAutomationManager.getTrajectoryType() == PositionTrajectoryType::drawing
                                       && !mIsPlaying && mShowTrajectory);

    if (mShowTrajectory) {
        // Draw recording trajectory path and current position dot.
        g.setColour(juce::Colour::fromRGB(176, 176, 228));
        if (mLineDrawingStartPosition.has_value() && mLineDrawingEndPosition.has_value()) {
            juce::Line<float> lineInSourceSpace{ *mLineDrawingStartPosition, *mLineDrawingEndPosition };
            auto const lineInComponentSpace{ sourcePositionToComponentPosition(lineInSourceSpace) };
            g.drawLine(lineInComponentSpace, 0.75f);
        }
        if (mAutomationManager.getTrajectory().has_value()) {
            auto const trajectoryPath{ mAutomationManager.getTrajectory()->getDrawablePath(getEffectiveArea(),
                                                                                           mSpatMode) };
            g.strokePath(trajectoryPath, juce::PathStrokeType(.75f));
        }
        // position dot
        if (mIsPlaying && !isMouseButtonDown()
            && mAutomationManager.getTrajectoryType() != PositionTrajectoryType::realtime
            && mAutomationManager.getPositionActivateState()) {
            constexpr float radius = 4.0f;
            constexpr float diameter = radius * 2.0f;
            juce::Point<float> const dotCenter{ sourcePositionToComponentPosition(
                mAutomationManager.getCurrentTrajectoryPoint()) };
            g.fillEllipse(dotCenter.getX() - radius, dotCenter.getY() - radius, diameter, diameter);
        }
    }
}

//==============================================================================
void PositionFieldComponent::resized()
{
    FieldComponent::resized();
    for (auto * sourceComponent : mSourceComponents) {
        sourceComponent->updatePositionInParent();
    }
    mDrawingHandleComponent.updatePositionInParent();
}

//==============================================================================
void PositionFieldComponent::drawSpans(juce::Graphics & g) const
{
    if (mSpatMode == SpatMode::dome) {
        drawDomeSpans(g);
    } else {
        drawCubeSpans(g);
    }
}

//==============================================================================
void PositionFieldComponent::mouseDown(juce::MouseEvent const & event)
{
    setSelectedSource(std::nullopt);

    if (mAutomationManager.getTrajectoryType() == PositionTrajectoryType::drawing && mShowTrajectory) {
        auto const mousePosition{ event.getPosition().toFloat() };
        auto const unclippedPosition{ componentPositionToSourcePosition(mousePosition) };
        auto const position{ Source::clipPosition(unclippedPosition, mSpatMode) };

        mOldSelectedSource.reset();
        mDrawingHandleComponent.setCentrePosition(sourcePositionToComponentPosition(position).toInt());

        auto const isShiftDown{ event.mods.isShiftDown() };
        auto const isCurrentlyDrawingStraightLine{ mLineDrawingStartPosition.has_value() };

        if (isCurrentlyDrawingStraightLine) {
            // bake current line into drawing
            mLineDrawingEndPosition = position;
            juce::Line<float> const lineInSourceSpace{ *mLineDrawingStartPosition, *mLineDrawingEndPosition };
            auto const lineInComponentSpace{ sourcePositionToComponentPosition(lineInSourceSpace) };
            auto const nbPoints{ static_cast<int>(std::round(lineInComponentSpace.getLength())) };
            for (int i{}; i < nbPoints; ++i) {
                auto const progression{ static_cast<float>(i) / (static_cast<float>(nbPoints - 1)) };
                auto const newPoint{ lineInSourceSpace.getPointAlongLineProportionally(progression) };
                mAutomationManager.addRecordingPoint(newPoint);
            }
            if (isShiftDown) {
                // start a new line from the old one
                mLineDrawingStartPosition = mLineDrawingEndPosition;
            } else {
                // finish gesture
                mLineDrawingStartPosition.reset();
            }
            mLineDrawingEndPosition.reset();
        } else {
            // not currently drawing a straight line
            mAutomationManager.resetRecordingTrajectory(position);
            mSources.getPrimarySource().setPosition(position, Source::OriginOfChange::userMove);
            if (isShiftDown) {
                // start a new Line
                mLineDrawingStartPosition = position;
            }
        }
    }

    repaint();
}

//==============================================================================
void PositionFieldComponent::mouseDrag(const juce::MouseEvent & event)
{
    if (mAutomationManager.getTrajectoryType() == PositionTrajectoryType::drawing && mShowTrajectory) {
        auto const mousePosition{ event.getPosition() };
        auto const positionAsSource_unclipped{ componentPositionToSourcePosition(mousePosition.toFloat()) };
        auto const positionAsSource{ Source::clipPosition(positionAsSource_unclipped, mSpatMode) };
        auto const positionAsComponent{ sourcePositionToComponentPosition(positionAsSource).roundToInt() };
        mDrawingHandleComponent.setCentrePosition(positionAsComponent.getX(), positionAsComponent.getY());

        if (mLineDrawingStartPosition.has_value()) {
            mLineDrawingEndPosition = positionAsSource;
        } else {
            mAutomationManager.addRecordingPoint(positionAsSource);
        }
        repaint();
    }
}

//==============================================================================
void PositionFieldComponent::mouseUp(const juce::MouseEvent & event)
{
    mouseDrag(event);
}

//==============================================================================
void PositionFieldComponent::mouseMove(juce::MouseEvent const & event)
{
    if (mAutomationManager.getTrajectoryType() == PositionTrajectoryType::drawing
        && mLineDrawingStartPosition.has_value()) {
        auto const mousePosition{ event.getPosition() };
        mDrawingHandleComponent.setCentrePosition(mousePosition.getX(), mousePosition.getY());
        auto const unclippedPosition{ componentPositionToSourcePosition(mousePosition.toFloat()) };
        auto const position{ Source::clipPosition(unclippedPosition, mSpatMode) };

        mLineDrawingEndPosition = position;
    }
}

//==============================================================================
void ElevationFieldComponent::drawSpans(juce::Graphics & g) const
{
    auto drawAnchor = [](juce::Graphics & graph,
                         juce::Point<float> const & position,
                         juce::Colour const colour,
                         float const saturation,
                         float const componentSize) {
        constexpr auto anchorThickness = 5;
        juce::Line<float> anchor{ position, position.translated(0, componentSize) };
        graph.setColour(colour.withSaturation(saturation).withAlpha(0.5f));
        graph.drawLine(anchor, anchorThickness);
    };

    jassert(getWidth() == getHeight());
    auto const componentSize{ static_cast<float>(getWidth()) };
    auto const effectiveArea{ getEffectiveArea() };

    float sourceIndex{};
    for (auto const & source : mSources) {
        auto const lineThickness{ (mSelectedSource == source.getIndex()) ? 2 : 1 };
        auto const saturation{ (mSelectedSource == source.getIndex()) ? 1.0f : 0.75f };
        auto const position{ sourceElevationToComponentPosition(source.getElevation(), source.getIndex()) };

        auto const halfSpanHeight{ source.getElevationSpan().get() * effectiveArea.getHeight() };
        auto const spanHeight{ halfSpanHeight * 2.0f };

        juce::Rectangle<float> spanArea{ position.getX() - SOURCE_FIELD_COMPONENT_RADIUS,
                                         position.getY() - halfSpanHeight,
                                         SOURCE_FIELD_COMPONENT_DIAMETER,
                                         spanHeight };

        drawAnchor(g, position, source.getColour(), saturation, componentSize);
        // draw Spans
        g.setColour(source.getColour().withSaturation(saturation).withAlpha(0.3f));
        juce::Graphics::ScopedSaveState graphicsState{ g };
        g.reduceClipRegion(effectiveArea.toNearestInt()); // TODO: this clips everything after!
        g.drawEllipse(spanArea, static_cast<float>(lineThickness));
        g.fillEllipse(spanArea);
        sourceIndex += 1.0f;
    }

    if (mAutomationManager.getTrajectoryType() == ElevationTrajectoryType::drawing && mShowTrajectory) {
        auto const handlePosition{ mDrawingHandle.getBounds().getCentre().toFloat() };
        drawAnchor(g, handlePosition, mDrawingHandle.getColour(), 0.75f, componentSize);
    }
}

//==============================================================================
ElevationFieldComponent::ElevationFieldComponent(Sources & sources,
                                                 ElevationTrajectoryManager & automationManager) noexcept
    : FieldComponent(sources)
    , mAutomationManager(automationManager)
{
    mDrawingHandle.setCentrePosition(sourceElevationToComponentPosition(Radians{ 0.0f }, SourceIndex{ -1 }).toInt());
    addAndMakeVisible(mDrawingHandle);
}

//==============================================================================
void ElevationFieldComponent::paint(juce::Graphics & g)
{
    FieldComponent::paint(g);

    auto const trajectoryType{ mAutomationManager.getTrajectoryType() };
    auto const effectiveArea{ getEffectiveArea() };
    mDrawingHandle.setVisible(trajectoryType == ElevationTrajectoryType::drawing && !mIsPlaying && mShowTrajectory);

    if (mShowTrajectory) {
        // Draw recording trajectory path and current position dot.
        g.setColour(juce::Colour::fromRGB(176, 176, 228));
        if (mAutomationManager.getTrajectory().has_value()) {
            auto const trajectoryPath{ mAutomationManager.getTrajectory()->getDrawablePath(
                effectiveArea,
                mSources.getPrimarySource().getSpatMode()) };
            g.strokePath(trajectoryPath, juce::PathStrokeType{ .75f });
        }
        if (mIsPlaying && !isMouseButtonDown()
            && static_cast<ElevationTrajectoryType>(mAutomationManager.getTrajectoryType())
                   != ElevationTrajectoryType::realtime
            && mAutomationManager.getPositionActivateState()) {
            auto const currentTrajectoryPosition{ mAutomationManager.getCurrentTrajectoryPoint() };
            auto const normalizedCurrentTrajectoryPosition{
                (currentTrajectoryPosition + juce::Point<float>{ 1.0f, 1.0f }) / 2.0f
            };
            auto const positionDot{ normalizedCurrentTrajectoryPosition * effectiveArea.getWidth()
                                    + effectiveArea.getPosition() };
            g.fillEllipse(positionDot.getX() - 4, positionDot.getY() - 4, 8, 8);
        }
    }
}

//==============================================================================
void ElevationFieldComponent::resized()
{
    FieldComponent::resized();
    for (auto * sourceComponent : mSourceComponents) {
        sourceComponent->updatePositionInParent();
    }
    mDrawingHandle.updatePositionInParent();
}

//==============================================================================
void ElevationFieldComponent::mouseDown([[maybe_unused]] juce::MouseEvent const & event)
{
    mSelectedSource.reset();
    mOldSelectedSource.reset();
    setSelectedSource(std::nullopt);

    if (mAutomationManager.getTrajectoryType() == ElevationTrajectoryType::drawing && mShowTrajectory) {
        mDrawingHandle.mouseDown(event);
    }

    repaint();
}

//==============================================================================
void ElevationFieldComponent::notifySourcePositionChanged(SourceIndex const sourceIndex)
{
    mListeners.call([&](Listener & l) { l.fieldSourcePositionChangedCallback(sourceIndex, 1); });
}

//==============================================================================
void ElevationFieldComponent::mouseDrag([[maybe_unused]] const juce::MouseEvent & event)
{
    if (mAutomationManager.getTrajectoryType() == ElevationTrajectoryType::drawing && mShowTrajectory) {
        mDrawingHandle.mouseDrag(event);
    }
}

//==============================================================================
void ElevationFieldComponent::mouseUp([[maybe_unused]] const juce::MouseEvent & event)
{
    if (mAutomationManager.getTrajectoryType() == ElevationTrajectoryType::drawing && mShowTrajectory) {
        mDrawingHandle.mouseDrag(event);
    }
}

//==============================================================================
void ElevationFieldComponent::rebuildSourceComponents([[maybe_unused]] int const numberOfSources)
{
    mSourceComponents.clearQuick(true);
    for (auto & source : mSources) {
        mSourceComponents.add(new ElevationSourceComponent{ *this, source });
        addAndMakeVisible(mSourceComponents.getLast());
    }
}

//==============================================================================
juce::Point<float> ElevationFieldComponent::sourceElevationToComponentPosition(Radians const sourceElevation,
                                                                               SourceIndex const index) const
{
    auto const availableWidth{ static_cast<float>(getWidth()) - LEFT_PADDING - RIGHT_PADDING };
    auto const availableHeight{ static_cast<float>(getHeight()) - TOP_PADDING - BOTTOM_PADDING };
    auto const widthBetweenEachSource{ availableWidth / static_cast<float>(mSources.size() + 1) };

    auto const x{
        LEFT_PADDING + widthBetweenEachSource * (static_cast<float>(index.get() + 1))
    }; // We add +1 to the index for the drawing handle.
    auto const clippedElevation{ sourceElevation.clamped(Radians {MIN_ELEVATION}, Radians {MAX_ELEVATION}) };
    jassert(!std::isnan(clippedElevation.getAsRadians()));
    auto const y{ clippedElevation / Radians{ MAX_ELEVATION } * availableHeight + TOP_PADDING };
    juce::Point<float> const result{ x, y };

    jassert(x > 0 && x < getWidth());
    jassert(y > 0 && y < getHeight());

    return result;
}

//==============================================================================
Radians ElevationFieldComponent::componentPositionToSourceElevation(juce::Point<float> const & componentPosition) const
{
    auto const effectiveHeight{ static_cast<float>(getHeight()) - TOP_PADDING - BOTTOM_PADDING };

    Radians const elevation{ MAX_ELEVATION * ((componentPosition.getY() - TOP_PADDING) / effectiveHeight) };
    auto const result{ elevation.clamped(Radians {MIN_ELEVATION}, Radians {MAX_ELEVATION}) };

    return result;
}

//==============================================================================
void ElevationFieldComponent::setElevationMode(ElevationMode const & elevationMode)
{
    mElevationMode = elevationMode;
    repaint();
}

//==============================================================================
juce::Rectangle<float> ElevationFieldComponent::getEffectiveArea() const
{
    jassert(getWidth() == getHeight());

    auto const componentSize{ static_cast<float>(getWidth()) };
    auto const effectiveWidth{ componentSize - LEFT_PADDING - RIGHT_PADDING };
    auto const effectiveHeight{ componentSize - TOP_PADDING - BOTTOM_PADDING };

    juce::Rectangle<float> const result{ LEFT_PADDING, TOP_PADDING, effectiveWidth, effectiveHeight };
    return result;
}

//==============================================================================
void ElevationFieldComponent::setShowTrajectory(bool shouldShowTrajectory)
{
    mShowTrajectory = shouldShowTrajectory;
}

} // namespace gris
