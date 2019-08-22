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
#include "PluginProcessor.h"
#include "PluginEditor.h"
#include "ControlGrisConstants.h"

ControlGrisAudioProcessorEditor::ControlGrisAudioProcessorEditor (ControlGrisAudioProcessor& p,
                                                                  AudioProcessorValueTreeState& vts,
                                                                  AutomationManager& automan,
                                                                  AutomationManager& automanAlt)
    : AudioProcessorEditor (&p),
      processor (p),
      valueTreeState (vts), 
      automationManager (automan), 
      automationManagerAlt (automanAlt), 
      mainField (automan),
      elevationField (automanAlt)
{ 
    setLookAndFeel(&grisLookAndFeel);

    m_fixedSourcesWindowVisible = false;
    m_selectedSource = 0;

    // Set up the interface.
    //----------------------
    mainBanner.setLookAndFeel(&grisLookAndFeel);
    mainBanner.setText("Azimuth - Elevation", NotificationType::dontSendNotification);
    addAndMakeVisible(&mainBanner);

    elevationBanner.setLookAndFeel(&grisLookAndFeel);
    elevationBanner.setText("Elevation", NotificationType::dontSendNotification);
    addAndMakeVisible(&elevationBanner);

    trajectoryBanner.setLookAndFeel(&grisLookAndFeel);
    trajectoryBanner.setText("Trajectories", NotificationType::dontSendNotification);
    addAndMakeVisible(&trajectoryBanner);

    settingsBanner.setLookAndFeel(&grisLookAndFeel);
    settingsBanner.setText("Configuration", NotificationType::dontSendNotification);
    addAndMakeVisible(&settingsBanner);

    mainField.setLookAndFeel(&grisLookAndFeel);
    mainField.addListener(this);
    addAndMakeVisible(&mainField);

    elevationField.setLookAndFeel(&grisLookAndFeel);
    elevationField.addListener(this);
    addAndMakeVisible(&elevationField);

    parametersBox.setLookAndFeel(&grisLookAndFeel);
    parametersBox.addListener(this);
    addAndMakeVisible(&parametersBox);

    trajectoryBox.setLookAndFeel(&grisLookAndFeel);
    trajectoryBox.addListener(this);
    addAndMakeVisible(trajectoryBox);
    trajectoryBox.setSourceLink(automationManager.getSourceLink());
    trajectoryBox.setSourceLinkAlt(automationManagerAlt.getSourceLink());

    settingsBox.setLookAndFeel(&grisLookAndFeel);
    settingsBox.addListener(this);

    sourceBox.setLookAndFeel(&grisLookAndFeel);
    sourceBox.addListener(this);

    interfaceBox.setLookAndFeel(&grisLookAndFeel);

    Colour bg = grisLookAndFeel.findColour (ResizableWindow::backgroundColourId);

    configurationComponent.setLookAndFeel(&grisLookAndFeel);
    configurationComponent.setColour(TabbedComponent::backgroundColourId, bg);
    configurationComponent.addTab ("Settings", bg, &settingsBox, false);
    configurationComponent.addTab ("Source", bg, &sourceBox, false);
    configurationComponent.addTab ("Controllers", bg, &interfaceBox, false);
    addAndMakeVisible(configurationComponent);

    fixedPositionEditor.setLookAndFeel(&grisLookAndFeel);
    fixedPositionEditor.loadData(processor.getFixedPositionData());
    fixedPositionEditor.addListener(this);
    addChildComponent(&fixedPositionEditor);

    // Add sources to the fields.
    //---------------------------
    mainField.setSources(processor.getSources(), processor.getNumberOfSources());
    elevationField.setSources(processor.getSources(), processor.getNumberOfSources());

    parametersBox.setSelectedSource(&processor.getSources()[m_selectedSource]);
    processor.setSelectedSourceId(m_selectedSource);

    // Manage dynamic window size of the plugin.
    //------------------------------------------
    setResizeLimits(FIELD_WIDTH, FIELD_WIDTH + 20, 1800, 1300);

    lastUIWidth .referTo (processor.parameters.state.getChildWithName ("uiState").getPropertyAsValue ("width",  nullptr));
    lastUIHeight.referTo (processor.parameters.state.getChildWithName ("uiState").getPropertyAsValue ("height", nullptr));

    // set our component's initial size to be the last one that was stored in the filter's settings
    setSize(lastUIWidth.getValue(), lastUIHeight.getValue());

    lastUIWidth.addListener(this);
    lastUIHeight.addListener(this);

    // Load the last saved state of the plugin.
    //-----------------------------------------
    setPluginState();
}

