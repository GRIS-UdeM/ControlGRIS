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

#include "cg_SectionSourceSpan.hpp"

namespace gris
{
//==============================================================================
SectionSourceSpan::SectionSourceSpan(GrisLookAndFeel & grisLookAndFeel)
    : mGrisLookAndFeel(grisLookAndFeel)
    , mAzimuthSpan(grisLookAndFeel)
    , mElevationSpan(grisLookAndFeel)
{
    setName("SectionSourcesSpan");

    mAzimuthLabel.setText("Azimuth Span", juce::NotificationType::dontSendNotification);
    addAndMakeVisible(&mAzimuthLabel);

    mElevationLabel.setText("Elevation Span", juce::NotificationType::dontSendNotification);
    addAndMakeVisible(&mElevationLabel);

    mAzimuthSpan.addListener(this);
    mElevationSpan.addListener(this);

    mAzimuthSpan.onDragStart = [&]() { mListeners.call([&](Listener & l) { l.azimuthSpanDragStartedCallback(); }); };
    mAzimuthSpan.onDragEnd = [&]() { mListeners.call([&](Listener & l) { l.azimuthSpanDragEndedCallback(); }); };
    mElevationSpan.onDragStart
        = [&]() { mListeners.call([&](Listener & l) { l.elevationSpanDragStartedCallback(); }); };
    mElevationSpan.onDragEnd = [&]() { mListeners.call([&](Listener & l) { l.elevationSpanDragEndedCallback(); }); };

    addAndMakeVisible(&mAzimuthSpan);
    addAndMakeVisible(&mElevationSpan);
}

//==============================================================================
void SectionSourceSpan::setSelectedSource(Source * source)
{
    if (mSelectedSource != source || source->getAzimuthSpan().get() != mAzimuthSpan.getValue()
        || mElevationSpan.getValue() != source->getElevationSpan().get()) {
        mSelectedSource = source;
        mAzimuthSpan.setValue(source->getAzimuthSpan().get(), juce::NotificationType::dontSendNotification);
        mElevationSpan.setValue(source->getElevationSpan().get(), juce::NotificationType::dontSendNotification);
        repaint();
    }
}

//==============================================================================
void SectionSourceSpan::setDistanceEnabled(bool const distanceEnabled)
{
    mDistanceEnabled = distanceEnabled;
    resized();
}

//==============================================================================
void SectionSourceSpan::setSpanLinkState(bool const spanLinkState)
{
    mSpanLinked = spanLinkState;
    repaint();
}

//==============================================================================
void SectionSourceSpan::mouseDown(juce::MouseEvent const & event)
{
    // Area where the spanLinked arrow is shown.
    juce::Rectangle<float> const spanLinkedArrowArea{ 132.0f, 5.0f, 30.0f, 17.0f };
    if (spanLinkedArrowArea.contains(event.getMouseDownPosition().toFloat())) {
        mSpanLinked = !mSpanLinked;
        repaint();
    }
}

//==============================================================================
void SectionSourceSpan::sliderValueChanged(NumSlider::Slider * slider)
{
    auto const value{ slider->getValue() };
    auto const parameterId{ (slider == &mAzimuthSpan) ? SourceParameter::azimuthSpan : SourceParameter::elevationSpan };

    mListeners.call([&](Listener & l) { l.parameterChangedCallback(parameterId, value); });

    if (mSpanLinked) {
        if (parameterId == SourceParameter::azimuthSpan) {
            mElevationSpan.setValue(value, juce::NotificationType::sendNotificationAsync);
            mListeners.call([&](Listener & l) { l.parameterChangedCallback(SourceParameter::elevationSpan, value); });
        } else if (parameterId == SourceParameter::elevationSpan) {
            mAzimuthSpan.setValue(value, juce::NotificationType::sendNotificationAsync);
            mListeners.call([&](Listener & l) { l.parameterChangedCallback(SourceParameter::azimuthSpan, value); });
        }
    }
}

//==============================================================================
void SectionSourceSpan::paint(juce::Graphics & g)
{
    if (mSpanLinked)
        g.setColour(juce::Colours::orange);
    else
        g.setColour(juce::Colours::black);
    g.drawArrow(juce::Line<float>(155.0f, 13.0f, 135.0f, 13.0f), 4, 10, 7);
    g.drawArrow(juce::Line<float>(140.0f, 13.0f, 160.0f, 13.0f), 4, 10, 7);
}

//==============================================================================
void SectionSourceSpan::resized()
{
    mAzimuthLabel.setBounds(5, 3, 80, 20);
    mAzimuthSpan.setBounds(85, 7, 35, 12);
    mElevationLabel.setBounds(170, 3, 80, 20);
    mElevationSpan.setBounds(250, 7, 35, 12);
}

} // namespace gris
