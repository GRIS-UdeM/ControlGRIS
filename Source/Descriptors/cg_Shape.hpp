/*
 This file is part of ControlGris.

 Developers: Hicheme BEN GAIED, Gaël LANE LÉPINE

 ControlGris is free software: you can redistribute it and/or modify
 it under the terms of the GNU Lesser General Public License as
 published by the Free Software Foundation, either version 3 of the
 License, or (at your option) any later version.

 ControlGris is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU Lesser General Public License for more details.

 You should have received a copy of the GNU Lesser General Public
 License along with ControlGris.  If not, see
 <http://www.gnu.org/licenses/>.
*/

//==============================================================================

#pragma once

#include "cg_Descriptors.hpp"

namespace gris
{
//==============================================================================
class ShapeD : public Descriptor
{
public:
    //==============================================================================
    void reset() override
    {
        mShape.reset(new fluid::algorithm::SpectralShape(fluid::FluidDefaultAllocator()));
        mStft.reset(new fluid::algorithm::STFT{ WINDOW_SIZE, FFT_SIZE, HOP_SIZE });
    }

    void shapeProcess(fluid::RealVector & magnitude, fluid::RealVector & shapeDesc, double sampleRate)
    {
        mShape->processFrame(magnitude,
                             shapeDesc,
                             sampleRate,
                             MIN_FREQ,
                             MAX_FREQ,
                             0.95,
                             true,
                             true,
                             fluid::FluidDefaultAllocator());
    }

    fluid::algorithm::SpectralShape * getShape() const { return mShape.get(); }

    fluid::RealVector process(fluid::RealMatrix & matrix, fluid::algorithm::MultiStats & stats)
    {
        return computeStats(matrix, stats);
    }

    fluid::RealVectorView calculateWindow(fluid::RealVector & padded, int & i)
    {
        return padded(fluid::Slice(i * HOP_SIZE, WINDOW_SIZE));
    }

    fluid::RealVector calculatePadded(fluid::RealVector in)
    {
        return in.size() + WINDOW_SIZE + HOP_SIZE;
    }

    fluid::index calculateFrames(fluid::RealVector padded)
    {
        return static_cast<fluid::index>(floor((padded.size() - WINDOW_SIZE) / HOP_SIZE));
    }

    fluid::Slice paddedValue(fluid::RealVector in) { return fluid::Slice(HALF_WINDOW, in.size()); }

    void setFrame(fluid::ComplexVector & frame) { frame.resize(NBINS); }

    void setMagnitude(fluid::RealVector & magnitude) { magnitude.resize(NBINS); }

    //==============================================================================
    // Stft stuff
    // Second argument is output
    void stftProcess(fluid::RealVectorView & window, fluid::ComplexVector & frame)
    {
        mStft->processFrame(window, frame);
    }

    // Second argument is output
    void stftMagnitude(fluid::ComplexVector & frame, fluid::RealVector & magnitude)
    {
        mStft->magnitude(frame, magnitude);
    }

private:
    //==============================================================================
    void init() override {}
    double getValue() override { return 0; }

    std::unique_ptr<fluid::algorithm::SpectralShape> mShape;
    std::unique_ptr<fluid::algorithm::STFT> mStft;

    static constexpr fluid::index NBINS = 4097;
    static constexpr fluid::index HOP_SIZE = 512;
    static constexpr fluid::index WINDOW_SIZE = 4096;
    static constexpr fluid::index HALF_WINDOW = WINDOW_SIZE / 2;
    static constexpr fluid::index FFT_SIZE = 8192;
    static constexpr double MIN_FREQ = 20.0;
    static constexpr double MAX_FREQ = 20000.0;

    //==============================================================================
    JUCE_LEAK_DETECTOR(ShapeD)
};
} // namespace gris
