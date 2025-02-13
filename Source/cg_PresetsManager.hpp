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

#include <array>
#include <optional>

#include <JuceHeader.h>

#include "cg_SourceLinkEnforcer.hpp"
#include "cg_constants.hpp"

namespace gris
{
enum class FixedPositionType { terminal, initial };
juce::String
    getFixedPosSourceName(FixedPositionType const fixedPositionType, SourceIndex const index, int const dimension);
//==============================================================================
class PresetsManager final : public juce::ChangeBroadcaster
{
    //==============================================================================
    int mLastLoadedPreset{ 0 };
    juce::XmlElement & mData;
    Sources & mSources;
    SourceLinkEnforcer & mPositionLinkEnforcer;
    SourceLinkEnforcer & mElevationLinkEnforcer;

public:
    //==============================================================================
    PresetsManager() = delete;
    ~PresetsManager() noexcept override = default;

    PresetsManager(PresetsManager const &) = delete;
    PresetsManager(PresetsManager &&) = delete;

    PresetsManager & operator=(PresetsManager const &) = delete;
    PresetsManager & operator=(PresetsManager &&) = delete;
    //==============================================================================
    PresetsManager(juce::XmlElement & data,
                   Sources & sources,
                   SourceLinkEnforcer & positionLinkEnforcer,
                   SourceLinkEnforcer & elevationLinkEnforcer);
    //==============================================================================
    [[nodiscard]] int getCurrentPreset() const;
    [[nodiscard]] std::array<bool, NUMBER_OF_POSITION_PRESETS> getSavedPresets() const;

    bool loadIfPresetChanged(int presetNumber);
    bool forceLoad(int presetNumber);
    void save(int presetNumber) const;
    [[nodiscard]] bool deletePreset(int presetNumber) const;
    void numberOfSourcesChanged();

private:
    //==============================================================================
    [[nodiscard]] bool contains(int presetNumber) const;
    [[nodiscard]] std::unique_ptr<juce::XmlElement> createPresetData(int presetNumber) const;
    [[nodiscard]] std::optional<juce::XmlElement *> getPresetData(int presetNumber) const;

    [[nodiscard]] bool load(int presetNumber);
    //==============================================================================
    JUCE_LEAK_DETECTOR(PresetsManager)
};

} // namespace gris
