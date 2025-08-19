/**************************************************************************
 * Copyright 2018 UdeM - GRIS - Gaël Lane Lépine, Samuel Béland                          *
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
#include "cg_PersistentStorage.h"

namespace gris
{
//==============================================================================
PersistentStorage::PersistentStorage()
{
    juce::PropertiesFile::Options options{};
    options.applicationName = juce::String{ "ControlGRIS_" } + JUCE_STRINGIFY(JUCE_APP_VERSION);
    options.commonToAllUsers = false;
    options.filenameSuffix = "xml";
    options.folderName = "GRIS";
    options.storageFormat = juce::PropertiesFile::storeAsXML;
    options.ignoreCaseOfKeyNames = true;
    options.osxLibrarySubFolder = "Application Support";
    mPropertiesFile = std::make_unique<juce::PropertiesFile>(options);
}

//==============================================================================
bool PersistentStorage::getShowSecondarySourceDragErrorMessage()
{
    return mPropertiesFile->getBoolValue(SHOW_SECONDARY_SOURCE_DRAG_ERROR_MESSAGE_TAG, true);
}

//==============================================================================
void PersistentStorage::setShowSecondarySourceDragErrorMessage(bool state)
{
    mPropertiesFile->setValue(SHOW_SECONDARY_SOURCE_DRAG_ERROR_MESSAGE_TAG, state);
    mPropertiesFile->saveIfNeeded();
}
} // namespace gris
