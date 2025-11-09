/**************************************************************************
 * Copyright 2024 UdeM - GRIS - Gaël LANE LÉPINE                         *
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
 * <https://www.gnu.org/licenses/>.                                       *
 *************************************************************************/

#include "cg_NumSlider.h"
#include "cg_ControlGrisLookAndFeel.hpp"

namespace gris
{

//==============================================================================
NumSlider::NumSlider(GrisLookAndFeel & grisLookAndFeel) : mGrisLookAndFeel(grisLookAndFeel)
{
    setLookAndFeel(&grisLookAndFeel);
    setTitle("NumSlider");
    setRange(0.0, 1.0, 0.001);
    setSliderStyle(juce::Slider::LinearBar);
    setSliderSnapsToMousePosition(false);
    setNumDecimalPlacesToDisplay(3);
    setScrollWheelEnabled(true);
    setTextBoxStyle(juce::Slider::TextEntryBoxPosition::NoTextBox, false, getWidth(), getHeight());
    setTextBoxIsEditable(false);
}

//==============================================================================
void NumSlider::mouseWheelMove(const juce::MouseEvent & /*event*/, const juce::MouseWheelDetails & wheel)
{
    mLastValue = getValue();
    juce::Time currentTime = juce::Time::getCurrentTime();
    auto timeDiff = static_cast<double>((currentTime - mLastTime).inMilliseconds());
    if (timeDiff <= 0.0001)
        timeDiff = 1.0;
    double valueDiff = wheel.deltaY * getInterval();
    double velocity = valueDiff / timeDiff * 1000;
    double newValue = mLastValue - velocity;
    newValue = std::clamp(newValue, getRange().getStart(), getRange().getEnd());

    setValue(newValue);

    mLastTime = currentTime;
    mLastValue = newValue;
}

//==============================================================================
void NumSlider::paint(juce::Graphics & g)
{
    auto rangeVals{ getRange() };
    auto val{ getValue() };
    auto bounds{ getLocalBounds() };

    auto onColor{ mGrisLookAndFeel.getOnColor() };
    auto lightColor{ mGrisLookAndFeel.getLightColor() };

    if (!isEnabled()) {
        onColor = onColor.withBrightness(0.8f);
        lightColor = lightColor.withBrightness(0.8f);
    }

    if (val > rangeVals.getStart()) {
        juce::Rectangle<float> drawRec;
        g.setColour(onColor);
        double xLimitProportion{};

        if (rangeVals.getStart() < 0) {
            xLimitProportion = (val + rangeVals.getLength() / 2) / rangeVals.getLength();
        } else if (rangeVals.getStart() > 0) {
            xLimitProportion = (val - rangeVals.getStart()) / rangeVals.getLength();
        } else {
            xLimitProportion = val / rangeVals.getLength();
        }

        drawRec = juce::Rectangle<float>{ 0,
                                          0,
                                          static_cast<float>(bounds.getWidth() * xLimitProportion),
                                          static_cast<float>(bounds.getHeight()) };
        g.fillRect(drawRec);

        g.setColour(lightColor);
        drawRec = juce::Rectangle<float>{ static_cast<float>(bounds.getWidth() * xLimitProportion),
                                          0,
                                          static_cast<float>(bounds.getWidth()),
                                          static_cast<float>(bounds.getHeight()) };
        g.fillRect(drawRec);
    } else {
        g.setColour(lightColor);
        g.fillRect(bounds);
    }

    g.setColour(mGrisLookAndFeel.getDarkColor());
    g.setFont(mGrisLookAndFeel.getNumSliderFont());
    g.drawText(getTextFromValue(getValue()), bounds.translated(0, 1), juce::Justification::centred);
}

//==============================================================================
void NumSlider::valueChanged()
{
    mLastValue = getValue();

    if (mLastValue >= 10000 && getNumDecimalPlacesToDisplay() > 0) {
        setNumDecimalPlacesToDisplay(0);
    } else if (mLastValue < 10000 && getNumDecimalPlacesToDisplay() == 0) {
        setNumDecimalPlacesToDisplay(mDefaultNumDecimalToDisplay);
    }
}

//==============================================================================
void NumSlider::mouseDown(const juce::MouseEvent & event)
{
    mMouseDragStartPos = event.getMouseDownPosition();
    mMouseDiffFromStartY = 0;
}

//==============================================================================
void NumSlider::mouseDrag(const juce::MouseEvent & event)
{
    const auto isShiftDown{ event.mods.isShiftDown() };
    const auto mouseDragCurrentPos{ event.getPosition() };
    const auto mouseDiffFromStartY{ mMouseDragStartPos.getY() - mouseDragCurrentPos.getY() };
    const auto mouseDiffDragY{ mouseDiffFromStartY - mMouseDiffFromStartY };
    const auto range{ getRange() };
    const auto smallestSliderVal{ std::pow(10, -1 * (getNumDecimalPlacesToDisplay())) };
    const auto increment{ isShiftDown ? smallestSliderVal * 0.1f * std::abs(mouseDiffDragY)
                                    : range.getLength() / 100 * std::abs(mouseDiffDragY) };

    if (!isShiftDown && mIsFineDragging) {
        stopFineClickDragging(event);
    } else if (isShiftDown && !mIsFineDragging) {
        startFineClickDragging(event);
    }

    if (mouseDiffDragY > 0) {
        if (increment < smallestSliderVal && mIncrementBuffer < smallestSliderVal) {
            mIncrementBuffer += increment;
            if (mIncrementBuffer >= smallestSliderVal) {
                double newValue = getValue() + mIncrementBuffer;
                setValue(juce::jlimit(getMinimum(), getMaximum(), newValue), juce::sendNotificationSync);
                mIncrementBuffer = 0.0f;
            }
        } else {
            double newValue = getValue() + increment;
            setValue(juce::jlimit(getMinimum(), getMaximum(), newValue), juce::sendNotificationSync);
        }
        mMouseDiffFromStartY = mouseDiffFromStartY;
    } else if (mouseDiffDragY < 0) {
        if (increment < smallestSliderVal && mIncrementBuffer < smallestSliderVal) {
            mIncrementBuffer += increment;
            if (mIncrementBuffer >= smallestSliderVal) {
                double newValue = getValue() - mIncrementBuffer;
                setValue(juce::jlimit(getMinimum(), getMaximum(), newValue), juce::sendNotificationSync);
                mIncrementBuffer = 0.0f;
            }
        } else {
            double newValue = getValue() - increment;
            setValue(juce::jlimit(getMinimum(), getMaximum(), newValue), juce::sendNotificationSync);
        }
        mMouseDiffFromStartY = mouseDiffFromStartY;
    }
}

//==============================================================================
void NumSlider::mouseUp(const juce::MouseEvent & event)
{
    if (event.mods.isAltDown() && event.mods.isLeftButtonDown()) {
        setValue(mDefaultReturnValue);
    }

    if (event.mods.isShiftDown() && mIsFineDragging) {
        stopFineClickDragging(event);
    }
}

//==============================================================================
void NumSlider::mouseDoubleClick(const juce::MouseEvent & /*event*/)
{
    auto sliderEditor{ std::make_unique<juce::TextEditor>("SliderEditor") };
    sliderEditor->setLookAndFeel(&mGrisLookAndFeel);
    sliderEditor->setJustification(juce::Justification::centred);
    sliderEditor->addListener(this);
    sliderEditor->setMultiLine(false);
    sliderEditor->setSize(60, 20);
    if (getRange().getStart() < 0) {
        sliderEditor->setInputRestrictions(5, "0123456789,.-");
    } else {
        sliderEditor->setInputRestrictions(5, "0123456789,.");
    }
    sliderEditor->setText(juce::String(getValue()), false);
    sliderEditor->selectAll();

    auto & box = juce::CallOutBox::launchAsynchronously(std::move(sliderEditor), getScreenBounds(), nullptr);
    box.setLookAndFeel(&mGrisLookAndFeel);
}

//==============================================================================
void NumSlider::setDefaultNumDecimalPlacesToDisplay(int numDec)
{
    mDefaultNumDecimalToDisplay = numDec;
    setNumDecimalPlacesToDisplay(mDefaultNumDecimalToDisplay);
}

void NumSlider::setDefaultReturnValue(double value)
{
    mDefaultReturnValue = value;
}

//==============================================================================
void NumSlider::textEditorReturnKeyPressed(juce::TextEditor & ed)
{
    if (!ed.getText().isEmpty()) {
        auto val = ed.getText().replace(",", ".").getDoubleValue();
        mLastValue = val;
        setValue(val);
    }

    auto callOutBox = dynamic_cast<juce::CallOutBox *>(ed.getParentComponent());

    if (callOutBox != nullptr) {
        callOutBox->dismiss();
    }
}

//==============================================================================
void NumSlider::textEditorEscapeKeyPressed(juce::TextEditor & ed)
{
    auto callOutBox = dynamic_cast<juce::CallOutBox *>(ed.getParentComponent());

    if (callOutBox != nullptr) {
        callOutBox->dismiss();
    }
}

//==============================================================================
void NumSlider::startFineClickDragging(const juce::MouseEvent & event)
{
    setMouseCursor(juce::MouseCursor::NoCursor);
    event.source.enableUnboundedMouseMovement(true);
    mMouseScreenPos = event.source.getScreenPosition();
    mIsFineDragging = true;
}

//==============================================================================
void NumSlider::stopFineClickDragging(const juce::MouseEvent & event)
{
    juce::MouseInputSource inputSource = juce::MouseInputSource(event.source);
    inputSource.enableUnboundedMouseMovement(false);
    inputSource.setScreenPosition(mMouseScreenPos);
    setMouseCursor(juce::MouseCursor::NormalCursor);
    mIsFineDragging = false;
}

} // namespace gris