ControlGrisAudioProcessorEditor::~ControlGrisAudioProcessorEditor() {
    configurationComponent.setLookAndFeel(nullptr);
    setLookAndFeel(nullptr);
}

void ControlGrisAudioProcessorEditor::setPluginState() {
    // Set state for the link buttons.
    //--------------------------------
    parametersBox.setLinkState(SOURCE_ID_AZIMUTH_SPAN, valueTreeState.state.getProperty("azimuthSpanLink", false));
    parametersBox.setLinkState(SOURCE_ID_ELEVATION_SPAN, valueTreeState.state.getProperty("elevationSpanLink", false));

    // Set global settings values.
    //----------------------------
    settingsBoxOscFormatChanged(processor.getOscFormat());
    settingsBoxOscPortNumberChanged(processor.getOscPortNumber());
    settingsBoxOscActivated(processor.getOscConnected());
    settingsBoxNumberOfSourcesChanged(processor.getNumberOfSources());
    settingsBoxFirstSourceIdChanged(processor.getFirstSourceId());

    // Set state for trajectory box persistent values.
    //------------------------------------------------
    trajectoryBox.setDurationUnit(valueTreeState.state.getProperty("durationUnit", 1));
    trajectoryBox.setCycleDuration(valueTreeState.state.getProperty("cycleDuration", 5.0));

    // Update the interface.
    //----------------------
    parametersBox.setSelectedSource(&processor.getSources()[m_selectedSource]);
    mainField.setSelectedSource(m_selectedSource);
    elevationField.setSelectedSource(m_selectedSource);
    processor.setSelectedSourceId(m_selectedSource);
    sourceBox.updateSelectedSource(&processor.getSources()[m_selectedSource], m_selectedSource, processor.getOscFormat());
}

void ControlGrisAudioProcessorEditor::updateSourceLinkCombo(int value) {
    trajectoryBox.sourceLinkCombo.setSelectedId(value, NotificationType::dontSendNotification);
}

void ControlGrisAudioProcessorEditor::updateSourceLinkAltCombo(int value) {
    trajectoryBox.sourceLinkAltCombo.setSelectedId(value, NotificationType::dontSendNotification);
}

// Value::Listener callback. Called when the stored window size changes.
//----------------------------------------------------------------------
void ControlGrisAudioProcessorEditor::valueChanged (Value&) {
    setSize (lastUIWidth.getValue(), lastUIHeight.getValue());
}

// SettingsBoxComponent::Listener callbacks.
//------------------------------------------
void ControlGrisAudioProcessorEditor::settingsBoxOscFormatChanged(SPAT_MODE_ENUM mode) {
    settingsBox.setOscFormat(mode);
    processor.setOscFormat(mode);
    bool selectionIsLBAP = mode == SPAT_MODE_LBAP;
    parametersBox.setDistanceEnabled(selectionIsLBAP);
    mainField.setSpatMode(mode);
    trajectoryBox.setSpatMode(mode);
    repaint();
    resized();
}

void ControlGrisAudioProcessorEditor::settingsBoxOscPortNumberChanged(int oscPort) {
    processor.setOscPortNumber(oscPort);
    settingsBox.setOscPortNumber(oscPort);
}

void ControlGrisAudioProcessorEditor::settingsBoxOscActivated(bool state) {
    processor.handleOscConnection(state);
    settingsBox.setActivateButtonState(processor.getOscConnected());
}

