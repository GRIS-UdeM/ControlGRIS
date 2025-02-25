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
DomeControls::DomeControls(SectionSourcePosition & sourceBoxComponent) : mSourceBoxComponent(sourceBoxComponent)
{
    mCurrentAzimuth = {};
    mCurrentElevation = MAX_ELEVATION;

    mElevationLabel.setText("Elevation:", juce::NotificationType::dontSendNotification);
    addAndMakeVisible(&mElevationLabel);

    mElevationSlider.setNormalisableRange(juce::NormalisableRange<double>(0.0f, 1.0f, 0.01f));
    mElevationSlider.setValue(1.0, juce::NotificationType::dontSendNotification);
    mElevationSlider.setTextBoxStyle(juce::Slider::TextBoxRight, false, 40, 20);
    mElevationSlider.setColour(juce::Slider::textBoxOutlineColourId, juce::Colours::transparentBlack);
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

    mAzimuthSlider.setNormalisableRange(juce::NormalisableRange<double>(0.0f, 360.0f, 0.01f));
    mAzimuthSlider.setValue(0.0, juce::NotificationType::dontSendNotification);
    mAzimuthSlider.setTextBoxStyle(juce::Slider::TextBoxRight, false, 40, 20);
    mAzimuthSlider.setColour(juce::Slider::textBoxOutlineColourId, juce::Colours::transparentBlack);
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

    mElevationLabel.setBounds(0, 0, 150, 15);
    mElevationSlider.setBounds(75, 0, 200, 20);

    mAzimuthLabel.setBounds(0, 30, 150, 15);
    mAzimuthSlider.setBounds(75, 30, 200, 20);
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
CubeControls::CubeControls(SectionSourcePosition & sourceBoxComponent) : mSourceBoxComponent(sourceBoxComponent)
{
    auto const initLabel = [&](juce::Label & label, juce::String const & text) {
        label.setText(text + ":", juce::NotificationType::dontSendNotification);
        label.setJustificationType(juce::Justification::centredRight);
        addAndMakeVisible(label);
    };

    auto const initSlider = [&](juce::Slider & slider, double const minValue) {
        slider.setNormalisableRange(juce::NormalisableRange<double>{ minValue, 1.0, 0.01 });
        slider.setValue(0.0, juce::dontSendNotification);
        slider.setColour(juce::Slider::textBoxOutlineColourId, juce::Colours::transparentBlack);
        slider.setTextBoxStyle(juce::Slider::TextBoxRight, false, 40, 15);
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

    auto const setLine = [&](juce::Label & label, juce::Slider & slider, int const line) {
        static constexpr auto LINE_HEIGHT = 20;
        auto const y{ line * LINE_HEIGHT };
        label.setBounds(0, y, 70, LINE_HEIGHT);
        slider.setBounds(75, y, 200, LINE_HEIGHT);
    };

    setLine(mXLabel, mXSlider, 0);
    setLine(mYLabel, mYSlider, 1);
    setLine(mZLabel, mZSlider, 2);
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
SectionSourcePosition::SectionSourcePosition(GrisLookAndFeel & grisLookAndFeel, SpatMode const spatMode)
    : mGrisLookAndFeel(grisLookAndFeel)
    , mDomeControls(*this)
    , mCubeControls(*this)
{
    mSelectedSource = SourceIndex{};

    // Source Placement
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

    // Speaker Setup as Source Placement
    mSpeakerSetupLabel.setText("Load Speaker Setup...", juce::NotificationType::dontSendNotification);
    addAndMakeVisible(&mSpeakerSetupLabel);

    addAndMakeVisible(&mSpeakerSetupCombo);
    mSpeakerSetupCombo.setTextWhenNothingSelected("Choose a file...");
    mSpeakerSetupCombo.addItemList(SOURCE_PLACEMENT_SKETCH, 1);
    mSpeakerSetupCombo.onChange = [this] {
        mListeners.call([&](Listener & l) {
#if JUCE_MAC
            l.speakerSetupSelectedCallback({ "/Users/rymer/Documents/git/sat/GRIS/SpatGRIS/Resources/templates/Speaker setups/DOME/Dome4(4)Subs1 Quad.xml" });
#else
            l.speakerSetupSelectedCallback({ "C:/Users/barth/Documents/git/sat/GRIS/SpatGRIS/Resources/templates/Speaker setups/DOME/Dome4(4)Subs1 Quad.xml" });
#endif
        });
    };

    // Source Number
    mSourceNumberLabel.setText("Source Number:", juce::NotificationType::dontSendNotification);
    addAndMakeVisible(&mSourceNumberLabel);

    addAndMakeVisible(&mSourceNumberCombo);
    mSourceNumberCombo.setTextWhenNothingSelected("Choose a source...");
    for (auto i{ 1 }; i <= 8; ++i) {
        mSourceNumberCombo.addItem(juce::String{ i }, i);
    }
    mSourceNumberCombo.setSelectedId(mSelectedSource.get());
    mSourceNumberCombo.onChange = [this] {
        mSelectedSource = SourceIndex{ mSourceNumberCombo.getSelectedItemIndex() };
        mListeners.call([&](Listener & l) { l.sourceSelectionChangedCallback(mSelectedSource); });
    };

    // Other controls
    addAndMakeVisible(&mDomeControls);
    addAndMakeVisible(&mCubeControls);
    setSpatMode(spatMode);
}

//==============================================================================
void SectionSourcePosition::paint(juce::Graphics & g)
{
    g.fillAll(mGrisLookAndFeel.findColour(juce::ResizableWindow::backgroundColourId));
}

//==============================================================================
void SectionSourcePosition::resized()
{
    auto const positionLabelAndCombo = [](juce::Rectangle<int> && bounds, juce::Label & label, juce::ComboBox & combo) {
        auto const labelWidth = label.getFont().getStringWidth(label.getText());
        label.setBounds(bounds.removeFromLeft(labelWidth));
        combo.setBounds(bounds);
    };

    auto bounds = getLocalBounds().reduced(5, 10);

    //left half
    auto leftHalf = bounds.removeFromLeft(bounds.getWidth() / 2);
    positionLabelAndCombo(leftHalf.removeFromTop(20), mSourcePlacementLabel, mSourcePlacementCombo);
    leftHalf.removeFromTop(5);
    positionLabelAndCombo(leftHalf.removeFromTop(20), mSpeakerSetupLabel, mSpeakerSetupCombo);

    //right half
    positionLabelAndCombo(bounds.removeFromTop(20), mSourceNumberLabel, mSourceNumberCombo);

    bounds.removeFromTop(5);
    auto const domeAndCubeBounds{ bounds.removeFromTop(500) };
    mDomeControls.setBounds(domeAndCubeBounds);
    mCubeControls.setBounds(domeAndCubeBounds.withHeight (550));
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
