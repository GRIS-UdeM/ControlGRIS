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
    mCurrentElevation = Radians{ MAX_ELEVATION };

    mElevationLabel.setText("Elevation:", juce::NotificationType::dontSendNotification);
    addAndMakeVisible(&mElevationLabel);

    mElevationSlider.setNormalisableRange(juce::NormalisableRange<double>(0.0f, 1.0f, 0.01f));
    mElevationSlider.setValue(1.0, juce::NotificationType::dontSendNotification);
    addAndMakeVisible(&mElevationSlider);
    mElevationSlider.onValueChange = [this] {
        mCurrentElevation = Radians{ MAX_ELEVATION * (1.0f - static_cast<float>(mElevationSlider.getValue())) };
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
                                            Radians{ mCurrentAzimuth },
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
    mCurrentElevation = Radians{ MAX_ELEVATION * source->getNormalizedElevation().get() };

    if (mCurrentAzimuth.getAsDegrees() < 0.0f) {
        mCurrentAzimuth += Degrees{ 360.0f };
    }
    mAzimuthSlider.setValue(mCurrentAzimuth.getAsDegrees(), juce::NotificationType::dontSendNotification);
    mElevationSlider.setValue(1.0f - mCurrentElevation / Radians{ MAX_ELEVATION },
                              juce::NotificationType::dontSendNotification);
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

    auto const setLine = [&](juce::Label & label, NumSlider & slider, int x, int y) {
        constexpr auto X_SPACE = 30;
        label.setBounds(x, y + 1, 30, 10);
        slider.setBounds(x + X_SPACE, y, 35, 12);
    };

    setLine(mXLabel, mXSlider, -10, 3); // why -10 ?
    setLine(mYLabel, mYSlider, 50, 3);
    setLine(mZLabel, mZSlider, 110, 3);
}

//==============================================================================
void CubeControls::updateSliderValues(Source * source)
{
    mCurrentX = source->getX();
    mCurrentY = source->getY();
    mCurrentZ = 1.0f - source->getElevation() / Radians{ MAX_ELEVATION };

    mXSlider.setValue(mCurrentX, juce::NotificationType::dontSendNotification);
    mYSlider.setValue(mCurrentY * -1.0f, juce::NotificationType::dontSendNotification);
    mZSlider.setValue(mCurrentZ, juce::NotificationType::dontSendNotification);
}

//==============================================================================
SectionSourcePosition::SectionSourcePosition(GrisLookAndFeel & grisLookAndFeel,
                                             SpatMode const spatMode,
                                             SectionSourceSpan & sectionSourceSpan,
                                             juce::AudioProcessorValueTreeState & apvts)
    : mGrisLookAndFeel(grisLookAndFeel)
    , mSectionSourceSpan(sectionSourceSpan)
    , mAPVTS(apvts)
    , mZSourceLinkScaleSlider(grisLookAndFeel)
    , mDomeControls(*this, grisLookAndFeel)
    , mCubeControls(*this, grisLookAndFeel)
{
    setName("SectionSourcePosition");

    mSelectedSource = SourceIndex{};

    mSourcesBanner.setLookAndFeel(&mGrisLookAndFeel);
    mSourcesBanner.setText("Sources", juce::dontSendNotification);
    addAndMakeVisible(&mSourcesBanner);

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

    // Speaker Setup as Source position

    auto folderImage = juce::ImageCache::getFromMemory(BinaryData::folder_icon_png, BinaryData::folder_icon_pngSize);

    mLoadSpeakerSetupButton.setImages(false,
                                      true,
                                      true,
                                      folderImage,
                                      1.0f,
                                      juce::Colours::transparentWhite,
                                      folderImage,
                                      0.7f,
                                      juce::Colours::transparentWhite,
                                      folderImage,
                                      0.7f,
                                      juce::Colours::transparentWhite);
    mLoadSpeakerSetupButton.setTooltip("Load Speaker Setup as source positions");

    addAndMakeVisible(&mLoadSpeakerSetupButton);
    mLoadSpeakerSetupButton.onClick = [this] {
        juce::FileChooser chooser("Select a Speaker Setup file...", {}, "*.xml");
        if (chooser.browseForFileToOpen())
            mListeners.call([&](Listener & l) { l.speakerSetupSelectedCallback(chooser.getResult()); });
    };

    // Source Number
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

    mZSourceLinkLabel.setText("Z Sources Link", juce::dontSendNotification);
    addAndMakeVisible(&mZSourceLinkLabel);

    mZSourceLinkCombo.addItemList(ELEVATION_SOURCE_LINK_TYPES, 1);
    mZSourceLinkCombo.setSelectedId(1);
    addChildComponent(&mZSourceLinkCombo);
    mZSourceLinkCombo.onChange = [this] {
        mListeners.call([&](Listener & l) {
            l.elevationSourceLinkChangedCallback(static_cast<ElevationSourceLink>(mZSourceLinkCombo.getSelectedId()));
        });

        if (mZSourceLinkCombo.getSelectedItemIndex() == 2 || mZSourceLinkCombo.getSelectedItemIndex() == 3) {
            mZSourceLinkScaleLabel.setVisible(true);
            mZSourceLinkScaleSlider.setVisible(true);
            mZSourceLinkCombo.setBounds(120, 102 + titleHeight, 96, 15);
            mZSourceLinkScaleLabel.setBounds(214, 106 + titleHeight, 40, 10);
            mZSourceLinkScaleSlider.setBounds(250, 104 + titleHeight, 35, 12);
        } else {
            mZSourceLinkScaleLabel.setVisible(false);
            mZSourceLinkScaleSlider.setVisible(false);
            mZSourceLinkCombo.setBounds(120, 102 + titleHeight, 165, 15);
        }
    };

    mZSourceLinkScaleLabel.setText("Scale", juce::dontSendNotification);
    addAndMakeVisible(&mZSourceLinkScaleLabel);

    mZSourceLinkScaleSlider.setNormalisableRange(juce::NormalisableRange<double>(0.0, 1.0, 0.01));
    auto eleSourceLinkScale{ mAPVTS.state.getProperty("eleSourceLinkScale") };
    if (eleSourceLinkScale.isVoid()) {
        eleSourceLinkScale = 1.0;
    }
    mZSourceLinkScaleSlider.setValue(eleSourceLinkScale, juce::dontSendNotification);
    mZSourceLinkScaleSlider.setNumDecimalPlacesToDisplay(2);
    mZSourceLinkScaleSlider.setDoubleClickReturnValue(true, 1.0);
    addAndMakeVisible(&mZSourceLinkScaleSlider);
    mZSourceLinkScaleSlider.onValueChange = [this] {
        auto scaleVal{ mZSourceLinkScaleSlider.getValue() };
        mAPVTS.state.setProperty("eleSourceLinkScale", scaleVal, nullptr);
        mListeners.call([&](Listener & l) { l.elevationSourceLinkScaleChangedCallback(scaleVal); });
    };

    // Other controls
    addAndMakeVisible(&mDomeControls);
    addAndMakeVisible(&mCubeControls);
    addAndMakeVisible(&mSectionSourceSpan);
    setSpatMode(spatMode);
}