void ControlGrisAudioProcessorEditor::settingsBoxNumberOfSourcesChanged(int numOfSources) {
    m_selectedSource = 0;
    processor.setNumberOfSources(numOfSources);
    processor.setSelectedSourceId(m_selectedSource);
    settingsBox.setNumberOfSources(numOfSources);
    parametersBox.setSelectedSource(&processor.getSources()[m_selectedSource]);
    mainField.setSources(processor.getSources(), numOfSources);
    elevationField.setSources(processor.getSources(), numOfSources);
    sourceBox.setNumberOfSources(numOfSources, processor.getFirstSourceId());
}

void ControlGrisAudioProcessorEditor::settingsBoxFirstSourceIdChanged(int firstSourceId) {
    processor.setFirstSourceId(firstSourceId);
    settingsBox.setFirstSourceId(firstSourceId);
    parametersBox.setSelectedSource(&processor.getSources()[m_selectedSource]);
    sourceBox.setNumberOfSources(processor.getNumberOfSources(), firstSourceId);

    mainField.repaint();
    if (processor.getOscFormat() == SPAT_MODE_LBAP)
        elevationField.repaint();
}

// SourceBoxComponent::Listener callbacks.
//----------------------------------------
void ControlGrisAudioProcessorEditor::sourceBoxSelectionChanged(int sourceNum) {
    m_selectedSource = sourceNum;

    parametersBox.setSelectedSource(&processor.getSources()[m_selectedSource]);
    mainField.setSelectedSource(m_selectedSource);
    elevationField.setSelectedSource(m_selectedSource);
    processor.setSelectedSourceId(m_selectedSource);
    sourceBox.updateSelectedSource(&processor.getSources()[m_selectedSource], m_selectedSource, processor.getOscFormat());
}

