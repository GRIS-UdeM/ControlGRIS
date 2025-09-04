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

#include "cg_ControlGrisLookAndFeel.hpp"

namespace gris
{
//==============================================================================
GrisLookAndFeel::GrisLookAndFeel()
{
    mBackgroundAndFieldColor = juce::Colour::fromRGB(75, 75, 75);
    mWinBackgroundAndFieldColor = juce::Colour::fromRGB(46, 46, 46);
    mLightColor = juce::Colour::fromRGB(235, 245, 250);
    mDarkColor = juce::Colour::fromRGB(15, 10, 5);
    mGreyColor = juce::Colour::fromRGB(120, 120, 120);
    mDisableBackgroundColor = juce::Colour::fromRGB(150, 150, 150);
    mEditBackgroundColor = juce::Colour::fromRGB(172, 172, 172);
    mHlBackgroundColor = juce::Colour::fromRGB(190, 125, 18);
    mOnColor = juce::Colour::fromRGB(255, 165, 25);
    mOnColorOver = juce::Colour::fromRGB(255, 184, 75);
    mOnColorDown = juce::Colour::fromRGB(222, 144, 22);
    mOffColor = juce::Colour::fromRGB(56, 56, 56);
    mGreenColor = juce::Colour::fromRGB(56, 156, 56);
    mRedColor = juce::Colour::fromRGB(220, 48, 35);

    setColour(juce::PopupMenu::highlightedBackgroundColourId, this->mOnColor);
    setColour(juce::PopupMenu::backgroundColourId, this->mEditBackgroundColor);
    setColour(juce::PopupMenu::textColourId, this->mDarkColor);
    setColour(juce::TextEditor::textColourId, this->mLightColor);
    setColour(juce::TextEditor::backgroundColourId, this->mEditBackgroundColor);
    setColour(juce::TextEditor::highlightColourId, this->mHlBackgroundColor);
    setColour(juce::TextEditor::shadowColourId, this->mEditBackgroundColor);
    setColour(juce::TextButton::buttonColourId, this->mEditBackgroundColor);
    setColour(juce::TextButton::textColourOnId, this->mDarkColor);
    setColour(juce::TextButton::textColourOffId, this->mDarkColor);
    setColour(juce::ComboBox::backgroundColourId, this->mEditBackgroundColor);
    setColour(juce::ComboBox::outlineColourId, this->mEditBackgroundColor);
    setColour(juce::ComboBox::textColourId, this->mDarkColor);
    setColour(juce::Slider::thumbColourId, this->mLightColor);
    setColour(juce::Slider::rotarySliderFillColourId, this->mOnColor);
    setColour(juce::Slider::trackColourId, this->mDarkColor);
    setColour(juce::Slider::textBoxBackgroundColourId, this->mEditBackgroundColor);
    setColour(juce::Slider::textBoxOutlineColourId, juce::Colours::transparentBlack);
    setColour(juce::TooltipWindow::ColourIds::backgroundColourId, this->mBackgroundAndFieldColor.withBrightness(0.8f));
    setColour(juce::TooltipWindow::ColourIds::outlineColourId, this->mBackgroundAndFieldColor.withBrightness(0.8f));
    setColour(juce::AlertWindow::backgroundColourId, this->mWinBackgroundAndFieldColor);
    setColour(juce::AlertWindow::outlineColourId, this->mOnColor);
    setColour(juce::AlertWindow::textColourId, this->mLightColor);
    setColour(juce::ToggleButton::textColourId, this->mLightColor);
    setColour(juce::Label::textColourId, this->mLightColor);

#if JUCE_WINDOWS
    mFontSize = 11.0f;
#elif JUCE_LINUX
    mFontSize = 12.0f;
#else
    mFontSize = 10.0f;
#endif
    mFont.setHeight(mFontSize);
    mNumSliderFont.setHeight(mFontSize);
    mBigFont.setHeight(mFontSize + 3.0f);
    mBiggerFont.setHeight(mFontSize + 6.0f);
}

//==============================================================================
void GrisLookAndFeel::drawComboBox(juce::Graphics & g,
                                   int width,
                                   int height,
                                   const bool,
                                   int const buttonX,
                                   int const buttonY,
                                   int const buttonW,
                                   int const buttonH,
                                   juce::ComboBox & box)
{
    box.setColour(juce::ColourSelector::backgroundColourId, mOnColor);

    auto cornerSize = box.findParentComponentOfClass<juce::ChoicePropertyComponent>() != nullptr ? 0.0f : 2.0f;
    juce::Rectangle<int> boxBounds(0, 0, width, height);

    g.setColour(this->mEditBackgroundColor);
    g.fillRoundedRectangle(boxBounds.toFloat(), cornerSize);

    g.setColour(this->mEditBackgroundColor);
    g.drawRoundedRectangle(boxBounds.toFloat().reduced(0.5f, 0.5f), cornerSize, 1.0f);

    auto const arrowX{ 0.3f };
    auto const arrowH{ 0.2f };

    auto const buttonXFloat{ static_cast<float>(buttonX) };
    auto const buttonYFloat{ static_cast<float>(buttonY) };
    auto const buttonWFloat{ static_cast<float>(buttonW) };
    auto const buttonHFloat{ static_cast<float>(buttonH) };

    juce::Path p;
    p.addTriangle(buttonXFloat + buttonWFloat * 0.5f,
                  buttonYFloat + buttonHFloat * (0.45f - arrowH),
                  buttonXFloat + buttonWFloat * (1.0f - arrowX),
                  buttonYFloat + buttonHFloat * 0.45f,
                  buttonXFloat + buttonWFloat * arrowX,
                  buttonYFloat + buttonHFloat * 0.45f);

    p.addTriangle(buttonXFloat + buttonWFloat * 0.5f,
                  buttonYFloat + buttonHFloat * (0.55f + arrowH),
                  buttonXFloat + buttonWFloat * (1.0f - arrowX),
                  buttonYFloat + buttonHFloat * 0.55f,
                  buttonXFloat + buttonWFloat * arrowX,
                  buttonYFloat + buttonHFloat * 0.55f);

    g.setColour(this->mDarkColor.withMultipliedAlpha(
        box.isEnabled() ? 1.0f : 0.3f)); // box.findColour (ComboBox::arrowColourId)
    g.fillPath(p);
}

//==============================================================================
void GrisLookAndFeel::positionComboBoxText(juce::ComboBox & box, juce::Label & label)
{
    label.setBounds(1, 1, box.getWidth() - 20, box.getHeight() - 2);

    label.setFont(getComboBoxFont(box));
}

//==============================================================================
void GrisLookAndFeel::drawRoundThumb(juce::Graphics & g,
                                     float const x,
                                     float const y,
                                     float const diameter,
                                     const juce::Colour & color,
                                     float const outlineThickness) const
{
    auto const halfThickness{ outlineThickness * 0.5f };

    juce::Path p;
    p.addEllipse(x + halfThickness, y + halfThickness, diameter - outlineThickness, diameter - outlineThickness);

    const juce::DropShadow ds(this->mDarkColor, 1, juce::Point<int>(0, 0));
    ds.drawForPath(g, p);

    g.setColour(color);
    g.fillPath(p);

    g.setColour(color.brighter());
    g.strokePath(p, juce::PathStrokeType(outlineThickness));
}

//==============================================================================
void GrisLookAndFeel::drawPopupMenuItem(juce::Graphics & g,
                                        const juce::Rectangle<int> & area,
                                        const bool isSeparator,
                                        const bool isActive,
                                        const bool isHighlighted,
                                        const bool isTicked,
                                        const bool hasSubMenu,
                                        const juce::String & text,
                                        const juce::String & shortcutKeyText,
                                        const juce::Drawable * icon,
                                        const juce::Colour * const textColourToUse)
{
    if (isSeparator) {
        auto r = area.reduced(5, 0);
        r.removeFromTop(r.getHeight() / 2 - 1);

        g.setColour(juce::Colour(0x33000000));
        g.fillRect(r.removeFromTop(1));

        g.setColour(juce::Colour(0x66ffffff));
        g.fillRect(r.removeFromTop(1));
    } else {
        auto textColor = findColour(juce::PopupMenu::textColourId);

        if (textColourToUse != nullptr)
            textColor = *textColourToUse;

        auto r = area.reduced(1);

        if (isHighlighted) {
            g.setColour(findColour(juce::PopupMenu::highlightedBackgroundColourId));
            g.fillRect(r);

            g.setColour(findColour(juce::PopupMenu::highlightedTextColourId));
        } else {
            g.setColour(textColor);
        }

        if (!isActive)
            g.setOpacity(0.3f);

        auto font{ getPopupMenuFont() };

        auto const maxFontHeight{ area.getHeight() / 1.3f };

        if (font.getHeight() > maxFontHeight)
            font.setHeight(maxFontHeight);

        g.setFont(font);

        auto const iconArea{ r.removeFromLeft((r.getHeight() * 5) / 4).reduced(3).toFloat() };

        if (icon != nullptr) {
            icon->drawWithin(g,
                             iconArea,
                             juce::RectanglePlacement::centred | juce::RectanglePlacement::onlyReduceInSize,
                             1.0f);
        } else if (isTicked) {
            // Removed check mark. -belangeo
            // auto tick = getTickShape (1.0f);
            // g.fillPath (tick, tick.getTransformToScaleToFit (iconArea, true));
        }

        if (hasSubMenu) {
            auto const arrowH{ 0.6f * getPopupMenuFont().getAscent() };

            auto const x{ static_cast<float>(r.removeFromRight(static_cast<int>(arrowH)).getX()) };
            auto const halfH{ static_cast<float>(r.getCentreY()) };

            juce::Path p;
            p.addTriangle(x, halfH - arrowH * 0.5f, x, halfH + arrowH * 0.5f, x + arrowH * 0.6f, halfH);
            g.fillPath(p);
        }

        r.removeFromRight(3);
        g.drawFittedText(text, r, juce::Justification::centredLeft, 1);

        if (shortcutKeyText.isNotEmpty()) {
            auto f2{ font };
            f2.setHeight(f2.getHeight() * 0.75f);
            f2.setHorizontalScale(0.95f);
            g.setFont(f2);

            g.drawText(shortcutKeyText, r, juce::Justification::centredRight, true);
        }
    }
}

//==============================================================================
void GrisLookAndFeel::drawButtonBackground(juce::Graphics & g,
                                           juce::Button & button,
                                           juce::Colour const & /*backgroundColor*/,
                                           bool const isMouseOverButton,
                                           bool const isButtonDown)
{
    auto const width{ static_cast<float>(button.getWidth()) - 1.0f };
    auto const height{ static_cast<float>(button.getHeight()) - 1.0f };
    auto const cornerSize{ std::min(15.0f, std::min(width, height) * 0.45f) };
    auto const lineThickness{ cornerSize * 0.1f };
    auto const halfThickness{ lineThickness * 0.5f };

    juce::Path outline;
    juce::Colour color;

    outline.addRectangle(0.5f + halfThickness, 0.5f + halfThickness, width - lineThickness, height - lineThickness);

    if (isButtonDown) {
        color = this->mOnColorDown;
    } else if (button.getToggleState()) {
        color = this->mOnColor;
    } else {
        color = button.findColour(juce::TextButton::buttonColourId);
    }

    if (isMouseOverButton) {
        color = color.withAlpha(0.8f);
    }
    g.setColour(color);
    g.fillPath(outline);
}

//==============================================================================
void GrisLookAndFeel::drawTickBox(juce::Graphics & g,
                                  juce::Component & component,
                                  float const x,
                                  float const y,
                                  float const w,
                                  float const h,
                                  bool const ticked,
                                  bool /*isEnabled*/,
                                  bool /*isMouseOverButton*/,
                                  bool /*isButtonDown*/)
{
    auto const boxSize{ w * 0.8f };
    juce::Rectangle<float> const r{ x, y + (h - boxSize) * 0.5f, boxSize, boxSize };

    if (ticked) {
        auto color{ mOnColor };

        if (component.isMouseOver()) {
            color = mOnColorOver;
        }

        if (!component.isEnabled()) {
            color = mOnColor.withBrightness(0.3f);
        }
        g.setColour(color);
        g.fillRect(r);
    } else {
        auto color{ mOffColor };
        if (!component.isEnabled()) {
            color = mOffColor.withBrightness(0.3f);
        }
        g.setColour(color);
        g.fillRect(r);
    }

    if (component.isEnabled() && component.isMouseButtonDown()) {
        g.setColour(this->mOnColorDown);
        g.fillRect(r);
    }
}

//==============================================================================
void GrisLookAndFeel::drawLinearSliderThumb(juce::Graphics & g,
                                            int const x,
                                            int const y,
                                            int const width,
                                            int const height,
                                            float const sliderPos,
                                            float /*minSliderPos*/,
                                            float /*maxSliderPos*/,
                                            juce::Slider::SliderStyle const style,
                                            juce::Slider & slider)
{
    auto const sliderRadius{ static_cast<float>(getSliderThumbRadius(slider) - 1) };
    float kx;
    float ky;

    if (style == juce::Slider::LinearVertical) {
        kx = static_cast<float>(x) + static_cast<float>(width) * 0.5f;
        ky = sliderPos;
    } else {
        kx = sliderPos;
        ky = static_cast<float>(y) + static_cast<float>(height) * 0.5f;
    }
    const juce::Rectangle<float> r(kx - (sliderRadius / 2.0f), ky - sliderRadius, 6, static_cast<float>(height) * 2.0f);

    if (slider.isEnabled()) {
        auto color{ mOnColor };

        if (slider.isMouseOver()) {
            color = mOnColorOver;
        }
        if (slider.isMouseButtonDown()) {
            color = mOnColorDown;
        }
        g.setColour(color);
        g.fillRect(r);
    } else {
        g.setColour(mOffColor);
        g.fillRect(r);
    }
}

//==============================================================================
void GrisLookAndFeel::drawLinearSlider(juce::Graphics & g,
                                       int const x,
                                       int const y,
                                       int const width,
                                       int const height,
                                       float const sliderPos,
                                       float const minSliderPos,
                                       float const maxSliderPos,
                                       juce::Slider::SliderStyle const style,
                                       juce::Slider & slider)
{
    drawLinearSliderBackground(g, x, y, width, height + 2, sliderPos, minSliderPos, maxSliderPos, style, slider);
    drawLinearSliderThumb(g, x, y, width, height, sliderPos, minSliderPos, maxSliderPos, style, slider);
}

//==============================================================================
void GrisLookAndFeel::drawLinearSliderBackground(juce::Graphics & g,
                                                 int const x,
                                                 int const y,
                                                 int const width,
                                                 int const height,
                                                 float,
                                                 float,
                                                 float,
                                                 const juce::Slider::SliderStyle,
                                                 juce::Slider & slider)
{
    auto const xFloat{ static_cast<float>(x) };
    auto const yFloat{ static_cast<float>(y) };
    auto const widthFloat{ static_cast<float>(width) };
    auto const heightFloat{ static_cast<float>(height) };

    auto const sliderRadius{ static_cast<float>(getSliderThumbRadius(slider)) - 5.0f };
    juce::Path on;
    juce::Path off;

    if (slider.isHorizontal()) {
        auto const iy{ yFloat + heightFloat * 0.5f - sliderRadius * 0.5f };
        juce::Rectangle<float> r{ xFloat - sliderRadius * 0.5f, iy, widthFloat + sliderRadius, sliderRadius };
        auto const onW{ r.getWidth() * static_cast<float>(slider.valueToProportionOfLength(slider.getValue())) };
        on.addRectangle(r.removeFromLeft(onW));
        off.addRectangle(r);
    } else {
        auto const ix{ xFloat + widthFloat * 0.5f - sliderRadius * 0.5f };
        juce::Rectangle<float> r{ ix,
                                  static_cast<float>(y) - sliderRadius * 0.5f,
                                  sliderRadius,
                                  static_cast<float>(height) + sliderRadius };
        auto const onH{ r.getHeight() * static_cast<float>(slider.valueToProportionOfLength(slider.getValue())) };
        on.addRectangle(r.removeFromBottom(onH));
        off.addRectangle(r);
    }

    if (slider.isEnabled()) {
        g.setColour(slider.findColour(juce::Slider::rotarySliderFillColourId));
        g.fillPath(on);
        g.setColour(slider.findColour(juce::Slider::trackColourId));
        g.fillPath(off);
    } else {
        g.setColour(mOffColor);
        g.fillPath(on);
        g.fillPath(off);
    }
}

//==============================================================================
void GrisLookAndFeel::fillTextEditorBackground(juce::Graphics & g, int /*width*/, int /*height*/, juce::TextEditor & t)
{
    if (t.isEnabled())
        g.setColour(this->mEditBackgroundColor);
    else
        g.setColour(this->mDisableBackgroundColor);
    g.fillAll();
}

//==============================================================================
void GrisLookAndFeel::drawTextEditorOutline(juce::Graphics & g, int const width, int const height, juce::TextEditor & t)
{
    if (t.hasKeyboardFocus(true)) {
        g.setColour(this->mOnColor);
        g.drawRect(0, 0, width, height);
    }
}

//==============================================================================
void GrisLookAndFeel::drawToggleButton(juce::Graphics & g,
                                       juce::ToggleButton & button,
                                       bool const isMouseOverButton,
                                       bool const isButtonDown)
{
    if (button.hasKeyboardFocus(true)) {
        g.setColour(button.findColour(juce::TextEditor::focusedOutlineColourId));
    }

    if (button.getButtonText().length() == 1) {
        drawTickBox(g,
                    button,
                    0,
                    0,
                    static_cast<float>(button.getWidth()),
                    static_cast<float>(button.getHeight()),
                    button.getToggleState(),
                    button.isEnabled(),
                    isMouseOverButton,
                    isButtonDown);
        g.setColour(button.findColour(juce::ToggleButton::textColourId));
        g.setFont(mFont);

        if (!button.isEnabled())
            g.setOpacity(0.5f);

        g.drawFittedText(button.getButtonText(),
                         -2,
                         2,
                         button.getWidth(),
                         button.getHeight(),
                         juce::Justification::centred,
                         10);
    } else {
        auto const fontSize{ std::min(15.0f, static_cast<float>(button.getHeight()) * 0.75f) };
        auto const tickWidth{ fontSize * 1.1f };

        drawTickBox(g,
                    button,
                    4.0f,
                    (static_cast<float>(button.getHeight()) - tickWidth) * 0.5f,
                    tickWidth,
                    tickWidth,
                    button.getToggleState(),
                    button.isEnabled(),
                    isMouseOverButton,
                    isButtonDown);

        g.setColour(button.findColour(juce::ToggleButton::textColourId));
        g.setFont(this->mFont);

        if (!button.isEnabled())
            g.setOpacity(0.5f);

        auto const textX{ static_cast<int>(tickWidth + 5) };

        g.drawFittedText(button.getButtonText(),
                         textX,
                         1,
                         button.getWidth() - (textX - 5),
                         button.getHeight(),
                         juce::Justification::centredLeft,
                         10);
    }
}

//==============================================================================
void GrisLookAndFeel::drawTabButton(juce::TabBarButton & button,
                                    juce::Graphics & g,
                                    bool const isMouseOver,
                                    bool const isMouseDown)
{
    auto const activeArea{ button.getActiveArea() };
    // activeArea.withHeight(18);
    auto const o{ button.getTabbedButtonBar().getOrientation() };
    auto const bkg{ button.getTabBackgroundColour() };

    if (button.getToggleState()) {
        g.setColour(bkg);
    } else {
        g.setColour(bkg.brighter(0.1f));
    }

    g.fillRect(activeArea);

    g.setColour(this->mWinBackgroundAndFieldColor);

    auto r{ activeArea };
    if (o != juce::TabbedButtonBar::TabsAtTop)
        g.fillRect(r.removeFromBottom(1));
    if (o != juce::TabbedButtonBar::TabsAtRight)
        g.fillRect(r.removeFromLeft(1));
    if (o != juce::TabbedButtonBar::TabsAtLeft)
        g.fillRect(r.removeFromRight(1));

    auto const alpha{ button.isEnabled() ? ((isMouseOver || isMouseDown) ? 1.0f : 0.8f) : 0.3f };
    auto const col{ bkg.contrasting().withMultipliedAlpha(alpha) };
    auto const area{ button.getTextArea().toFloat() };

    auto length{ area.getWidth() };
    auto depth{ area.getHeight() };

    if (button.getTabbedButtonBar().isVertical())
        std::swap(length, depth);

    juce::TextLayout textLayout;
    createTabTextLayout(button, length, depth, col, textLayout);
    textLayout.draw(g, juce::Rectangle<float>(length, depth));
}

//==============================================================================
void GrisLookAndFeel::createTabTextLayout(const juce::TabBarButton & button,
                                          float const length,
                                          float const depth,
                                          juce::Colour const color,
                                          juce::TextLayout & textLayout) const
{
    auto font{ this->mFont };
    font.setUnderline(button.hasKeyboardFocus(false));

    juce::AttributedString s;
    s.setJustification(juce::Justification::centred);
    s.append(button.getButtonText().trim(), font, color);

    textLayout.createLayout(s, length);
}

//==============================================================================
void GrisLookAndFeel::drawRotarySlider(juce::Graphics & g,
                                       int const x,
                                       int const y,
                                       int const width,
                                       int const height,
                                       float const sliderPos,
                                       float const rotaryStartAngle,
                                       float const rotaryEndAngle,
                                       juce::Slider & slider)
{
    auto const xFloat{ static_cast<float>(x) };
    auto const yFloat{ static_cast<float>(y) };
    auto const widthFloat{ static_cast<float>(width) };
    auto const heightFloat{ static_cast<float>(height) };

    auto const radius = std::min(widthFloat / 2.0f, heightFloat / 2.0f) - 2.0f;
    auto const centreX = xFloat + widthFloat * 0.5f;
    auto const centreY = (yFloat + heightFloat * 0.5f) + 6.0f;
    auto const rx = centreX - radius;
    auto const ry = centreY - radius;
    auto const rw = radius * 2.0f;
    auto const angle = rotaryStartAngle + sliderPos * (rotaryEndAngle - rotaryStartAngle);
    auto const isMouseOver = slider.isMouseOverOrDragging() && slider.isEnabled();

    if (slider.isEnabled()) {
        if (isMouseOver) {
            g.setColour(this->mOnColorOver);
        } else {
            g.setColour(this->mOnColor);
        }
    } else {
        g.setColour(this->mOffColor);
    }
    juce::Path filledArc;
    filledArc.addPieSegment(rx, ry, rw, rw, rotaryStartAngle, angle, 0.0);
    g.fillPath(filledArc);
    auto const lineThickness = std::min(15.0f, std::min(widthFloat, heightFloat) * 0.45f) * 0.1f;
    juce::Path outlineArc;
    outlineArc.addPieSegment(rx, ry, rw, rw, rotaryStartAngle, rotaryEndAngle, 0.0);
    g.strokePath(outlineArc, juce::PathStrokeType(lineThickness));
}

} // namespace gris
