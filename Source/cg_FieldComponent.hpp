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

#include <type_traits>

#include <JuceHeader.h>

#include "cg_ElevationDrawingHandle.hpp"
#include "cg_ElevationSourceComponent.hpp"
#include "cg_PositionSourceComponent.hpp"
#include "cg_SectionGeneralSettings.hpp"
#include "cg_Source.hpp"
#include "cg_TrajectoryManager.hpp"

namespace gris
{
//==============================================================================
// This file defines the classes that implement the 2D view (azimuth-elevation
// or azimuth-distance) and the elevation view.
//
// Classes:
//   FieldComponent : The base class. Implements the common attributes and
//                    methods of the two views.
//   PositionFieldComponent : The 2D view as a cartesian plane. It is used to show
//                        and control two parameters, azimuth-elevation for the
//                        VBAP algorithm and azimuth-distance for the LBAP
//                        algorithm.
//   ElevationFieldComponent : The 1D view used to show and control the elevation
//                             parameter for the LBAP algorithm.
//==============================================================================

//==============================================================================
class FieldComponent
    : public juce::Component
    , public Source::Listener
{
public:
    //==============================================================================
    class Listener
    {
    public:
        Listener() noexcept = default;
        virtual ~Listener() noexcept = default;

        Listener(Listener const &) = delete;
        Listener(Listener &&) = delete;

        Listener & operator=(Listener const &) = delete;
        Listener & operator=(Listener &&) = delete;
        //==============================================================================
        virtual void fieldSourcePositionChangedCallback(SourceIndex sourceId, int whichField) = 0;

    private:
        //==============================================================================
        JUCE_LEAK_DETECTOR(Listener)

    }; // class FieldComponent::Listener

    //==============================================================================
    juce::ListenerList<Listener> mListeners{};

protected:
    //==============================================================================
    Sources & mSources;
    bool mIsPlaying{ false };
    std::optional<SourceIndex> mSelectedSource{};
    std::optional<SourceIndex> mOldSelectedSource{};
    bool mDisplayInvalidSourceMoveWarning{};

public:
    //==============================================================================
    FieldComponent() = delete;
    ~FieldComponent() noexcept override;

    FieldComponent(FieldComponent const &) = delete;
    FieldComponent(FieldComponent &&) = delete;

    FieldComponent & operator=(FieldComponent const &) = delete;
    FieldComponent & operator=(FieldComponent &&) = delete;
    //==============================================================================
    explicit FieldComponent(Sources & sources) noexcept;
    //==============================================================================
    void refreshSources();
    void setSelectedSource(std::optional<SourceIndex> selectedSource);
    [[nodiscard]] auto const & getSelectedSourceIndex() const { return mSelectedSource; }
    void setIsPlaying(bool const state) { mIsPlaying = state; }
    [[nodiscard]] bool isPlaying() const { return mIsPlaying; }
    void addListener(Listener * l) { mListeners.add(l); }
    void displayInvalidSourceMoveWarning(bool state);

    void paint(juce::Graphics & g) override;

    [[nodiscard]] Sources & getSources() { return mSources; }
    [[nodiscard]] Sources const & getSources() const { return mSources; }
    //==============================================================================
    [[nodiscard]] virtual juce::Rectangle<float> getEffectiveArea() const = 0;

protected:
    //==============================================================================
    void drawBackgroundGrid(juce::Graphics & g) const;
    //==============================================================================
    void sourceMovedCallback() override;
    //==============================================================================
    virtual void drawBackground(juce::Graphics & g) const = 0;
    virtual void applySourceSelectionToComponents() = 0;
    virtual void notifySourcePositionChanged(SourceIndex sourceIndex) = 0;
    virtual void rebuildSourceComponents(int numberOfSources) = 0;
    virtual void drawSpans(juce::Graphics & g) const = 0;

private:
    //==============================================================================
    JUCE_LEAK_DETECTOR(FieldComponent)

}; // class FieldComponent

//==============================================================================
class PositionFieldComponent final : public FieldComponent
{
    PositionTrajectoryManager & mAutomationManager;
    SpatMode mSpatMode{ SpatMode::dome };
    std::optional<juce::Point<float>> mLineDrawingStartPosition{ std::nullopt };
    std::optional<juce::Point<float>> mLineDrawingEndPosition{ std::nullopt };
    SourceComponent mDrawingHandleComponent{ juce::Colour::fromRGB(176, 176, 228), "X" };
    PersistentStorage & mStorage;
    juce::OwnedArray<PositionSourceComponent> mSourceComponents{};

public:
    //==============================================================================
    PositionFieldComponent() = delete;
    PositionFieldComponent(Sources & sources,
                           PositionTrajectoryManager & positionAutomationManager,
                           PersistentStorage & storage) noexcept;
    ~PositionFieldComponent() noexcept override = default;