void ControlGrisAudioProcessorEditor::sourceBoxPlacementChanged(int value) {
    int numOfSources = processor.getNumberOfSources();
    const float azims2[2] = {-90.0f, 90.0f};
    const float azims4[4] = {-45.0f, 45.0f, -135.0f, 135.0f};
    const float azims6[6] = {-30.0f, 30.0f, -90.0f, 90.0f, -150.0f, 150.0f};
    const float azims8[8] = {-22.5f, 22.5f, -67.5f, 67.5f, -112.5f, 112.5f, -157.5f, 157.5f};

    float offset = 360.0f / numOfSources / 2.0f;
    float distance = processor.getOscFormat() == SPAT_MODE_LBAP ? 0.7f : 1.0f;

    switch(value) {
        case SOURCE_PLACEMENT_LEFT_ALTERNATE:
            for (int i = 0; i < numOfSources; i++) {
                if (numOfSources <= 2)
                    processor.getSources()[i].setCoordinates(-azims2[i], 0.0f, distance);
                else if (numOfSources <= 4)
                    processor.getSources()[i].setCoordinates(-azims4[i], 0.0f, distance);
                else if (numOfSources <= 6)
                    processor.getSources()[i].setCoordinates(-azims6[i], 0.0f, distance);
                else
                    processor.getSources()[i].setCoordinates(-azims8[i], 0.0f, distance);
            }
            break;
        case SOURCE_PLACEMENT_RIGHT_ALTERNATE:
            for (int i = 0; i < numOfSources; i++) {
                if (numOfSources <= 2)
                    processor.getSources()[i].setCoordinates(azims2[i], 0.0f, distance);
                else if (numOfSources <= 4)
                    processor.getSources()[i].setCoordinates(azims4[i], 0.0f, distance);
                else if (numOfSources <= 6)
                    processor.getSources()[i].setCoordinates(azims6[i], 0.0f, distance);
                else
                    processor.getSources()[i].setCoordinates(azims8[i], 0.0f, distance);
            }
            break;
        case SOURCE_PLACEMENT_LEFT_CLOCKWISE:
            for (int i = 0; i < numOfSources; i++) {
                processor.getSources()[i].setCoordinates(360.0f / numOfSources * -i + offset, 0.0f, distance);
            }
            break;
        case SOURCE_PLACEMENT_LEFT_COUNTER_CLOCKWISE:
            for (int i = 0; i < numOfSources; i++) {
                processor.getSources()[i].setCoordinates(360.0f / numOfSources * i + offset, 0.0f, distance);
            }
            break;
        case SOURCE_PLACEMENT_RIGHT_CLOCKWISE:
            for (int i = 0; i < numOfSources; i++) {
                processor.getSources()[i].setCoordinates(360.0f / numOfSources * -i - offset, 0.0f, distance);
            }
            break;
        case SOURCE_PLACEMENT_RIGHT_COUNTER_CLOCKWISE:
            for (int i = 0; i < numOfSources; i++) {
                processor.getSources()[i].setCoordinates(360.0f / numOfSources * i - offset, 0.0f, distance);
            }
            break;
        case SOURCE_PLACEMENT_TOP_CLOCKWISE:
            for (int i = 0; i < numOfSources; i++) {
                processor.getSources()[i].setCoordinates(360.0f / numOfSources * -i, 0.0f, distance);
            }
            break;
        case SOURCE_PLACEMENT_TOP_COUNTER_CLOCKWISE:
            for (int i = 0; i < numOfSources; i++) {
                processor.getSources()[i].setCoordinates(360.0f / numOfSources * i, 0.0f, distance);
            }
            break;
    }

    for (int i = 0; i < numOfSources; i++) {
        processor.setSourceParameterValue(i, SOURCE_ID_AZIMUTH, processor.getSources()[i].getNormalizedAzimuth());
        processor.setSourceParameterValue(i, SOURCE_ID_ELEVATION, processor.getSources()[i].getNormalizedElevation());
        processor.setSourceParameterValue(i, SOURCE_ID_DISTANCE, processor.getSources()[i].getDistance());
    }

    sourceBox.updateSelectedSource(&processor.getSources()[m_selectedSource], m_selectedSource, processor.getOscFormat());

    if (automationManager.getActivateState() || automationManagerAlt.getActivateState()) {
        processor.addNewFixedPosition();
    }

    repaint();
}

void ControlGrisAudioProcessorEditor::sourceBoxPositionChanged(int sourceNum, float angle, float rayLen) {
    if (processor.getOscFormat() == SPAT_MODE_LBAP) {
        float currentElevation = processor.getSources()[sourceNum].getElevation();
        processor.getSources()[sourceNum].setCoordinates(angle, currentElevation, rayLen);
    } else {
        processor.getSources()[sourceNum].setCoordinates(angle, 90.0f - (rayLen * 90.0f), 1.0f);
    }

    if (automationManager.getActivateState() || automationManagerAlt.getActivateState()) {
        processor.addNewFixedPosition();
    }

    repaint();
}

// ParametersBoxComponent::Listener callbacks.
//--------------------------------------------
void ControlGrisAudioProcessorEditor::parametersBoxLinkChanged(int parameterId, bool value) {
    StringArray parameterNames({"", "", "", "", "", "azimuthSpanLink", "elevationSpanLink"});
    valueTreeState.state.setProperty(parameterNames[parameterId], value, nullptr);
}

void ControlGrisAudioProcessorEditor::parametersBoxParameterChanged(int parameterId, double value) {
    processor.setSourceParameterValue(m_selectedSource, parameterId, value);

    mainField.repaint();
    if (processor.getOscFormat() == SPAT_MODE_LBAP)
        elevationField.repaint();
}

void ControlGrisAudioProcessorEditor::parametersBoxSelectedSourceClicked() {
    m_selectedSource = (m_selectedSource + 1) % processor.getNumberOfSources();
    parametersBox.setSelectedSource(&processor.getSources()[m_selectedSource]);
    mainField.setSelectedSource(m_selectedSource);
    elevationField.setSelectedSource(m_selectedSource);
    processor.setSelectedSourceId(m_selectedSource);
    sourceBox.updateSelectedSource(&processor.getSources()[m_selectedSource], m_selectedSource, processor.getOscFormat());
}