void SectionSourcePosition::mouseDown(juce::MouseEvent const & event)
{
    auto const x{ 267.0f };
    auto const y{ 15.0f + titleHeight };
    // Area where the selected source is shown.
    juce::Rectangle<float> const selectedSourceArea{ x - 5.0f, y - 13.0f, 24.0f, 24.0f };
    if (selectedSourceArea.contains(event.getMouseDownPosition().toFloat())) {
        mListeners.call([&](Listener & l) { l.selectedSourceClickedCallback(); });
    }
}

//==============================================================================
void SectionSourcePosition::paint(juce::Graphics & g)
{
    Source * selectedSource = mSectionSourceSpan.getSectionSourceSpanSelectedSource();

    // draw Source ellipse
    if (selectedSource != nullptr) {
        auto const x{ 267.0f };
        auto const y{ 5.0f + titleHeight };

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
    const auto width = getWidth();

    mSourcesBanner.setBounds(0, 0, width, titleHeight);

    mLoadSpeakerSetupButton.setBounds(width - 40, 2, 40, titleHeight - 4);

    mSourceNumberLabel.setBounds(5, 10 + titleHeight, 150, 10);
    mSourceNumberCombo.setBounds(70, 7 + titleHeight, 50, 15);

    mDomeControls.setBounds(5, 30 + titleHeight, 300, 15);
    mCubeControls.setBounds(5, 30 + titleHeight, 300, 15);

    mSectionSourceSpan.setBounds(0, 50 + titleHeight, getWidth(), 25);

    mSourceLinkLabel.setBounds(5, 81 + titleHeight, 150, 10);
    mPositionSourceLinkCombo.setBounds(120, 77 + titleHeight, 165, 15);

    if (mSpatMode == SpatMode::cube) {
        mZSourceLinkLabel.setVisible(true);
        mZSourceLinkCombo.setVisible(true);
        mZSourceLinkLabel.setBounds(5, 106 + titleHeight, 150, 10);
        if (mZSourceLinkCombo.getSelectedItemIndex() == 3 || mZSourceLinkCombo.getSelectedItemIndex() == 4) {
            mZSourceLinkScaleLabel.setVisible(true);
            mZSourceLinkScaleSlider.setVisible(true);
            mZSourceLinkCombo.setBounds(120, 102 + titleHeight, 96, 15);
            mZSourceLinkScaleLabel.setBounds(214, 106 + titleHeight, 40, 10);
            mZSourceLinkScaleSlider.setBounds(250, 104 + titleHeight, 35, 12);
        } else {
            mZSourceLinkScaleLabel.setVisible(false);
            mZSourceLinkScaleSlider.setVisible(false);
            mZSourceLinkCombo.setBounds(120, 102 + titleHeight, 165, 15);
        }
        mSourcePlacementLabel.setBounds(5, 130 + titleHeight, 150, 10);
        mSourcePlacementCombo.setBounds(120, 127 + titleHeight, 165, 15);
    } else {
        mZSourceLinkLabel.setVisible(false);
        mZSourceLinkCombo.setVisible(false);
        mZSourceLinkScaleLabel.setVisible(false);
        mZSourceLinkScaleSlider.setVisible(false);
        mSourcePlacementLabel.setBounds(5, 105 + titleHeight, 150, 10);
        mSourcePlacementCombo.setBounds(120, 102 + titleHeight, 165, 15);
    }
}

//==============================================================================
void SectionSourcePosition::setPositionSourceLink(PositionSourceLink value)
{
    mPositionSourceLinkCombo.setSelectedId(static_cast<int>(value));
}

//==============================================================================
void SectionSourcePosition::setElevationSourceLink(ElevationSourceLink value)
{
    mZSourceLinkCombo.setSelectedId(static_cast<int>(value));
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
        mZSourceLinkCombo.setSelectedId(1);
        mZSourceLinkCombo.setEnabled(false);
    } else {
        mPositionSourceLinkCombo.setEnabled(true);
        mZSourceLinkCombo.setEnabled(true);
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
    mSpatMode = spatMode;

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

    resized();
}

//==============================================================================
void SectionSourcePosition::actualizeValueTreeState()
{
    mZSourceLinkScaleSlider.onValueChange();
}

} // namespace gris
