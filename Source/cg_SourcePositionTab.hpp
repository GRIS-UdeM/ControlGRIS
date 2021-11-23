/**************************************************************************
 * Copyright 2021 UdeM - GRIS - Samuel Béland & Olivier Belanger          *
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

#include "cg_ControlGrisLookAndFeel.hpp"
#include "cg_SourcePlacement.hpp"
#include "cg_constants.hpp"

#include <optional>

namespace gris
{
class Source;
class SourcePositionTab;

//==============================================================================
/** A bunch of sliders used to modify a source's position when in DOME mode. */
class DomeControls final : public juce::Component
{
    SourcePositionTab & mSourceBoxComponent;

    Degrees mCurrentAzimuth;
    Radians mCurrentElevation;
    juce::Label mElevationLabel;
    juce::Label mAzimuthLabel;
    juce::Slider mElevationSlider;
    juce::Slider mAzimuthSlider;

public:
    //==============================================================================
    explicit DomeControls(SourcePositionTab & sourceBoxComponent);
    DomeControls() = delete;
    ~DomeControls() override = default;
    //==============================================================================
    DomeControls(DomeControls const &) = delete;
    DomeControls(DomeControls &&) = delete;
    DomeControls & operator=(DomeControls const &) = delete;
    DomeControls & operator=(DomeControls &&) = delete;
    //==============================================================================
    void updateSliderValues(Source * source);

private:
    //==============================================================================
    JUCE_LEAK_DETECTOR(DomeControls)
}; // DomeControls

//==============================================================================
/** A bunch of sliders used to modify a source's position when in CUBE mode. */
class CubeControls final : public juce::Component
{
    SourcePositionTab & mSourceBoxComponent;

    float mCurrentX{};
    float mCurrentY{};
    float mCurrentZ{};
    juce::Label mXLabel{};
    juce::Label mYLabel{};
    juce::Label mZLabel{};
    juce::Slider mXSlider{};
    juce::Slider mYSlider{};
    juce::Slider mZSlider{};

public:
    //==============================================================================
    explicit CubeControls(SourcePositionTab & sourceBoxComponent);
    ~CubeControls() override = default;
    //==============================================================================
    CubeControls(CubeControls const &) = delete;
    CubeControls(CubeControls &&) = delete;
    CubeControls & operator=(CubeControls const &) = delete;
    CubeControls & operator=(CubeControls &&) = delete;
    //==============================================================================
    void updateSliderValues(Source * source);

private:
    //==============================================================================
    JUCE_LEAK_DETECTOR(CubeControls)
}; // CubeControls

//==============================================================================
/** A tabbed component used to precisely modify a the selected source's position. */
class SourcePositionTab final : public juce::Component
{
    friend DomeControls;
    friend CubeControls;

public:
    //==============================================================================
    class Listener
    {
    public:
        Listener() = default;
        virtual ~Listener() = default;
        //==============================================================================
        Listener(Listener const &) = delete;
        Listener(Listener &&) = default;
        Listener & operator=(Listener const &) = delete;
        Listener & operator=(Listener &&) = default;
        //==============================================================================
        virtual void sourcesPlacementChangedCallback(SourcePlacement value) = 0;
        virtual void sourceSelectionChangedCallback(SourceIndex sourceIndex) = 0;
        virtual void sourcePositionChangedCallback(SourceIndex sourceIndex,
                                                   std::optional<Radians> azimuth,
                                                   std::optional<Radians> elevation,
                                                   std::optional<float> x,
                                                   std::optional<float> y,
                                                   std::optional<float> z)
            = 0;

    private:
        //==============================================================================
        JUCE_LEAK_DETECTOR(Listener)
    };

private:
    //==============================================================================
    GrisLookAndFeel & mGrisLookAndFeel;

    juce::ListenerList<Listener> mListeners;
    SourceIndex mSelectedSource;
    juce::Label mSourcePlacementLabel;
    juce::ComboBox mSourcePlacementCombo;
    juce::Label mSourceNumberLabel;
    juce::ComboBox mSourceNumberCombo;

    DomeControls mDomeControls;
    CubeControls mCubeControls;

public:
    //==============================================================================
    explicit SourcePositionTab(GrisLookAndFeel & grisLookAndFeel, SpatMode spatMode);
    SourcePositionTab() = delete;
    ~SourcePositionTab() override = default;
    //==============================================================================
    SourcePositionTab(SourcePositionTab const &) = delete;
    SourcePositionTab(SourcePositionTab &&) = delete;
    SourcePositionTab & operator=(SourcePositionTab const &) = delete;
    SourcePositionTab & operator=(SourcePositionTab &&) = delete;
    //==============================================================================
    void setNumberOfSources(int numOfSources, SourceId firstSourceId);
    void updateSelectedSource(Source * source, SourceIndex sourceIndex, SpatMode spatMode);
    void addListener(Listener * l) { mListeners.add(l); }
    void removeListener(Listener * l) { mListeners.remove(l); }
    void setSpatMode(SpatMode spatMode);
    //==============================================================================
    void paint(juce::Graphics &) override;
    void resized() override;

private:
    //==============================================================================
    JUCE_LEAK_DETECTOR(SourcePositionTab)
};

} // namespace gris