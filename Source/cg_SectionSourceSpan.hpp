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

#include "cg_ControlGrisLookAndFeel.hpp"
#include "cg_NumSlider.h"
#include "cg_Source.hpp"

namespace gris
{
//==============================================================================
class SectionSourceSpan final
    : public juce::Component
    , public NumSlider::Slider::Listener
{
public:
    //==============================================================================
    struct Listener {
        virtual ~Listener() = default;

        virtual void azimuthSpanDragStartedCallback() = 0;
        virtual void azimuthSpanDragEndedCallback() = 0;
        virtual void elevationSpanDragStartedCallback() = 0;
        virtual void elevationSpanDragEndedCallback() = 0;
        virtual void parameterChangedCallback(SourceParameter sourceId, double value) = 0;
    };

private:
    //==============================================================================
    GrisLookAndFeel & mGrisLookAndFeel;

    juce::ListenerList<Listener> mListeners;

    bool mDistanceEnabled{ false };
    bool mSpanLinked{ false };

    Source * mSelectedSource{};

    juce::Label mAzimuthLabel{};
    juce::Label mElevationLabel{};
    NumSlider mAzimuthSpan;
    NumSlider mElevationSpan;

public:
    //==============================================================================
    SectionSourceSpan(GrisLookAndFeel & grisLookAndFeel);
    //==============================================================================
    SectionSourceSpan() = delete;
    ~SectionSourceSpan() override = default;

    SectionSourceSpan(SectionSourceSpan const &) = delete;
    SectionSourceSpan(SectionSourceSpan &&) = delete;

    SectionSourceSpan & operator=(SectionSourceSpan const &) = delete;
    SectionSourceSpan & operator=(SectionSourceSpan &&) = delete;
    //==============================================================================
    void mouseDown(juce::MouseEvent const & event) override;
    void sliderValueChanged(NumSlider::Slider * slider) override;
    void paint(juce::Graphics &) override;
    void resized() override;

    void setSelectedSource(Source * source);
    void setDistanceEnabled(bool distanceEnabled);
    void setSpanLinkState(bool spanLinkState);
    bool getSpanLinkState() const { return mSpanLinked; }
    Source * getSectionSourceSpanSelectedSource() { return mSelectedSource; }

    void addListener(Listener * l) { mListeners.add(l); }
    void removeListener(Listener * l) { mListeners.remove(l); }

private:
    //==============================================================================
    JUCE_LEAK_DETECTOR(SectionSourceSpan)
};

} // namespace gris
