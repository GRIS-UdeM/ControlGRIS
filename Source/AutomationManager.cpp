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
#include "AutomationManager.h"
#include "ControlGrisConstants.h"

AutomationManager::AutomationManager() {
    sourceLink = SOURCE_LINK_INDEPENDANT;
    drawingType = TRAJECTORY_TYPE_DRAWING;
    activateState = false;
    source.setX(0.5f);
    source.setY(0.5f);
    playbackDuration = 5.0;
    currentTrajectoryPoint = Point<float> (FIELD_WIDTH / 2, FIELD_WIDTH / 2);
    playbackPosition = Point<float> (-1.0f, -1.0f);
    trajectoryDeltaTime = 0.0;
}

AutomationManager::~AutomationManager() {}

void AutomationManager::setActivateState(bool state) {
    activateState = state;
}

bool AutomationManager::getActivateState() {
    return activateState;
}

void AutomationManager::setPlaybackDuration(double value) {
    playbackDuration = value;
}

double AutomationManager::getPlaybackDuration() {
    return playbackDuration;
}

void AutomationManager::setPlaybackPositionX(float value) {
    playbackPosition.x = value;
    source.setX(value);
}

void AutomationManager::setPlaybackPositionY(float value) {
    playbackPosition.y = value;
    source.setY(value);
}

bool AutomationManager::hasValidPlaybackPosition() {
    return playbackPosition != Point<float> (-1.0f, -1.0f);
}

Point<float> AutomationManager::getPlaybackPosition() {
    return playbackPosition;
}

void AutomationManager::resetRecordingTrajectory(Point<float> currentPosition) {
    playbackPosition = Point<float> (-1.0f, -1.0f);
    trajectoryPoints.clear();
    lastRecordingPoint = currentPosition;
}

Point<float> AutomationManager::smoothRecordingPosition(Point<float> pos) {
    Point<float> smoothed;
    smoothed.x = lastRecordingPoint.x = pos.x + (lastRecordingPoint.x - pos.x) * 0.8f;
    smoothed.y = lastRecordingPoint.y = pos.y + (lastRecordingPoint.y - pos.y) * 0.8f;
    return smoothed;
}

void AutomationManager::addRecordingPoint(Point<float> pos) {
    trajectoryPoints.add(smoothRecordingPosition(pos));
}

int AutomationManager::getRecordingTrajectorySize() {
    return trajectoryPoints.size();
}

Point<float> AutomationManager::getFirstRecordingPoint() {
    return trajectoryPoints.getFirst();
}

Point<float> AutomationManager::getLastRecordingPoint() {
    return trajectoryPoints.getLast();
}

void AutomationManager::createRecordingPath(Path& path) {
    path.startNewSubPath(trajectoryPoints[0]);
    for (int i = 1; i < trajectoryPoints.size(); i++) {
        path.lineTo(trajectoryPoints[i]);
    }
}

void AutomationManager::setTrajectoryDeltaTime(double relativeTimeFromPlay) {
    trajectoryDeltaTime = relativeTimeFromPlay / playbackDuration;
    trajectoryDeltaTime = std::fmod(trajectoryDeltaTime, 1.0f);
    computeCurrentTrajectoryPoint();
}

void AutomationManager::computeCurrentTrajectoryPoint() {
    if (trajectoryPoints.size() > 0) {
        double delta = trajectoryDeltaTime * trajectoryPoints.size();
        if (delta + 1 >= trajectoryPoints.size()) {
            delta = std::fmod(delta, trajectoryPoints.size());
        }
        int index = (int)delta;
        if (index + 1 < trajectoryPoints.size()) {
            double frac = delta - index;
            Point<float> p1 = trajectoryPoints[index];
            Point<float> p2 = trajectoryPoints[index+1];
            currentTrajectoryPoint = Point<float> ((p1.x + (p2.x - p1.x) * frac), (p1.y + (p2.y - p1.y) * frac));
        } else {
            currentTrajectoryPoint = Point<float> (trajectoryPoints.getLast().x, trajectoryPoints.getLast().y);
        }
    }

    if (activateState) {
        setSourcePosition(Point<float> (currentTrajectoryPoint.x / FIELD_WIDTH, 1.0 - currentTrajectoryPoint.y / FIELD_WIDTH));
    }
}

Point<float> AutomationManager::getCurrentTrajectoryPoint() {
    if (activateState) {
        return currentTrajectoryPoint;
    } else {
        return Point<float> (getSourcePosition().x * FIELD_WIDTH, (1.0 - getSourcePosition().y) * FIELD_WIDTH);
    }
}

void AutomationManager::setSourcePosition(Point<float> pos) {
    source.setPos(pos);
    if (activateState) {
        listeners.call([&] (Listener& l) { l.trajectoryPositionChanged(this, pos); });
    }
}

Source& AutomationManager::getSource() {
    return source;
}

Point<float> AutomationManager::getSourcePosition() {
    return source.getPos();
}

void AutomationManager::fixSourcePosition() {
    bool shouldBeFixed = sourceLink != SOURCE_LINK_INDEPENDANT;
    source.fixSourcePosition(shouldBeFixed);
}

