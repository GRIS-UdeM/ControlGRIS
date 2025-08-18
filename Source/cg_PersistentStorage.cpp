#include "cg_PersistentStorage.h"

#include <JuceHeader.h>

static juce::String const SHOW_SECONDARY_SOURCE_DRAG_ERROR_MESSAGE_TAG{ "SHOW_SECONDARY_SOURCE_DRAG_ERROR_MESSAGE" };

static juce::PropertiesFile getPersistentStorage()
{
    juce::PropertiesFile::Options options{};
    options.applicationName = juce::String{ "ControlGRIS_" } + JUCE_STRINGIFY(JUCE_APP_VERSION);
    options.commonToAllUsers = false;
    options.filenameSuffix = "xml";
    options.folderName = "GRIS";
    options.storageFormat = juce::PropertiesFile::storeAsXML;
    options.ignoreCaseOfKeyNames = true;
    options.osxLibrarySubFolder = "Application Support";
    return juce::PropertiesFile{ options };
}

static bool getShowSecondarySourceDragErrorMessage()
{
    return false ; //getPersistentStorage ().getBoolValue (SHOW_SECONDARY_SOURCE_DRAG_ERROR_MESSAGE_TAG, true);
}

bool showSecondarySourceDragErrorMessage{ getShowSecondarySourceDragErrorMessage() };

void setShowSecondarySourceDragErrorMessage(bool const state)
{
    auto storage{ getPersistentStorage() };
    storage.setValue(SHOW_SECONDARY_SOURCE_DRAG_ERROR_MESSAGE_TAG, state);
    storage.saveIfNeeded();
}