// TrajectoryBoxComponent::Listener callbacks.
//--------------------------------------------
void ControlGrisAudioProcessorEditor::trajectoryBoxSourceLinkChanged(int value) {
    automationManager.setSourceLink(value);
    automationManager.fixSourcePosition();

    processor.onSourceLinkChanged(value);
    if (automationManager.getActivateState()) {
        processor.addNewFixedPosition();
    }

    valueTreeState.getParameterAsValue("sourceLink").setValue((value - 1) / 5.f);

    mainField.repaint();
}

void ControlGrisAudioProcessorEditor::trajectoryBoxSourceLinkAltChanged(int value) {
    automationManagerAlt.setSourceLink(value);
    automationManagerAlt.fixSourcePosition();

    processor.onSourceLinkAltChanged(value);
    if (automationManagerAlt.getActivateState()) {
        processor.addNewFixedPosition();
    }

    valueTreeState.getParameterAsValue("sourceLinkAlt").setValue((value - 1) / 4.f);

    elevationField.repaint();
}

void ControlGrisAudioProcessorEditor::trajectoryBoxTrajectoryTypeChanged(int value) {
    automationManager.setDrawingType(value);
    mainField.repaint();
}

void ControlGrisAudioProcessorEditor::trajectoryBoxTrajectoryTypeAltChanged(int value) {
    automationManagerAlt.setDrawingTypeAlt(value);
    elevationField.repaint();
}

void ControlGrisAudioProcessorEditor::trajectoryBoxDurationChanged(double duration, int mode) {
    processor.parameters.state.setProperty("cycleDuration", duration, nullptr);
    processor.parameters.state.setProperty("durationUnit", mode, nullptr);
    double dur = duration;
    if (mode == 2) {
        dur = duration * 60.0 / processor.getBPM();
    }
    automationManager.setPlaybackDuration(dur);
    automationManagerAlt.setPlaybackDuration(dur);
}

void ControlGrisAudioProcessorEditor::trajectoryBoxActivateChanged(bool value) {
    automationManager.setActivateState(value);
}

void ControlGrisAudioProcessorEditor::trajectoryBoxActivateAltChanged(bool value) {
    automationManagerAlt.setActivateState(value);
}

void ControlGrisAudioProcessorEditor::trajectoryBoxEditFixedSourceButtonClicked() {
    fixedPositionEditor.loadData(processor.getFixedPositionData());
    m_fixedSourcesWindowVisible = ! m_fixedSourcesWindowVisible;
    resized();
}

void ControlGrisAudioProcessorEditor::trajectoryBoxFixSourceButtonClicked() {
    int numOfSources = processor.getNumberOfSources();
    int link = automationManager.getSourceLink();
    int linkAlt = automationManagerAlt.getSourceLink();

    automationManager.fixSourcePosition();
    automationManagerAlt.fixSourcePosition();

    bool shouldBeFixed = link != SOURCE_LINK_INDEPENDANT || linkAlt != SOURCE_LINK_ALT_INDEPENDANT;
    for (int i = 0; i < numOfSources; i++) {
        processor.getSources()[i].fixSourcePosition(shouldBeFixed);
    }
    processor.addNewFixedPosition();
}

void ControlGrisAudioProcessorEditor::trajectoryBoxClearButtonClicked() {
    float center = FIELD_WIDTH / 2.0 - kSourceRadius;
    automationManager.resetRecordingTrajectory(Point<float> (center, center));
    mainField.repaint();
}

void ControlGrisAudioProcessorEditor::trajectoryBoxClearAltButtonClicked() {
    float center = FIELD_WIDTH / 2.0 - kSourceRadius;
    automationManagerAlt.resetRecordingTrajectory(Point<float> (center, center));
    elevationField.repaint();
}

