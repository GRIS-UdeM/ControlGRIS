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
    mOscOutputPluginIdLabel.setText("OSC plugin ID:", juce::NotificationType::dontSendNotification);
    addAndMakeVisible(&mOscOutputPluginIdLabel);

    mOscOutputPluginIdEditor.setLookAndFeel(&mGrisLookAndFeel);
    mOscOutputPluginIdEditor.setFont(grisLookAndFeel.getFont());
    mOscOutputPluginIdEditor.setText(juce::String(1));
    mOscOutputPluginIdEditor.setInputRestrictions(3, "0123456789");
    mOscOutputPluginIdEditor.onFocusLost = [this] {
        mOscOutputPluginIdEditor.moveCaretToEnd();
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
        unfocusAllComponents();
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

    mOscReceiveIpEditor.setLookAndFeel(&mGrisLookAndFeel);
    mOscReceiveIpEditor.setFont(grisLookAndFeel.getFont());
    mOscReceiveIpEditor.setText(juce::IPAddress::getLocalAddress().toString());
    // mOscReceiveIpEditor.setText("127.0.0.1");
    mOscReceiveIpEditor.setReadOnly(true);
    addAndMakeVisible(&mOscReceiveIpEditor);

    mOscReceivePortEditor.setLookAndFeel(&mGrisLookAndFeel);
    mOscReceivePortEditor.setFont(grisLookAndFeel.getFont());
    mOscReceivePortEditor.setText(juce::String(mLastOscReceivePort));
    mOscReceivePortEditor.setInputRestrictions(5, "0123456789");
    mOscReceivePortEditor.onFocusLost = [this] {
        mOscReceivePortEditor.moveCaretToEnd();
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
        unfocusAllComponents();
    };

    addAndMakeVisible(&mOscReceivePortEditor);

    mLastOscSendAddress = juce::String("192.168.1.100");
    mOscSendIpEditor.setLookAndFeel(&mGrisLookAndFeel);
    mOscSendIpEditor.setFont(grisLookAndFeel.getFont());
    mOscSendIpEditor.setText(mLastOscSendAddress);
    mOscSendIpEditor.setInputRestrictions(16, ".0123456789");
    mOscSendIpEditor.onFocusLost = [this] {
        mOscSendIpEditor.moveCaretToEnd();
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
        unfocusAllComponents();
    };

    addAndMakeVisible(&mOscSendIpEditor);

    mOscSendPortEditor.setLookAndFeel(&mGrisLookAndFeel);
    mOscSendPortEditor.setFont(grisLookAndFeel.getFont());
    mOscSendPortEditor.setText(juce::String(mLastOscSendPort));
    mOscSendPortEditor.setInputRestrictions(5, "0123456789");
    mOscSendPortEditor.onFocusLost = [this] {
        mOscSendPortEditor.moveCaretToEnd();
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
        unfocusAllComponents();
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
void SectionOscController::resized()
{
    mOscOutputPluginIdLabel.setBounds(5, 13, 135, 10);
    mOscOutputPluginIdEditor.setBounds(130, 10, 24, 15);

    mOscReceiveToggle.setBounds(5, 30, 200, 15);
    mOscReceivePortEditor.setBounds(130, 30, 40, 15);
    mOscReceiveIpEditor.setBounds(175, 30, 110, 15);

    mOscSendToggle.setBounds(5, 50, 200, 15);
    mOscSendPortEditor.setBounds(130, 50, 40, 15);
    mOscSendIpEditor.setBounds(175, 50, 110, 15);
}

} // namespace gris
