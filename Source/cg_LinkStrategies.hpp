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

#include <JuceHeader.h>

#include "cg_Source.hpp"
#include "cg_SourceSnapshot.hpp"
#include "cg_constants.hpp"

namespace gris
{
class Source;
class Sources;
struct SourcesSnapshots;
class SourceLinkEnforcer;

namespace source_link_strategies
{
//==============================================================================
class Base
{
    friend SourceLinkEnforcer;
    bool mInitialized{ false };

public:
    //==============================================================================
    Base() noexcept = default;
    virtual ~Base() noexcept = default;

    Base(Base const &) = default;
    Base(Base &&) noexcept = default;

    Base & operator=(Base const &) = default;
    Base & operator=(Base &&) = default;
    //==============================================================================
    void computeParameters(Sources const & finalStates, SourcesSnapshots const & initialStates);
    void enforce(Sources & finalStates, SourcesSnapshots const & initialState, SourceIndex sourceIndex) const;
    void enforce(Sources & finalStates, SourcesSnapshots const & initialState) const;
    [[nodiscard]] SourceSnapshot computeInitialStateFromFinalState(Sources const & finalStates,
                                                                   SourcesSnapshots const & initialStates,
                                                                   SourceIndex sourceIndex) const;
    [[nodiscard]] bool isInitialized() const { return mInitialized; }
    //==============================================================================
    static std::unique_ptr<Base> make(PositionSourceLink sourceLink);
    static std::unique_ptr<Base> make(ElevationSourceLink sourceLink);

private:
    //==============================================================================
    virtual void computeParameters_implementation(Sources const & finalStates, SourcesSnapshots const & initialStates)
        = 0;
    virtual void enforce_implementation(Sources & finalStates,
                                        SourcesSnapshots const & initialStates,
                                        SourceIndex sourceIndex) const
        = 0;
    [[nodiscard]] virtual SourceSnapshot
        computeInitialStateFromFinalState_implementation(Sources const & finalStates,
                                                         SourcesSnapshots const & initialStates,
                                                         SourceIndex sourceIndex) const
        = 0;
    //==============================================================================
    JUCE_LEAK_DETECTOR(Base)

}; // class Base

//==============================================================================
// only use full to recall saved positions
class PositionIndependent final : public Base
{
    void computeParameters_implementation(Sources const &, SourcesSnapshots const &) override {}
    void enforce_implementation(Sources & finalStates,
                                SourcesSnapshots const & initialStates,
                                SourceIndex sourceIndex) const override;
    [[nodiscard]] SourceSnapshot
        computeInitialStateFromFinalState_implementation(Sources const & finalStates,
                                                         SourcesSnapshots const & initialStates,
                                                         SourceIndex sourceIndex) const override;
    //==============================================================================
    JUCE_LEAK_DETECTOR(PositionIndependent)
};

//==============================================================================
class Circular final : public Base
{
    Radians mRotation{};
    float mRadiusRatio{};
    //==============================================================================
    void computeParameters_implementation(Sources const & finalState, SourcesSnapshots const & initialStates) override;
    void enforce_implementation(Sources & finalStates,
                                SourcesSnapshots const & initialStates,
                                SourceIndex sourceIndex) const override;
    [[nodiscard]] SourceSnapshot
        computeInitialStateFromFinalState_implementation(Sources const & finalStates,
                                                         SourcesSnapshots const & initialStates,
                                                         SourceIndex sourceIndex) const override;
    //==============================================================================
    JUCE_LEAK_DETECTOR(Circular)
};

//==============================================================================
class CircularFixedRadius final : public Base
{
    Radians mRotation{};
    float mRadius{};
    //==============================================================================
    void computeParameters_implementation(Sources const & finalStates, SourcesSnapshots const & initialStates) override;
    void enforce_implementation(Sources & finalStates,
                                SourcesSnapshots const & initialStates,
                                SourceIndex sourceIndex) const override;
    [[nodiscard]] SourceSnapshot
        computeInitialStateFromFinalState_implementation(Sources const & finalStates,
                                                         SourcesSnapshots const & initialStates,
                                                         SourceIndex sourceIndex) const override;
    //==============================================================================
    JUCE_LEAK_DETECTOR(CircularFixedRadius)
};

//==============================================================================
class CircularFixedAngle final : public Base
{
    Radians mDeviationPerSource{};
    Radians mPrimarySourceFinalAngle{};
    Radians mRotation{};
    std::vector<float> mSecSourcesLengthRatio{};
    bool mSecSourcesLengthRatioInitialized{};
    std::vector<int> mOrdering{};
    //==============================================================================
    void computeParameters_implementation(Sources const & finalStates, SourcesSnapshots const & initialStates) override;
    void enforce_implementation(Sources & finalStates,
                                SourcesSnapshots const & initialStates,
                                SourceIndex sourceIndex) const override;
    [[nodiscard]] SourceSnapshot
        computeInitialStateFromFinalState_implementation(Sources const & finalStates,
                                                         SourcesSnapshots const & initialStates,
                                                         SourceIndex sourceIndex) const override;
    //==============================================================================
public:
    CircularFixedAngle ()
    {
        auto const MAX_NUMBER_OF_SOURCES { juce::JUCEApplicationBase::isStandaloneApp() ? 128 : 8};
        mSecSourcesLengthRatio.resize(MAX_NUMBER_OF_SOURCES);
        mOrdering.resize(MAX_NUMBER_OF_SOURCES);
    }

