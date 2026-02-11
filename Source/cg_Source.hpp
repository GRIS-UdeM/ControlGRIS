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

#pragma once

#include "cg_constants.hpp"
#include <Data/StrongTypes/sg_Radians.hpp>
#include <Data/StrongTypes/sg_SourceIndex.hpp>
#include <JuceHeader.h>
#include <vector>

namespace gris
{
//==============================================================================
// Forward declaration
class ControlGrisAudioProcessor;

enum class SourceParameter { azimuth, elevation, distance, x, y, azimuthSpan, elevationSpan };

//==============================================================================
// Source class definition
class Source
{
public:
    // Default constructor
    Source() = default;

    // Copy constructor
    Source(const Source & other)
        : mIndex(other.mIndex)
        , mId(other.mId)
        , mSpatMode(other.mSpatMode)
        , mAzimuth(other.mAzimuth)
        , mElevation(other.mElevation)
        , mDistance(other.mDistance)
        , mPosition(other.mPosition)
        , mAzimuthSpan(other.mAzimuthSpan)
        , mElevationSpan(other.mElevationSpan)
        , mColour(other.mColour)
        , mProcessor(other.mProcessor)
    {
    }

    // Move constructor
    Source(Source && other) noexcept
        : mIndex(other.mIndex)
        , mId(other.mId)
        , mSpatMode(other.mSpatMode)
        , mAzimuth(other.mAzimuth)
        , mElevation(other.mElevation)
        , mDistance(other.mDistance)
        , mPosition(other.mPosition)
        , mAzimuthSpan(other.mAzimuthSpan)
        , mElevationSpan(other.mElevationSpan)
        , mColour(std::move(other.mColour))
        , mProcessor(other.mProcessor)
    {
    }

    // Copy assignment operator
    Source & operator=(const Source & other)
    {
        if (this != &other) {
            mIndex = other.mIndex;
            mId = other.mId;
            mSpatMode = other.mSpatMode;
            mAzimuth = other.mAzimuth;
            mElevation = other.mElevation;
            mDistance = other.mDistance;
            mPosition = other.mPosition;
            mAzimuthSpan = other.mAzimuthSpan;
            mElevationSpan = other.mElevationSpan;
            mColour = other.mColour;
            mProcessor = other.mProcessor;
        }
        return *this;
    }

    // Move assignment operator
    Source & operator=(Source && other) noexcept
    {
        if (this != &other) {
            mIndex = std::move(other.mIndex);
            mId = std::move(other.mId);
            mSpatMode = std::move(other.mSpatMode);
            mAzimuth = std::move(other.mAzimuth);
            mElevation = std::move(other.mElevation);
            mDistance = std::move(other.mDistance);
            mPosition = std::move(other.mPosition);
            mAzimuthSpan = std::move(other.mAzimuthSpan);
            mElevationSpan = std::move(other.mElevationSpan);
            mColour = std::move(other.mColour);
            mProcessor = std::move(other.mProcessor);
        }
        return *this;
    }

    //==============================================================================
    enum class OriginOfChange {
        none,
        userMove,
        userAnchorMove,
        link,
        trajectory,
        automation,
        presetRecall,
        osc,
        audioAnalysis,
        audioAnalysisRecAutomation
    };
    enum class ChangeType { position, elevation };

    //==============================================================================

    class Listener : private juce::AsyncUpdater
    {
    public:
        //==============================================================================

        Listener() = default;
        virtual ~Listener() override = default;

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
    };

private:
    //==============================================================================
    juce::LightweightListenerList<Listener> mGuiListeners;

    /**
     * @brief The internal index of the source.
     *
     * This is not shown to the user, and is used internally to identify a source within a collection of sources.
     * For example it is used when iterating over sources.
     */
    SourceIndex mIndex{};

    /**
     * @brief The Source ID is set by the user, e.g., in ControlGrisAudioProcessorEditor::firstSourceIdChangedCallback()
     *
     * It is used by the user to identify sound sources through the GRIS workflow; e..g, it is used to differentiate
     * between sources in the OSC messages.
     */
    SourceId mId{ 1 };

    SpatMode mSpatMode{ SpatMode::dome };