void AutomationManager::setDrawingType(int type) {
    drawingType = type;

    trajectoryPoints.clear();

    int offset = FIELD_WIDTH / 2;
    int radius = offset - 10;
    switch (drawingType) {
        case TRAJECTORY_TYPE_DRAWING:
            break;
        case TRAJECTORY_TYPE_CIRCLE_CLOCKWISE:
            for (int i = 0; i < 200; i++) {
                float x = sinf(2.0 * M_PI * i / 199) * radius + offset;
                float y = -cosf(2.0 * M_PI * i / 199) * radius + offset;
                trajectoryPoints.add(Point<float> (x, y));
            }
            break;
        case TRAJECTORY_TYPE_CIRCLE_COUNTER_CLOCKWISE:
            for (int i = 0; i < 200; i++) {
                float x = -sinf(2.0 * M_PI * i / 199) * radius + offset;
                float y = -cosf(2.0 * M_PI * i / 199) * radius + offset;
                trajectoryPoints.add(Point<float> (x, y));
            }
            break;
        case TRAJECTORY_TYPE_ELLIPSE_CLOCKWISE:
            for (int i = 0; i < 200; i++) {
                float x = sinf(2.0 * M_PI * i / 199) * radius * 0.5 + offset;
                float y = -cosf(2.0 * M_PI * i / 199) * radius + offset;
                trajectoryPoints.add(Point<float> (x, y));
            }
            break;
        case TRAJECTORY_TYPE_ELLIPSE_COUNTER_CLOCKWISE:
            for (int i = 0; i < 200; i++) {
                float x = -sinf(2.0 * M_PI * i / 199) * radius * 0.5 + offset;
                float y = -cosf(2.0 * M_PI * i / 199) * radius + offset;
                trajectoryPoints.add(Point<float> (x, y));
            }
            break;
        case TRAJECTORY_TYPE_SPIRAL_CLOCKWISE_OUT_IN:
            for (int i = 0; i < 300; i++) {
                float x = sinf(2.0 * M_PI * i / 99) * radius * (1.0 - i / 300.0) + offset;
                float y = -cosf(2.0 * M_PI * i / 99) * radius * (1.0 - i / 300.0) + offset;
                trajectoryPoints.add(Point<float> (x, y));
            }
            break;
        case TRAJECTORY_TYPE_SPIRAL_COUNTER_CLOCKWISE_OUT_IN:
            for (int i = 0; i < 300; i++) {
                float x = -sinf(2.0 * M_PI * i / 99) * radius * (1.0 - i / 300.0) + offset;
                float y = -cosf(2.0 * M_PI * i / 99) * radius * (1.0 - i / 300.0) + offset;
                trajectoryPoints.add(Point<float> (x, y));
            }
            break;
        case TRAJECTORY_TYPE_SPIRAL_CLOCKWISE_IN_OUT:
            for (int i = 0; i < 300; i++) {
                float x = sinf(2.0 * M_PI * i / 99) * radius * (i / 300.0) + offset;
                float y = -cosf(2.0 * M_PI * i / 99) * radius * (i / 300.0) + offset;
                trajectoryPoints.add(Point<float> (x, y));
            }
            break;
        case TRAJECTORY_TYPE_SPIRAL_COUNTER_CLOCKWISE_IN_OUT:
            for (int i = 0; i < 300; i++) {
                float x = -sinf(2.0 * M_PI * i / 99) * radius * (i / 300.0) + offset;
                float y = -cosf(2.0 * M_PI * i / 99) * radius * (i / 300.0) + offset;
                trajectoryPoints.add(Point<float> (x, y));
            }
            break;
    }
}

void AutomationManager::setDrawingTypeAlt(int type) {
    drawingType = type;

    trajectoryPoints.clear();

    float x = 10.0 + kSourceRadius;
    float minPos = 15.0, maxPos = FIELD_WIDTH - 20.0;

    switch (drawingType) {
        case TRAJECTORY_TYPE_ALT_DRAWING:
            break;
        case TRAJECTORY_TYPE_ALT_DOWN_UP:
            for (int i = 0; i < 200; i++) {
                float y = (i / 199.0) * (maxPos - minPos) + minPos;
                trajectoryPoints.add(Point<float> (x, y));
            }
            break;
        case TRAJECTORY_TYPE_ALT_UP_DOWN:
            for (int i = 0; i < 200; i++) {
                float y = (1.0 - i / 199.0) * (maxPos - minPos) + minPos;
                trajectoryPoints.add(Point<float> (x, y));
            }
            break;
        case TRAJECTORY_TYPE_ALT_BACK_AND_FORTH_UP:
            for (int i = 0; i < 200; i++) {
                float y;
                if (i < 100) {
                    y = (i / 99.0) * (maxPos - minPos) + minPos;
                } else {
                    y = ((200 - i) / 99.0) * (maxPos - minPos) + minPos;
                }
                trajectoryPoints.add(Point<float> (x, y));
            }
            break;
        case TRAJECTORY_TYPE_ALT_BACK_AND_FORTH_DOWN:
            for (int i = 0; i < 200; i++) {
                float y;
                if (i < 100) {
                    y = (1.0 - i / 99.0) * (maxPos - minPos) + minPos;
                } else {
                    y = (1.0 - (200 - i) / 99.0) * (maxPos - minPos) + minPos;
                }
                trajectoryPoints.add(Point<float> (x, y));
            }
            break;
    }
}

int AutomationManager::getDrawingType() {
    return drawingType;
}

void AutomationManager::setSourceLink(int value) {
    sourceLink = value;
}

int AutomationManager::getSourceLink() {
    return sourceLink;
}