// Update the interface if anything has changed (mostly automations).
//-------------------------------------------------------------------
void ControlGrisAudioProcessorEditor::refresh() {
    parametersBox.setSelectedSource(&processor.getSources()[m_selectedSource]);
    sourceBox.updateSelectedSource(&processor.getSources()[m_selectedSource], m_selectedSource, processor.getOscFormat());

    mainField.setIsPlaying(processor.getIsPlaying());
    elevationField.setIsPlaying(processor.getIsPlaying());

    if (! m_fixedSourcesWindowVisible) {
        mainField.repaint();
        if (processor.getOscFormat() == SPAT_MODE_LBAP)
            elevationField.repaint();
    }
}

// FieldComponent::Listener callback.
//-----------------------------------
void ControlGrisAudioProcessorEditor::fieldSourcePositionChanged(int sourceId) {
    m_selectedSource = sourceId;
    parametersBox.setSelectedSource(&processor.getSources()[sourceId]);
    processor.setSourceParameterValue(sourceId, SOURCE_ID_AZIMUTH, processor.getSources()[sourceId].getNormalizedAzimuth());
    processor.setSourceParameterValue(sourceId, SOURCE_ID_ELEVATION, processor.getSources()[sourceId].getNormalizedElevation());
    processor.setSourceParameterValue(sourceId, SOURCE_ID_DISTANCE, processor.getSources()[sourceId].getDistance());

    mainField.setSelectedSource(m_selectedSource);
    elevationField.setSelectedSource(m_selectedSource);
    processor.setSelectedSourceId(m_selectedSource);
    sourceBox.updateSelectedSource(&processor.getSources()[m_selectedSource], m_selectedSource, processor.getOscFormat());

    validateSourcePositions();
    if (processor.getOscFormat() == SPAT_MODE_LBAP) {
        validateSourcePositionsAlt();
    }
}

// FixedPositionEditor::Listener callback.
//----------------------------------------
void ControlGrisAudioProcessorEditor::fixedPositionEditorCellChanged(int row, int column, double value) {
    processor.changeFixedPosition(row, column, value);
    fixedPositionEditor.loadData(processor.getFixedPositionData());
}

void ControlGrisAudioProcessorEditor::fixedPositionEditorCellDeleted(int row, int column) {
    processor.deleteFixedPosition(row, column);
    fixedPositionEditor.loadData(processor.getFixedPositionData());
}

void ControlGrisAudioProcessorEditor::fixedPositionEditorClosed() {
    m_fixedSourcesWindowVisible = false;
    resized();
}

//==============================================================================
void ControlGrisAudioProcessorEditor::paint (Graphics& g) {
    GrisLookAndFeel *lookAndFeel;
    lookAndFeel = static_cast<GrisLookAndFeel *> (&getLookAndFeel());
    g.fillAll (lookAndFeel->findColour (ResizableWindow::backgroundColourId));
}

void ControlGrisAudioProcessorEditor::resized() {
    double width = getWidth();
    double height = getHeight();

    double fieldSize = width / 2;
    if (fieldSize < FIELD_WIDTH) { fieldSize = FIELD_WIDTH; }

    mainBanner.setBounds(0, 0, fieldSize, 20);
    mainField.setBounds(0, 20, fieldSize, fieldSize);

    if (processor.getOscFormat() == SPAT_MODE_LBAP) {
        mainBanner.setText("Azimuth - Distance", NotificationType::dontSendNotification);
        elevationBanner.setVisible(true);
        elevationField.setVisible(true);
        elevationBanner.setBounds(fieldSize, 0, fieldSize, 20);
        elevationField.setBounds(fieldSize, 20, fieldSize, fieldSize);
    } else {
        mainBanner.setText("Azimuth - Elevation", NotificationType::dontSendNotification);
        elevationBanner.setVisible(false);
        elevationField.setVisible(false);
    }

    parametersBox.setBounds(0, fieldSize + 20, width, 50);

    trajectoryBanner.setBounds(0, fieldSize + 70, width, 20);
    trajectoryBox.setBounds(0, fieldSize + 90, width, 140);

    settingsBanner.setBounds(0, fieldSize + 230, width, 20);
    configurationComponent.setBounds(0, fieldSize + 250, width, 130);

    lastUIWidth  = getWidth();
    lastUIHeight = getHeight();

    if (m_fixedSourcesWindowVisible) {
        fixedPositionEditor.setVisible(true);
        fixedPositionEditor.setBounds(0, 0, width, height);
    } else {
        fixedPositionEditor.setVisible(false);
    }
}

