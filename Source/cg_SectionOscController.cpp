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

#include "cg_SectionOscController.hpp"

namespace gris
{
//==============================================================================
SectionOscController::SectionOscController(GrisLookAndFeel & grisLookAndFeel) : mGrisLookAndFeel(grisLookAndFeel)
{
    mOscOutputPluginIdLabel.setText("OSC output plugin ID:", juce::NotificationType::dontSendNotification);
    addAndMakeVisible(&mOscOutputPluginIdLabel);

    mOscOutputPluginIdEditor.setText(juce::String(1));
    mOscOutputPluginIdEditor.setInputRestrictions(3, "0123456789");
    mOscOutputPluginIdEditor.addListener(this);
    mOscOutputPluginIdEditor.onReturnKey = [this] { this->grabKeyboardFocus(); };
    mOscOutputPluginIdEditor.onFocusLost = [this] {
        if (!mOscOutputPluginIdEditor.isEmpty()) {
            mListeners.call([&](Listener & l) {
                l.oscOutputPluginIdChangedCallback(mOscOutputPluginIdEditor.getText().getIntValue());
            });
        } else {
            mListeners.call([&](Listener & l) {
                l.oscOutputPluginIdChangedCallback(1);
                mOscOutputPluginIdEditor.setText(juce::String(1));
            });
        }
    };

    addAndMakeVisible(&mOscOutputPluginIdEditor);

    mOscReceiveToggle.setButtonText("Receive on port : IP");
    mOscReceiveToggle.setExplicitFocusOrder(1);
    addAndMakeVisible(&mOscReceiveToggle);
    mOscReceiveToggle.onClick = [this] {
        mListeners.call([&](Listener & l) {
            l.oscInputConnectionChangedCallback(mOscReceiveToggle.getToggleState(),
                                                mOscReceivePortEditor.getText().getIntValue());
        });
    };

    mOscSendToggle.setButtonText("Send on port : IP");
    addAndMakeVisible(&mOscSendToggle);
    mOscSendToggle.onClick = [this] {
        mListeners.call([&](Listener & l) {
            l.oscOutputConnectionChangedCallback(mOscSendToggle.getToggleState(),
                                                 mOscSendIpEditor.getText(),
                                                 mOscSendPortEditor.getText().getIntValue());
        });
    };

    mOscReceiveIpEditor.setText(juce::IPAddress::getLocalAddress().toString());
    // mOscReceiveIpEditor.setText("127.0.0.1");
    mOscReceiveIpEditor.setReadOnly(true);
    addAndMakeVisible(&mOscReceiveIpEditor);

    mOscReceivePortEditor.setText(juce::String(mLastOscReceivePort));
    mOscReceivePortEditor.setInputRestrictions(5, "0123456789");
    mOscReceivePortEditor.addListener(this);
    mOscReceivePortEditor.onReturnKey = [this] { this->grabKeyboardFocus(); };
    mOscReceivePortEditor.onFocusLost = [this] {
        if (!mOscReceivePortEditor.isEmpty()) {
            mListeners.call([&](Listener & l) {
                l.oscInputConnectionChangedCallback(mOscReceiveToggle.getToggleState(),
                                                    mOscReceivePortEditor.getText().getIntValue());
            });
        } else {
            mListeners.call([&](Listener & l) {
                l.oscInputConnectionChangedCallback(mOscReceiveToggle.getToggleState(), mLastOscReceivePort);
                mOscReceivePortEditor.setText(juce::String(mLastOscReceivePort));
            });
        }
    };

    addAndMakeVisible(&mOscReceivePortEditor);

    mLastOscSendAddress = juce::String("192.168.1.100");
    mOscSendIpEditor.setText(mLastOscSendAddress);
    mOscSendIpEditor.setInputRestrictions(16, ".0123456789");
    mOscSendIpEditor.addListener(this);
    mOscSendIpEditor.onReturnKey = [this] { this->grabKeyboardFocus(); };
    mOscSendIpEditor.onFocusLost = [this] {
        if (!mOscSendIpEditor.isEmpty()) {
            mListeners.call([&](Listener & l) {
                l.oscOutputConnectionChangedCallback(mOscSendToggle.getToggleState(),
                                                     mOscSendIpEditor.getText(),
                                                     mOscSendPortEditor.getText().getIntValue());
            });
        } else {
            mListeners.call([&](Listener & l) {
                l.oscOutputConnectionChangedCallback(mOscSendToggle.getToggleState(),
                                                     mLastOscSendAddress,
                                                     mOscSendPortEditor.getText().getIntValue());
                mOscSendIpEditor.setText(juce::String(mLastOscSendAddress));
            });
        }
    };

    addAndMakeVisible(&mOscSendIpEditor);

    mOscSendPortEditor.setText(juce::String(mLastOscSendPort));
    mOscSendPortEditor.setInputRestrictions(5, "0123456789");
    mOscSendPortEditor.addListener(this);
    mOscSendPortEditor.onReturnKey = [this] { this->grabKeyboardFocus(); };
    mOscSendPortEditor.onFocusLost = [this] {
        if (!mOscSendPortEditor.isEmpty()) {
            mListeners.call([&](Listener & l) {
                l.oscOutputConnectionChangedCallback(mOscSendToggle.getToggleState(),
                                                     mOscSendIpEditor.getText(),
                                                     mOscSendPortEditor.getText().getIntValue());
            });
        } else {
            mListeners.call([&](Listener & l) {
                l.oscOutputConnectionChangedCallback(mOscSendToggle.getToggleState(),
                                                     mOscSendIpEditor.getText(),
                                                     mLastOscSendPort);
                mOscSendPortEditor.setText(juce::String(mLastOscSendPort));
            });
        }
    };

    addAndMakeVisible(&mOscSendPortEditor);
}

//==============================================================================
void SectionOscController::setOscReceiveInputPort(int const port)
{
    mLastOscReceivePort = port;
    mOscReceivePortEditor.setText(juce::String(port));
}

//==============================================================================
void SectionOscController::setOscSendOutputPort(int const port)
{
    mLastOscSendPort = port;
    mOscSendPortEditor.setText(juce::String{ port });
}

//==============================================================================
void SectionOscController::setOscReceiveToggleState(bool const state)
{
    mOscReceiveToggle.setToggleState(state, juce::NotificationType::dontSendNotification);
}

//==============================================================================
void SectionOscController::setOscSendToggleState(bool const state)
{
    mOscSendToggle.setToggleState(state, juce::NotificationType::dontSendNotification);
}

//==============================================================================
void SectionOscController::setOscSendOutputAddress(juce::String const & address)
{
    mLastOscSendAddress = address;
    mOscSendIpEditor.setText(address);
}

//==============================================================================
void SectionOscController::paint(juce::Graphics & g)
{
    g.fillAll(mGrisLookAndFeel.findColour(juce::ResizableWindow::backgroundColourId));
}

//==============================================================================
void SectionOscController::resized()
{
    mOscOutputPluginIdLabel.setBounds(5, 10, 135, 20);
    mOscOutputPluginIdEditor.setBounds(140, 10, 70, 20);

    mOscReceiveToggle.setBounds(255, 10, 200, 20);
    mOscReceivePortEditor.setBounds(400, 10, 60, 20);
    mOscReceiveIpEditor.setBounds(470, 10, 120, 20);

    mOscSendToggle.setBounds(255, 35, 200, 20);
    mOscSendPortEditor.setBounds(400, 35, 60, 20);
    mOscSendIpEditor.setBounds(470, 35, 120, 20);
}

} // namespace gris