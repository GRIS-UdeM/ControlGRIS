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

#include "cg_SectionSourcePosition.hpp"

#include "cg_constants.hpp"

namespace gris
{
//==============================================================================
DomeControls::DomeControls(SectionSourcePosition & sourceBoxComponent, GrisLookAndFeel & grisLookAndFeel)
    : mSourceBoxComponent(sourceBoxComponent)
    , mElevationSlider(grisLookAndFeel)
    , mAzimuthSlider(grisLookAndFeel)
{
    mCurrentAzimuth = {};
    mCurrentElevation = MAX_ELEVATION;

    mElevationLabel.setText("Elevation:", juce::NotificationType::dontSendNotification);
    addAndMakeVisible(&mElevationLabel);

    mElevationSlider.setNormalisableRange(juce::NormalisableRange<double>(0.0f, 1.0f, 0.01f));
    mElevationSlider.setValue(1.0, juce::NotificationType::dontSendNotification);
    addAndMakeVisible(&mElevationSlider);
    mElevationSlider.onValueChange = [this] {
        mCurrentElevation = MAX_ELEVATION * (1.0f - static_cast<float>(mElevationSlider.getValue()));
        mSourceBoxComponent.mListeners.call([&](SectionSourcePosition::Listener & l) {
            l.sourcePositionChangedCallback(mSourceBoxComponent.mSelectedSource,
                                            std::nullopt,
                                            mCurrentElevation,
                                            std::nullopt,
                                            std::nullopt,
                                            std::nullopt);
        });
    };

    mAzimuthLabel.setText("Azimuth:", juce::NotificationType::dontSendNotification);
    addAndMakeVisible(&mAzimuthLabel);

    mAzimuthSlider.setNormalisableRange(juce::NormalisableRange<double>(0.0f, 360.0f, 0.1f));
    mAzimuthSlider.setValue(0.0, juce::NotificationType::dontSendNotification);
    mAzimuthSlider.setNumDecimalPlacesToDisplay(1);
    addAndMakeVisible(&mAzimuthSlider);
    mAzimuthSlider.onValueChange = [this] {
        mCurrentAzimuth = Degrees{ static_cast<float>(mAzimuthSlider.getValue()) };
        mSourceBoxComponent.mListeners.call([&](SectionSourcePosition::Listener & l) {
            l.sourcePositionChangedCallback(mSourceBoxComponent.mSelectedSource,
                                            mCurrentAzimuth,
                                            std::nullopt,
                                            std::nullopt,
                                            std::nullopt,
                                            std::nullopt);
        });
    };

    mAzimuthLabel.setBounds(0, 4, 100, 10);
    mAzimuthSlider.setBounds(80, 3, 35, 12);

    mElevationLabel.setBounds(165, 4, 100, 10);
    mElevationSlider.setBounds(245, 3, 35, 12);
}

//==============================================================================
void DomeControls::updateSliderValues(Source * source)
{
    mCurrentAzimuth = source->getAzimuth();
    mCurrentElevation = MAX_ELEVATION * source->getNormalizedElevation().get();

    if (mCurrentAzimuth.getAsDegrees() < 0.0f) {
        mCurrentAzimuth += Degrees{ 360.0f };
    }
    mAzimuthSlider.setValue(mCurrentAzimuth.getAsDegrees(), juce::NotificationType::dontSendNotification);
    mElevationSlider.setValue(1.0f - mCurrentElevation / MAX_ELEVATION, juce::NotificationType::dontSendNotification);
}

//==============================================================================
CubeControls::CubeControls(SectionSourcePosition & sourceBoxComponent, GrisLookAndFeel & grisLookAndFeel)
    : mSourceBoxComponent(sourceBoxComponent)
    , mXSlider(grisLookAndFeel)
    , mYSlider(grisLookAndFeel)
    , mZSlider(grisLookAndFeel)
{
    auto const initLabel = [&](juce::Label & label, juce::String const & text) {
        label.setText(text + ":", juce::NotificationType::dontSendNotification);
        label.setJustificationType(juce::Justification::centredRight);
        addAndMakeVisible(label);
    };

    auto const initSlider = [&](NumSlider & slider, double const minValue) {
        slider.setNormalisableRange(juce::NormalisableRange<double>{ minValue, 1.0, 0.001 });
        slider.setValue(0.0, juce::dontSendNotification);
        addAndMakeVisible(slider);
    };

    initLabel(mXLabel, "X");
    initLabel(mYLabel, "Y");
    initLabel(mZLabel, "Z");

    initSlider(mXSlider, -1.0);
    initSlider(mYSlider, -1.0);
    initSlider(mZSlider, 0.0);

    mXSlider.onValueChange = [this] {
        mCurrentX = static_cast<float>(mXSlider.getValue());
        mSourceBoxComponent.mListeners.call([&](SectionSourcePosition::Listener & l) {
            l.sourcePositionChangedCallback(mSourceBoxComponent.mSelectedSource,
                                            std::nullopt,
                                            std::nullopt,
                                            mCurrentX,
                                            std::nullopt,
                                            std::nullopt);
        });
    };
    mYSlider.onValueChange = [this] {
        mCurrentY = static_cast<float>(mYSlider.getValue() * -1.0);
        mSourceBoxComponent.mListeners.call([&](SectionSourcePosition::Listener & l) {
            l.sourcePositionChangedCallback(mSourceBoxComponent.mSelectedSource,
                                            std::nullopt,
                                            std::nullopt,
                                            std::nullopt,
                                            mCurrentY,
                                            std::nullopt);
        });
    };
    mZSlider.onValueChange = [this] {
        mCurrentZ = static_cast<float>(mZSlider.getValue());
        mSourceBoxComponent.mListeners.call([&](SectionSourcePosition::Listener & l) {
            l.sourcePositionChangedCallback(mSourceBoxComponent.mSelectedSource,
                                            std::nullopt,
                                            std::nullopt,
                                            std::nullopt,
                                            std::nullopt,
                                            1.0f - mCurrentZ);
        });
    };

    auto const setLine = [&](juce::Label & label, NumSlider & slider, double const line) {
        static constexpr auto LINE_HEIGHT = 10;
        auto const y{ line * LINE_HEIGHT };
        label.setBounds(0, y, 70, LINE_HEIGHT);
        slider.setBounds(75, y, 200, LINE_HEIGHT);
    };

    setLine(mXLabel, mXSlider, 0);
    setLine(mYLabel, mYSlider, 1.5);
    setLine(mZLabel, mZSlider, 3);
}

//==============================================================================
void CubeControls::updateSliderValues(Source * source)
{
    mCurrentX = source->getX();
    mCurrentY = source->getY();
    mCurrentZ = 1.0f - source->getElevation() / MAX_ELEVATION;

    mXSlider.setValue(mCurrentX, juce::NotificationType::dontSendNotification);
    mYSlider.setValue(mCurrentY * -1.0f, juce::NotificationType::dontSendNotification);
    mZSlider.setValue(mCurrentZ, juce::NotificationType::dontSendNotification);
}

//==============================================================================
SectionSourcePosition::SectionSourcePosition(GrisLookAndFeel & grisLookAndFeel,
                                             SpatMode const spatMode,
                                             SectionSourceSpan & sectionSourceSpan)
    : mGrisLookAndFeel(grisLookAndFeel)
    , mDomeControls(*this, grisLookAndFeel)
    , mCubeControls(*this, grisLookAndFeel)
    , mSectionSourceSpan(sectionSourceSpan)
{
    setName("SectionSourcePosition");

    mSelectedSource = SourceIndex{};

    mSourcePlacementLabel.setText("Sources Placement:", juce::NotificationType::dontSendNotification);
    addAndMakeVisible(&mSourcePlacementLabel);

    addAndMakeVisible(&mSourcePlacementCombo);
    mSourcePlacementCombo.setTextWhenNothingSelected("Choose a source placement...");
    mSourcePlacementCombo.addItemList(SOURCE_PLACEMENT_SKETCH, 1);
    mSourcePlacementCombo.onChange = [this] {
        mListeners.call([&](Listener & l) {
            l.sourcesPlacementChangedCallback(static_cast<SourcePlacement>(mSourcePlacementCombo.getSelectedId()));
            mSourcePlacementCombo.setSelectedId(0, juce::NotificationType::dontSendNotification);
        });
    };

    mSourceNumberLabel.setText("Source ID:", juce::NotificationType::dontSendNotification);
    addAndMakeVisible(&mSourceNumberLabel);

    addAndMakeVisible(&mSourceNumberCombo);
    mSourceNumberCombo.setLookAndFeel(&mGrisLookAndFeel);
    mSourceNumberCombo.setTextWhenNothingSelected("Choose a source...");
    for (auto i{ 1 }; i <= 8; ++i) {
        mSourceNumberCombo.addItem(juce::String{ i }, i);
    }
    mSourceNumberCombo.setSelectedId(mSelectedSource.get());
    mSourceNumberCombo.onChange = [this] {
        mSelectedSource = SourceIndex{ mSourceNumberCombo.getSelectedItemIndex() };
        mListeners.call([&](Listener & l) { l.sourceSelectionChangedCallback(mSelectedSource); });
        repaint();
    };

    mSourceLinkLabel.setText("Sources Link:", juce::NotificationType::dontSendNotification);
    addAndMakeVisible(&mSourceLinkLabel);

    mPositionSourceLinkCombo.addItemList(POSITION_SOURCE_LINK_TYPES, 1);
    mPositionSourceLinkCombo.setSelectedId(1);
    addAndMakeVisible(&mPositionSourceLinkCombo);
    mPositionSourceLinkCombo.onChange = [this] {
        mListeners.call([&](Listener & l) {
            l.positionSourceLinkChangedCallback(
                static_cast<PositionSourceLink>(mPositionSourceLinkCombo.getSelectedId()));
        });
    };

    addAndMakeVisible(&mDomeControls);
    addAndMakeVisible(&mCubeControls);
    addAndMakeVisible(&mSectionSourceSpan);
    setSpatMode(spatMode);
}

void SectionSourcePosition::mouseDown(juce::MouseEvent const & event)
{
    auto const x{ 267.0f };
    auto const y{ 15.0f };
    // Area where the selected source is shown.
    juce::Rectangle<float> const selectedSourceArea{ x - 5.0f, y - 13.0f, 24.0f, 24.0f };
    if (selectedSourceArea.contains(event.getMouseDownPosition().toFloat())) {
        mListeners.call([&](Listener & l) { l.selectedSourceClickedCallback(); });
    }
}

//==============================================================================
void SectionSourcePosition::paint(juce::Graphics & g)
{
    g.fillAll(mGrisLookAndFeel.findColour(juce::ResizableWindow::backgroundColourId));

    Source * selectedSource = mSectionSourceSpan.getSectionSourceSpanSelectedSource();

    // draw Source ellipse
    if (selectedSource != nullptr) {
        auto const x{ 267.0f };
        auto const y{ 5.0f };

        juce::Rectangle<float> area{ x, y, 15, 15 };
        area.expand(3, 3);
        g.setColour(juce::Colour(.2f, .2f, .2f, 1.0f));
        g.drawEllipse(area.translated(.5f, .5f), 1.0f);
        g.setGradientFill(juce::ColourGradient(selectedSource->getColour().withSaturation(1.0f).darker(1.0f),
                                               x + SOURCE_FIELD_COMPONENT_RADIUS,
                                               y + SOURCE_FIELD_COMPONENT_RADIUS,
                                               selectedSource->getColour().withSaturation(1.0f),
                                               x,
                                               y,
                                               true));
        g.fillEllipse(area);

        g.setColour(juce::Colours::white);
        g.drawFittedText(selectedSource->getId().toString(),
                         area.getSmallestIntegerContainer(),
                         juce::Justification::centred,
                         1);
    }
}

//==============================================================================
void SectionSourcePosition::resized()
{
    mSourceNumberLabel.setBounds(5, 10, 150, 10);
    mSourceNumberCombo.setBounds(93, 7, 27, 15);

    mDomeControls.setBounds(5, 30, 300, 15);
    mCubeControls.setBounds(105, 370, 275, 20); // adjust for Cube mode
    
    mSectionSourceSpan.setBounds(0, 50, getWidth(), 25);

    mSourceLinkLabel.setBounds(5, 81, 150, 10);
    mPositionSourceLinkCombo.setBounds(120, 77, 165, 17);

    mSourcePlacementLabel.setBounds(5, 105, 150, 10);
    mSourcePlacementCombo.setBounds(120, 102, 165, 17);
}

//==============================================================================
void SectionSourcePosition::setPositionSourceLink(PositionSourceLink value)
{
    mPositionSourceLinkCombo.setSelectedId(static_cast<int>(value));
}

//==============================================================================
void SectionSourcePosition::setSymmetricLinkComboState(bool allowed)
{
    mPositionSourceLinkCombo.setItemEnabled(static_cast<int>(PositionSourceLink::symmetricX), allowed);
    mPositionSourceLinkCombo.setItemEnabled(static_cast<int>(PositionSourceLink::symmetricY), allowed);
}

//==============================================================================
void SectionSourcePosition::setNumberOfSources(int const numOfSources, SourceId const firstSourceId)
{
    mSourceNumberCombo.clear();
    for (auto id = firstSourceId; id < firstSourceId + SourceId{ numOfSources }; ++id) {
        mSourceNumberCombo.addItem(id.toString(), id.get());
    }
    if (mSelectedSource >= SourceIndex{ numOfSources })
        mSelectedSource = SourceIndex{ 0 };
    mSourceNumberCombo.setSelectedItemIndex(mSelectedSource.get());

    if (numOfSources == 1) {
        mPositionSourceLinkCombo.setSelectedId(1);
        mPositionSourceLinkCombo.setEnabled(false);
    } else {
        mPositionSourceLinkCombo.setEnabled(true);
    }

    if (numOfSources == 2) {
        mPositionSourceLinkCombo.setItemEnabled(static_cast<int>(PositionSourceLink::symmetricX), true);
        mPositionSourceLinkCombo.setItemEnabled(static_cast<int>(PositionSourceLink::symmetricY), true);
    } else {
        mPositionSourceLinkCombo.setItemEnabled(static_cast<int>(PositionSourceLink::symmetricX), false);
        mPositionSourceLinkCombo.setItemEnabled(static_cast<int>(PositionSourceLink::symmetricY), false);
    }
}

//==============================================================================
void SectionSourcePosition::updateSelectedSource(Source * source, SourceIndex const sourceIndex, SpatMode /*spatMode*/)
{
    mSelectedSource = sourceIndex;
    mSourceNumberCombo.setSelectedItemIndex(mSelectedSource.get());
    mDomeControls.updateSliderValues(source);
    mCubeControls.updateSliderValues(source);
}

//==============================================================================
void SectionSourcePosition::setSpatMode(SpatMode const spatMode)
{
    switch (spatMode) {
    case SpatMode::dome:
        mDomeControls.setVisible(true);
        mCubeControls.setVisible(false);
        break;
    case SpatMode::cube:
        mDomeControls.setVisible(false);
        mCubeControls.setVisible(true);
        break;
    default:
        jassertfalse;
    }
}

} // namespace gris
