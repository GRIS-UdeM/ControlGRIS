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

namespace gris
{
//==============================================================================
class GrisLookAndFeel final : public juce::LookAndFeel_V4
{
    float mFontSize;
    juce::Font mFont{ juce::FontOptions(BinaryData::SinkinSans400Regular_otf,
                                        juce::Font::plain,
                                        static_cast<size_t>(BinaryData::SinkinSans400Regular_otfSize)) };
    juce::Font mNumSliderFont{ juce::FontOptions(BinaryData::SinkinSans400Regular_otf,
                                                 juce::Font::plain,
                                                 static_cast<size_t>(BinaryData::SinkinSans400Regular_otfSize)) };
    juce::Font mBigFont{ juce::FontOptions(BinaryData::SinkinSans400Regular_otf,
                                           juce::Font::plain,
                                           static_cast<size_t>(BinaryData::SinkinSans400Regular_otfSize)) };
    juce::Font mBiggerFont{ juce::FontOptions(BinaryData::SinkinSans400Regular_otf,
                                              juce::Font::plain,
                                              static_cast<size_t>(BinaryData::SinkinSans400Regular_otfSize)) };

    juce::Colour mBackgroundAndFieldColor;
    juce::Colour mWinBackgroundAndFieldColor;
    juce::Colour mLightColor;
    juce::Colour mDarkColor;
    juce::Colour mGreyColor;
    juce::Colour mEditBackgroundColor;
    juce::Colour mDisableBackgroundColor;
    juce::Colour mHlBackgroundColor;
    juce::Colour mOnColor;
    juce::Colour mOnColorOver;
    juce::Colour mOnColorDown;
    juce::Colour mOffColor;
    juce::Colour mGreenColor;
    juce::Colour mRedColor;

public:
    //==============================================================================
    GrisLookAndFeel();

    [[nodiscard]] juce::Font getFont() const { return this->mFont; }
    [[nodiscard]] juce::Font getNumSliderFont() const { return this->mNumSliderFont; }
    juce::Font getLabelFont([[maybe_unused]] juce::Label & label) override { return this->mFont; }
    juce::Font getComboBoxFont([[maybe_unused]] juce::ComboBox & comboBox) override { return this->mFont; }
    juce::Font getPopupMenuFont() override { return this->mFont; }
    juce::Font getTextButtonFont(juce::TextButton &, [[maybe_unused]] int buttonHeight) override { return this->mFont; }
    juce::Font getMenuBarFont(juce::MenuBarComponent &,
                              [[maybe_unused]] int itemIndex,
                              [[maybe_unused]] juce::String const & itemText) override
    {
        return this->mFont;
    }

    [[nodiscard]] juce::Colour getWinBackgroundColor() const { return this->mWinBackgroundAndFieldColor; }
    [[nodiscard]] juce::Colour getBackgroundColor() const { return this->mBackgroundAndFieldColor; }
    [[nodiscard]] juce::Colour getFieldColor() const { return this->mBackgroundAndFieldColor; }
    [[nodiscard]] juce::Colour getFontColor() const { return this->mLightColor; }
    [[nodiscard]] juce::Colour getScrollBarColor() const { return this->mGreyColor; }
    [[nodiscard]] juce::Colour getDarkColor() const { return this->mDarkColor; }
    [[nodiscard]] juce::Colour getLightColor() const { return this->mLightColor; }
    [[nodiscard]] juce::Colour getEditBackgroundColor() const { return this->mEditBackgroundColor; }
    [[nodiscard]] juce::Colour getDisableBackgroundColor() const { return this->mDisableBackgroundColor; }
    [[nodiscard]] juce::Colour getHighlightColor() const { return this->mHlBackgroundColor; }
    [[nodiscard]] juce::Colour getOnColor() const { return this->mOnColor; }
    [[nodiscard]] juce::Colour getOffColor() const { return this->mOffColor; }
    [[nodiscard]] juce::Colour getGreenColor() const { return this->mGreenColor; }
    [[nodiscard]] juce::Colour getRedColor() const { return this->mRedColor; }
    [[nodiscard]] juce::Colour getGreyColor() const { return this->mGreyColor; }