    std::vector<float> getSecSourcesLengthRatio();
    void setSecSourcesLengthRatio(std::vector<float> & secSourcesLengthRatio);
    void setSecSourcesLengthRatioInitialized();
    //==============================================================================
    JUCE_LEAK_DETECTOR(CircularFixedAngle)
};

//==============================================================================
// TODO : copy-pasted code from fixedAngle
class CircularFullyFixed final : public Base
{
    Radians mDeviationPerSource{};
    Radians mPrimarySourceFinalAngle{};
    Radians mRotation{};
    float mRadius{};
    std::vector<int> mOrdering{};
    bool mOrderingInitialized{};
    //==============================================================================
    void computeParameters_implementation(Sources const & finalStates, SourcesSnapshots const & initialStates) override;
    void enforce_implementation(Sources & finalStates,
                                SourcesSnapshots const & initialStates,
                                SourceIndex sourceIndex) const override;
    [[nodiscard]] SourceSnapshot
        computeInitialStateFromFinalState_implementation(Sources const & finalStates,
                                                         SourcesSnapshots const & initialStates,
                                                         SourceIndex sourceIndex) const override;
    //==============================================================================
public:
    CircularFullyFixed()
    {
        auto const MAX_NUMBER_OF_SOURCES { juce::JUCEApplicationBase::isStandaloneApp() ? 128 : 8};
        mOrdering.resize(MAX_NUMBER_OF_SOURCES);
    }