//==============================================================================
void ControlGrisAudioProcessorEditor::validateSourcePositions() {
    int numOfSources = processor.getNumberOfSources();
    int sourceLink = automationManager.getSourceLink();

    // Nothing to do for independant mode.

    // All circular modes.
    if (sourceLink >= SOURCE_LINK_CIRCULAR && sourceLink < SOURCE_LINK_DELTA_LOCK) {
        float deltaAzimuth = processor.getSources()[0].getDeltaAzimuth();
        float deltaElevation = processor.getSources()[0].getDeltaElevation();
        float deltaDistance = processor.getSources()[0].getDeltaDistance();
        for (int i = 1; i < numOfSources; i++) {
            processor.getSources()[i].setCoordinatesFromFixedSource(deltaAzimuth, deltaElevation, deltaDistance);
        }
    } 
    // Delta Lock mode.
    else if (sourceLink == SOURCE_LINK_DELTA_LOCK) {
        float deltaX = processor.getSources()[0].getDeltaX();
        float deltaY = processor.getSources()[0].getDeltaY();
        for (int i = 1; i < numOfSources; i++) {
            processor.getSources()[i].setXYCoordinatesFromFixedSource(deltaX, deltaY);
        }
    }

    // Fix source positions.
    automationManager.fixSourcePosition();
    bool shouldBeFixed = sourceLink != SOURCE_LINK_INDEPENDANT;
    if (sourceLink >= 2 && sourceLink < 6) {
        for (int i = 0; i < numOfSources; i++) {
            processor.getSources()[i].fixSourcePosition(shouldBeFixed);
        }
    }
}

void ControlGrisAudioProcessorEditor::validateSourcePositionsAlt() {
    int numOfSources = processor.getNumberOfSources();
    int sourceLink = automationManagerAlt.getSourceLink();

    // Fixed elevation.
    if (sourceLink == SOURCE_LINK_ALT_FIXED_ELEVATION) {
        for (int i = 1; i < numOfSources; i++) {
            processor.getSources()[i].setElevation(processor.getSources()[0].getElevation());
        }
    }
    // Linear min.
    else if (sourceLink == SOURCE_LINK_ALT_LINEAR_MIN) {
        for (int i = 1; i < numOfSources; i++) {
            float offset = processor.getSources()[0].getElevation();
            processor.getSources()[i].setElevation(60.0 / numOfSources * i + offset);
        }
    }
    // Linear max.
    else if (sourceLink == SOURCE_LINK_ALT_LINEAR_MAX) {
        for (int i = 1; i < numOfSources; i++) {
            float offset = 90.0 - processor.getSources()[0].getElevation();
            processor.getSources()[i].setElevation(90.0 - (60.0 / numOfSources * i) - offset);
        }
    }
    // Delta lock.
    else if (sourceLink == SOURCE_LINK_ALT_DELTA_LOCK) {
        float deltaY = processor.getSources()[0].getDeltaElevation();
        for (int i = 1; i < numOfSources; i++) {
            processor.getSources()[i].setElevationFromFixedSource(deltaY);
        }
    }

    // Fix source positions.
    automationManagerAlt.fixSourcePosition(); // not sure...
    bool shouldBeFixed = sourceLink != SOURCE_LINK_ALT_INDEPENDANT;
    if (sourceLink >= 2 && sourceLink < 5) {
        for (int i = 0; i < numOfSources; i++) {
            processor.getSources()[i].fixSourcePosition(shouldBeFixed);
        }
    }
}