    /**
     * @brief The azimuth angle of the source in radians.
     *
     * This member is set and/or kept in sync with mDistance and mPosition in the following functions:
     * - setAzimuth(Radians azimuth, OriginOfChange origin)
     * - setAzimuth(Normalized azimuth, OriginOfChange origin)
     * - setCoordinates(Radians azimuth, Radians elevation, float distance, OriginOfChange origin)
     * - computeAzimuthElevation()
     */
    Radians mAzimuth{};

    /**
     * @brief The elevation angle of the source in radians.
     *
     * This member is set and/or kept in sync with mDistance and mPosition in the following functions:
     * - setElevation(Radians elevation, OriginOfChange origin)
     * - setElevation(Normalized elevation, OriginOfChange origin)
     * - setCoordinates(Radians azimuth, Radians elevation, float distance, OriginOfChange origin)
     * - computeAzimuthElevation()
     */
    Radians mElevation{};

    /**
     * @brief The distance of the source from the origin.
     *
     * This represents the radial distance of the source to the origin of the spatialization field.
     * It is modified in setDistance(), setCoordinates(), and computeAzimuthElevation().
     */
    float mDistance{ 1.0f };

    /**
     * @brief The XY position of the source in Cartesian coordinates.
     *
     * This represents the (x, y) position of the source in the spatialization field.
     * It is modified in setX(), setY(), setPosition(), setElevation(), setCoordinates() and computeXY(),
     * computeAzimuthElevation.
     */
    juce::Point<float> mPosition{};

    Normalized mAzimuthSpan{};
    Normalized mElevationSpan{};

    juce::Colour mColour{ juce::Colours::black.withAlpha(0.0f) };
    ControlGrisAudioProcessor * mProcessor{};

public:
    //==============================================================================

    void setIndex(SourceIndex const index) { mIndex = index; }
    [[nodiscard]] SourceIndex getIndex() const { return mIndex; }

    void setId(SourceId const id) { mId = id; }
    [[nodiscard]] SourceId getId() const { return mId; }

    void setSpatMode(SpatMode const spatMode) { mSpatMode = spatMode; }
    [[nodiscard]] SpatMode getSpatMode() const { return mSpatMode; }

    /**
     * @brief Sets the azimuth angle of the source in radians.
     *
     * This function sets the azimuth angle of the source using a Radians type.
     * The azimuth angle represents the horizontal angle of the source in the spatialization field.
     *
     * @param azimuth The azimuth angle in radians.
     * @param origin The origin of the change.
     */
    void setAzimuth(Radians azimuth, OriginOfChange origin);

    /**
     * @brief Sets the azimuth angle of the source in normalized units.
     *
     * This function sets the azimuth angle of the source using a Normalized type.
     * The azimuth angle is normalized between 0 and 1, where 0 represents 0 radians and 1 represents 2*pi radians.
     *
     * @param azimuth The azimuth angle in normalized units.
     * @param origin The origin of the change.
     */
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
    void setColour(juce::Colour colour);
    [[nodiscard]] juce::Colour getColour() const { return mColour; }

    void addGuiListener(Listener * listener) { mGuiListeners.add(listener); }
    void removeGuiListener(Listener * listener) { mGuiListeners.remove(listener); }

    void setProcessor(ControlGrisAudioProcessor * processor) { mProcessor = processor; }

    /**
     * @brief Computes the position from a given angle and radius.
     *
     * This function calculates the Cartesian coordinates (x, y) from a given angle and radius.
     * The angle is rotated by -pi/2 radians before computing the coordinates.
     *
     * @param angle The angle in radians.
     * @param radius The radius or distance from the origin.
     * @return A juce::Point<float> representing the computed position.
     */
    static juce::Point<float> getPositionFromAngle(Radians angle, float radius);
    static Radians getAngleFromPosition(juce::Point<float> const & position);

    static juce::Point<float> clipPosition(juce::Point<float> const & position, SpatMode spatMode);
    static juce::Point<float> clipDomePosition(juce::Point<float> const & position);
    static juce::Point<float> clipCubePosition(juce::Point<float> const & position);

private:
    //==============================================================================

    bool shouldForceNotifications(OriginOfChange origin) const;
    void notify(ChangeType changeType, OriginOfChange origin);
    void notifyGuiListeners();
    static Radians clipElevation(Radians elevation);
    static float clipCoordinate(float coord);