    PositionFieldComponent(PositionFieldComponent const &) = delete;
    PositionFieldComponent(PositionFieldComponent &&) = delete;

    PositionFieldComponent & operator=(PositionFieldComponent const &) = delete;
    PositionFieldComponent & operator=(PositionFieldComponent &&) = delete;
    //==============================================================================
    [[nodiscard]] PositionTrajectoryManager const & getAutomationManager() const { return mAutomationManager; }
    PositionTrajectoryManager & getAutomationManager() { return mAutomationManager; }

    void drawDomeSpans(juce::Graphics & g) const;
    void drawCubeSpans(juce::Graphics & g) const;
    void setSpatMode(SpatMode spatMode);

    void drawSpans(juce::Graphics & g) const override;
    void paint(juce::Graphics & g) override;
    void resized() override;
    void rebuildSourceComponents(int numberOfSources) override;
    [[nodiscard]] juce::Rectangle<float> getEffectiveArea() const override;
    void notifySourcePositionChanged(SourceIndex sourceIndex) override;

    [[nodiscard]] juce::Point<float> sourcePositionToComponentPosition(juce::Point<float> const & sourcePosition) const;
    [[nodiscard]] juce::Line<float> sourcePositionToComponentPosition(juce::Line<float> const & sourcePosition) const;
    [[nodiscard]] juce::Point<float>
        componentPositionToSourcePosition(juce::Point<float> const & componentPosition) const;

private:
    //==============================================================================
    void mouseDown(juce::MouseEvent const & event) override;
    void mouseDrag(juce::MouseEvent const & event) override;
    void mouseUp(juce::MouseEvent const & event) override;
    void mouseMove(juce::MouseEvent const & event) override;
    void applySourceSelectionToComponents() override;
    void drawBackground(juce::Graphics & g) const override;
    //==============================================================================
    JUCE_LEAK_DETECTOR(PositionFieldComponent)

}; // class PositionFieldComponent

//==============================================================================
class ElevationFieldComponent final : public FieldComponent
{
    static constexpr auto TOP_PADDING{ SOURCE_FIELD_COMPONENT_DIAMETER };
    static constexpr auto BOTTOM_PADDING{ SOURCE_FIELD_COMPONENT_DIAMETER };
    static constexpr auto LEFT_PADDING{ SOURCE_FIELD_COMPONENT_DIAMETER };
    static constexpr auto RIGHT_PADDING{ SOURCE_FIELD_COMPONENT_DIAMETER };

    ElevationTrajectoryManager & mAutomationManager;
    ElevationDrawingHandle mDrawingHandle{ *this };
    juce::OwnedArray<ElevationSourceComponent> mSourceComponents{};
    ElevationMode mElevationMode{};

public:
    //==============================================================================
    ElevationFieldComponent() = delete;
    ElevationFieldComponent(Sources & sources, ElevationTrajectoryManager & positionTrajectoryManager) noexcept;
    ~ElevationFieldComponent() noexcept override = default;

    ElevationFieldComponent(ElevationFieldComponent const &) = delete;
    ElevationFieldComponent(ElevationFieldComponent &&) = delete;

    ElevationFieldComponent & operator=(ElevationFieldComponent const &) = delete;
    ElevationFieldComponent & operator=(ElevationFieldComponent &&) = delete;
    //==============================================================================
    [[nodiscard]] ElevationTrajectoryManager const & getAutomationManager() const { return mAutomationManager; }
    ElevationTrajectoryManager & getAutomationManager() { return mAutomationManager; }
    [[nodiscard]] juce::Point<float> sourceElevationToComponentPosition(Radians sourceElevation,
                                                                        SourceIndex index) const;
    [[nodiscard]] Radians componentPositionToSourceElevation(juce::Point<float> const & componentPosition) const;
    //==============================================================================
    void setElevationMode(ElevationMode const & elevationMode);
    //==============================================================================
    void paint(juce::Graphics & g) override;
    void resized() override;
    void mouseDown(juce::MouseEvent const & event) override;
    void mouseDrag(juce::MouseEvent const & event) override;
    void mouseUp(juce::MouseEvent const & event) override;
    void notifySourcePositionChanged(SourceIndex sourceIndex) override;
    void rebuildSourceComponents(int numberOfSources) override;
    [[nodiscard]] juce::Rectangle<float> getEffectiveArea() const override;

private:
    //==============================================================================
    void drawSpans(juce::Graphics & g) const override;
    void drawBackground(juce::Graphics & g) const override;
    void applySourceSelectionToComponents() override;
    //==============================================================================
    JUCE_LEAK_DETECTOR(ElevationFieldComponent)

}; // class ElevationFieldComponent

} // namespace gris