    // https://github.com/audioplastic/Juce-look-and-feel-examples/blob/master/JuceLibraryCode/modules/juce_gui_basics/lookandfeel/juce_LookAndFeel.cpp

    void drawComboBox(juce::Graphics & g,
                      int /*width*/,
                      int /*height*/,
                      bool /*isMouseButtonDown*/,
                      int buttonX,
                      int buttonY,
                      int buttonW,
                      int buttonH,
                      juce::ComboBox & box) override;

    void positionComboBoxText(juce::ComboBox & box, juce::Label & label) override;

    void drawRoundThumb(juce::Graphics & g,
                        float x,
                        float y,
                        float diameter,
                        const juce::Colour & color,
                        float outlineThickness) const;

    void drawPopupMenuItem(juce::Graphics & g,
                           const juce::Rectangle<int> & area,
                           bool isSeparator,
                           bool isActive,
                           bool isHighlighted,
                           bool isTicked,
                           bool hasSubMenu,
                           const juce::String & text,
                           const juce::String & shortcutKeyText,
                           const juce::Drawable * icon,
                           const juce::Colour * textColourToUse) override;

    void drawButtonBackground(juce::Graphics & g,
                              juce::Button & button,
                              [[maybe_unused]] juce::Colour const & backgroundColor,
                              bool isMouseOverButton,
                              bool isButtonDown) override;

    void drawTickBox(juce::Graphics & g,
                     juce::Component & component,
                     float x,
                     float y,
                     float w,
                     float h,
                     bool ticked,
                     [[maybe_unused]] bool isEnabled,
                     [[maybe_unused]] bool isMouseOverButton,
                     [[maybe_unused]] bool isButtonDown) override;

    void drawLinearSliderThumb(juce::Graphics & g,
                               int x,
                               int y,
                               int width,
                               int height,
                               float sliderPos,
                               [[maybe_unused]] float minSliderPos,
                               [[maybe_unused]] float maxSliderPos,
                               juce::Slider::SliderStyle const style,
                               juce::Slider & slider) override;

    void drawLinearSlider(juce::Graphics & g,
                          int x,
                          int y,
                          int width,
                          int height,
                          float sliderPos,
                          float minSliderPos,
                          float maxSliderPos,
                          juce::Slider::SliderStyle style,
                          juce::Slider & slider) override;

    void drawLinearSliderBackground(juce::Graphics & g,
                                    int x,
                                    int y,
                                    int width,
                                    int height,
                                    float /*sliderPos*/,
                                    float /*minSliderPos*/,
                                    float /*maxSliderPos*/,
                                    juce::Slider::SliderStyle /*style*/,
                                    juce::Slider & slider) override;

    void fillTextEditorBackground(juce::Graphics & g,
                                  [[maybe_unused]] int width,
                                  [[maybe_unused]] int height,
                                  juce::TextEditor & t) override;

    void drawTextEditorOutline(juce::Graphics & g, int width, int height, juce::TextEditor & t) override;

    void drawToggleButton(juce::Graphics & g,
                          juce::ToggleButton & button,
                          bool isMouseOverButton,
                          bool isButtonDown) override;

    void drawTabButton(juce::TabBarButton & button, juce::Graphics & g, bool isMouseOver, bool isMouseDown) override;

    void createTabTextLayout(const juce::TabBarButton & button,
                             float length,
                             float depth,
                             juce::Colour color,
                             juce::TextLayout & textLayout) const;

    // we don't use those, so far
    void drawRotarySlider(juce::Graphics & g,
                          int x,
                          int y,
                          int width,
                          int height,
                          float sliderPos,
                          float rotaryStartAngle,
                          float rotaryEndAngle,
                          juce::Slider & slider) override;
};

} // namespace gris