    std::vector<int> getOrdering();
    void setOrdering(std::vector<int> & ordering);
    void setOrderingInitialized();
    //==============================================================================
    JUCE_LEAK_DETECTOR(CircularFullyFixed)
};

//==============================================================================
class SymmetricX final : public Base
{
    juce::Point<float> mPrimarySourceFinalPosition;
    //==============================================================================
    void computeParameters_implementation(Sources const & finalStates, SourcesSnapshots const & initialStates) override;
    void enforce_implementation(Sources & finalStates,
                                SourcesSnapshots const & initialStates,
                                SourceIndex sourceIndex) const override;
    [[nodiscard]] SourceSnapshot
        computeInitialStateFromFinalState_implementation(Sources const & finalStates,
                                                         SourcesSnapshots const & initialStates,
                                                         SourceIndex sourceIndex) const override;
    //==============================================================================
    JUCE_LEAK_DETECTOR(SymmetricX)
};

//==============================================================================
class SymmetricY final : public Base
{
    juce::Point<float> mPrimarySourceFinalPosition;
    //==============================================================================
    void computeParameters_implementation(Sources const & finalStates, SourcesSnapshots const & initialStates) override;
    void enforce_implementation(Sources & finalStates,
                                SourcesSnapshots const & initialStates,
                                SourceIndex sourceIndex) const override;
    [[nodiscard]] SourceSnapshot
        computeInitialStateFromFinalState_implementation(Sources const & finalStates,
                                                         SourcesSnapshots const & initialStates,
                                                         SourceIndex sourceIndex) const override;
    //==============================================================================
    JUCE_LEAK_DETECTOR(SymmetricY)
};

//==============================================================================
class PositionDeltaLock final : public Base
{
    juce::Point<float> mDelta;
    //==============================================================================
    void computeParameters_implementation(Sources const & finalStates, SourcesSnapshots const & initialStates) override;
    void enforce_implementation(Sources & finalStates,
                                SourcesSnapshots const & initialStates,
                                SourceIndex sourceIndex) const override;
    [[nodiscard]] SourceSnapshot
        computeInitialStateFromFinalState_implementation(Sources const & finalStates,
                                                         SourcesSnapshots const & initialStates,
                                                         SourceIndex sourceIndex) const override;
    //==============================================================================
    JUCE_LEAK_DETECTOR(PositionDeltaLock)
};

//==============================================================================
// only usefuLl to recall saved positions
class ElevationIndependent final : public Base
{
    void computeParameters_implementation(Sources const &, SourcesSnapshots const &) override {}
    void enforce_implementation(Sources & finalStates,
                                SourcesSnapshots const & initialStates,
                                SourceIndex sourceIndex) const override;
    [[nodiscard]] SourceSnapshot
        computeInitialStateFromFinalState_implementation(Sources const & finalStates,
                                                         SourcesSnapshots const & initialStates,
                                                         SourceIndex sourceIndex) const override;
    //==============================================================================
    JUCE_LEAK_DETECTOR(ElevationIndependent)
};

//==============================================================================
class FixedElevation final : public Base
{
    Radians mElevation{};
    //==============================================================================
    void computeParameters_implementation(Sources const & finalStates, SourcesSnapshots const & initialStates) override;
    void enforce_implementation(Sources & finalStates,
                                SourcesSnapshots const & initialStates,
                                SourceIndex sourceIndex) const override;
    [[nodiscard]] SourceSnapshot
        computeInitialStateFromFinalState_implementation(Sources const & finalStates,
                                                         SourcesSnapshots const & initialStates,
                                                         SourceIndex sourceIndex) const override;
    //==============================================================================
    JUCE_LEAK_DETECTOR(FixedElevation)
};

//==============================================================================
class LinearMin final : public Base
{
    static constexpr Radians ELEVATION_DIFF{ -MAX_ELEVATION / 3.0f * 2.0f };
    Radians mBaseElevation{};
    Radians mElevationPerSource{};
    //==============================================================================
    void computeParameters_implementation(Sources const & sources, SourcesSnapshots const & snapshots) override;
    void enforce_implementation(Sources & finalStates,
                                SourcesSnapshots const & initialStates,
                                SourceIndex sourceIndex) const override;
    [[nodiscard]] SourceSnapshot
        computeInitialStateFromFinalState_implementation(Sources const & finalStates,
                                                         SourcesSnapshots const & initialStates,
                                                         SourceIndex sourceIndex) const override;
    //==============================================================================
    JUCE_LEAK_DETECTOR(LinearMin)
};

//==============================================================================
class LinearMax final : public Base
{
    static constexpr Radians ELEVATION_DIFF{ MAX_ELEVATION / 3.0f * 2.0f };
    Radians mBaseElevation{};
    Radians mElevationPerSource{};
    //==============================================================================
    void computeParameters_implementation(Sources const & sources, SourcesSnapshots const & snapshots) override;
    void enforce_implementation(Sources & finalStates,
                                SourcesSnapshots const & initialStates,
                                SourceIndex sourceIndex) const override;
    [[nodiscard]] SourceSnapshot
        computeInitialStateFromFinalState_implementation(Sources const & finalStates,
                                                         SourcesSnapshots const & initialStates,
                                                         SourceIndex sourceIndex) const override;
    //==============================================================================
    JUCE_LEAK_DETECTOR(LinearMax)
};

//==============================================================================
class ElevationDeltaLock final : public Base
{
    Radians mDelta;
    //==============================================================================
    void computeParameters_implementation(Sources const & sources, SourcesSnapshots const & snapshots) override;
    void enforce_implementation(Sources & finalStates,
                                SourcesSnapshots const & initialStates,
                                SourceIndex sourceIndex) const override;
    [[nodiscard]] SourceSnapshot
        computeInitialStateFromFinalState_implementation(Sources const & finalStates,
                                                         SourcesSnapshots const & initialStates,
                                                         SourceIndex sourceIndex) const override;
    //==============================================================================
    JUCE_LEAK_DETECTOR(ElevationDeltaLock)
};

} // namespace source_link_strategies

} // namespace gris
