/**************************************************************************
 * Copyright 2021 UdeM - GRIS - Samuel Béland & Olivier Belanger          *
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

#include "cg_MinSizedComponent.hpp"
namespace gris
{
class GrisLookAndFeel;

//==============================================================================
class TitledComponent final : public MinSizedComponent
{
    static constexpr auto TITLE_HEIGHT = 18;

    juce::String mTitle{};
    MinSizedComponent * mContentComponent;
    GrisLookAndFeel & mLookAndFeel;

public:
    //==============================================================================
    TitledComponent(juce::String title, MinSizedComponent * contentComponent, GrisLookAndFeel & lookAndFeel);
    ~TitledComponent() override = default;
    //==============================================================================
    TitledComponent(TitledComponent const &) = delete;
    TitledComponent(TitledComponent &&) = delete;
    TitledComponent & operator=(TitledComponent const &) = delete;
    TitledComponent & operator=(TitledComponent &&) = delete;
    //==============================================================================
    void resized() override;
    void paint(juce::Graphics & g) override;

    [[nodiscard]] int getMinWidth() const noexcept override { return mContentComponent->getMinWidth(); }
    [[nodiscard]] int getMinHeight() const noexcept override
    {
        return mContentComponent->getMinHeight() + TITLE_HEIGHT;
    }

private:
    //==============================================================================
    JUCE_LEAK_DETECTOR(TitledComponent)
};
} // namespace gris