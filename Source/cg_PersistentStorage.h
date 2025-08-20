/**************************************************************************
 * Copyright 2018 UdeM - GRIS - Gaël Lane Lépine, Samuel Béland                         *
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

extern bool showSecondarySourceDragErrorMessage;


void setShowSecondarySourceDragErrorMessage(bool state);

namespace gris
{
class PersistentStorage
{
public:
    //==============================================================================
    PersistentStorage();
    ~PersistentStorage() = default;

    PersistentStorage(PersistentStorage const &) = delete;
    PersistentStorage(PersistentStorage &&) = delete;

    PersistentStorage & operator=(PersistentStorage const &) = delete;
    PersistentStorage & operator=(PersistentStorage &&) = delete;
    //==============================================================================
    bool getShowSecondarySourceDragErrorMessage();
    void setShowSecondarySourceDragErrorMessage(bool state);

private:
    //==============================================================================
    std::unique_ptr<juce::PropertiesFile> mPropertiesFile;
    juce::String const SHOW_SECONDARY_SOURCE_DRAG_ERROR_MESSAGE_TAG{ "SHOW_SECONDARY_SOURCE_DRAG_ERROR_MESSAGE" };

    //==============================================================================
    JUCE_LEAK_DETECTOR(PersistentStorage)
};
} // namespace gris