    //==============================================================================

    JUCE_LEAK_DETECTOR(Source)
};

//==============================================================================
// Sources class definition
class Sources
{
    //==============================================================================

    struct Iterator {
        Sources * sources;
        int index;

        //==============================================================================

        bool operator!=(Iterator const & other) const { return index != other.index; }
        Iterator & operator++()
        {
            ++index;
            return *this;
        }
        Source & operator*() { return sources->get(index); }
        Source const & operator*() const { return sources->get(index); }
    };

    //==============================================================================

    struct ConstIterator {
        Sources const * sources;
        int index;

        //==============================================================================

        bool operator!=(ConstIterator const & other) const { return index != other.index; }
        ConstIterator & operator++()
        {
            ++index;
            return *this;
        }
        Source const & operator*() const { return sources->get(index); }
    };

    //==============================================================================

    int mSize{ 2 };
    Source mPrimarySource;
    std::vector<Source> mSecondarySources{};

public:
    static const int MAX_NUMBER_OF_SOURCES{ 256 };

    //==============================================================================
    Sources() { mSecondarySources.resize(MAX_NUMBER_OF_SOURCES - 1); }

    [[nodiscard]] int size() const { return mSize; }
    void setSize(int size);

    [[nodiscard]] Source & get(int const index)
    {
        jassert(index >= 0 && index < MAX_NUMBER_OF_SOURCES); // TODO: should check for mSize
        if (index == 0) {
            return mPrimarySource;
        }
        return mSecondarySources[static_cast<size_t>(index) - 1u];
    }
    [[nodiscard]] Source const & get(int const index) const
    {
        jassert(index >= 0 && index < MAX_NUMBER_OF_SOURCES); // TODO: should check for mSize
        if (index == 0) {
            return mPrimarySource;
        }
        return mSecondarySources[static_cast<size_t>(index) - 1u];
    }
    [[nodiscard]] Source & get(SourceIndex const index) { return get(index.get()); }
    [[nodiscard]] Source const & get(SourceIndex const index) const { return get(index.get()); }
    [[nodiscard]] Source & operator[](int const index)
    {
        jassert(index >= 0 && index < MAX_NUMBER_OF_SOURCES); // TODO: should check for mSize
        if (index == 0) {
            return mPrimarySource;
        }
        return mSecondarySources[static_cast<size_t>(index) - 1u];
    }
    [[nodiscard]] Source const & operator[](int const index) const
    {
        jassert(index >= 0 && index < MAX_NUMBER_OF_SOURCES); // TODO: should check for mSize
        if (index == 0) {
            return mPrimarySource;
        }
        return mSecondarySources[static_cast<size_t>(index) - 1u];
    }
    [[nodiscard]] Source & operator[](SourceIndex const index) { return (*this)[index.get()]; }
    [[nodiscard]] Source const & operator[](SourceIndex const index) const { return (*this)[index.get()]; }

    void init(ControlGrisAudioProcessor * processor)
    {
        SourceIndex currentIndex{};
        mPrimarySource.setIndex(currentIndex++);
        mPrimarySource.setProcessor(processor);
        mSecondarySources.resize(MAX_NUMBER_OF_SOURCES - 1);
        for (auto & secondarySource : mSecondarySources) {
            secondarySource.setIndex(currentIndex++);
            secondarySource.setProcessor(processor);
        }
    }

    [[nodiscard]] Source & getPrimarySource() { return mPrimarySource; }
    [[nodiscard]] Source const & getPrimarySource() const { return mPrimarySource; }
    [[nodiscard]] auto & getSecondarySources() { return mSecondarySources; }
    [[nodiscard]] auto const & getSecondarySources() const { return mSecondarySources; }

    [[nodiscard]] Iterator begin() { return Iterator{ this, 0 }; }
    [[nodiscard]] ConstIterator begin() const { return ConstIterator{ this, 0 }; }
    [[nodiscard]] Iterator end() { return Iterator{ this, mSize }; }
    [[nodiscard]] ConstIterator end() const { return ConstIterator{ this, mSize }; }

private:
    //==============================================================================

    JUCE_LEAK_DETECTOR(Sources)
};

} // namespace gris
