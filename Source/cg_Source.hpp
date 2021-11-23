/**************************************************************************
 * Copyright 2021 UdeM - GRIS - Samuel B�land & Olivier Belanger          *
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

#include "cg_constants.hpp"

namespace gris
{
//==============================================================================
class ControlGrisAudioProcessor;

enum class SourceParameter { azimuth, elevation, distance, x, y, azimuthSpan, elevationSpan };
//==============================================================================
class Source
{
public:
    //==============================================================================
    enum class OriginOfChange { none, userMove, userAnchorMove, link, trajectory, automation, presetRecall, osc };
    enum class ChangeType { position, elevation };
    //==============================================================================
    class Listener : private juce::AsyncUpdater
    {
    public:
        //==============================================================================
        Listener() = default;
        ~Listener() override = default;
        //==============================================================================
        Listener(Listener const &) = delete;
        Listener(Listener &&) = delete;
        Listener & operator=(Listener const &) = delete;
        Listener & operator=(Listener &&) = delete;
        //==============================================================================
        void update() { triggerAsyncUpdate(); }

    private:
        //==============================================================================
        void handleAsyncUpdate() override { sourceMovedCallback(); }
        virtual void sourceMovedCallback() = 0;
        //==============================================================================
        JUCE_LEAK_DETECTOR(Listener)

    }; // class Source::Listener

private:
    //==============================================================================
    juce::ListenerList<Listener> mGuiListeners;

    SourceIndex mIndex{};
    SourceId mId{ 1 };
    SpatMode mSpatMode{ SpatMode::dome };

    Radians mAzimuth{};
    Radians mElevation{};
    float mDistance{ 1.0f };

    juce::Point<float> mPosition{};

    Normalized mAzimuthSpan{};
    Normalized mElevationSpan{};

    juce::Colour mColour{ juce::Colours::black };
    ControlGrisAudioProcessor * mProcessor{};

public:
    //==============================================================================
    void setIndex(SourceIndex const index) { mIndex = index; }
    [[nodiscard]] SourceIndex getIndex() const { return mIndex; }

    void setId(SourceId const id) { mId = id; }
    [[nodiscard]] SourceId getId() const { return mId; }

    void setSpatMode(SpatMode const spatMode) { mSpatMode = spatMode; }
    [[nodiscard]] SpatMode getSpatMode() const { return mSpatMode; }

    void setAzimuth(Radians azimuth, OriginOfChange origin);
    void setAzimuth(Normalized azimuth, OriginOfChange origin);
    [[nodiscard]] Radians getAzimuth() const { return mAzimuth; }
    [[nodiscard]] Normalized getNormalizedAzimuth() const;

    void setElevation(Radians elevation, OriginOfChange origin);
    void setElevation(Normalized elevation, OriginOfChange origin);
    [[nodiscard]] Radians getElevation() const { return mElevation; }
    [[nodiscard]] Normalized getNormalizedElevation() const;

    void setDistance(float distance, OriginOfChange origin);
    [[nodiscard]] float getDistance() const { return mDistance; }
    void setAzimuthSpan(Normalized azimuthSpan);
    [[nodiscard]] Normalized getAzimuthSpan() const { return mAzimuthSpan; }
    void setElevationSpan(Normalized elevationSpan);
    [[nodiscard]] Normalized getElevationSpan() const { return mElevationSpan; }

    void setCoordinates(Radians azimuth, Radians elevation, float distance, OriginOfChange origin);
    [[nodiscard]] bool isPrimarySource() const { return mIndex == SourceIndex{ 0 }; }

    void setX(float x, OriginOfChange origin);
    void setX(Normalized x, OriginOfChange origin);
    void setY(Normalized y, OriginOfChange origin);
    [[nodiscard]] float getX() const { return mPosition.getX(); }
    void setY(float y, OriginOfChange origin);
    [[nodiscard]] float getY() const { return mPosition.getY(); }
    [[nodiscard]] juce::Point<float> const & getPos() const { return mPosition; }
    void setPosition(juce::Point<float> const & pos, OriginOfChange origin);

    void computeXY();
    void computeAzimuthElevation();

    void setColorFromIndex(int numTotalSources);
    [[nodiscard]] juce::Colour getColour() const { return mColour; }

    void addGuiListener(Listener * listener) { mGuiListeners.add(listener); }
    void removeGuiListener(Listener * listener) { mGuiListeners.remove(listener); }

    void setProcessor(ControlGrisAudioProcessor * processor) { mProcessor = processor; }

    static juce::Point<float> getPositionFromAngle(Radians angle, float radius);
    static Radians getAngleFromPosition(juce::Point<float> const & position);

    static juce::Point<float> clipPosition(juce::Point<float> const & position, SpatMode spatMode);
    static juce::Point<float> clipDomePosition(juce::Point<float> const & position);
    static juce::Point<float> clipCubePosition(juce::Point<float> const & position);

private:
    //==============================================================================
    [[nodiscard]] bool shouldForceNotifications(OriginOfChange origin) const;
    void notify(ChangeType changeType, OriginOfChange origin);
    void notifyGuiListeners();
    [[nodiscard]] static Radians clipElevation(Radians elevation);
    [[nodiscard]] static float clipCoordinate(float coordinate);
    //==============================================================================
    JUCE_LEAK_DETECTOR(Source)
};
} // namespace gris