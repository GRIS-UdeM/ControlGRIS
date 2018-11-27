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

#include "GrisLookAndFeel.h"
#include "Source.h"

//==============================================================================
class FieldComponent : public Component
{
public:
    FieldComponent();
    ~FieldComponent();

 	void mouseUp (const MouseEvent &event);

    void setSources(Source *sources, int numberOfSources);
    void setSelectedSource(int selectedId);

    struct Listener
    {
        virtual ~Listener() {}

        virtual void sourcePositionChanged(int sourceId) = 0;
    };

    void addListener(Listener* l) { listeners.add (l); }
    void removeListener(Listener* l) { listeners.remove (l); }

    ListenerList<Listener> listeners;

protected:
    GrisLookAndFeel mGrisFeel;

    Source *m_sources;
    int m_numberOfSources;
    int m_selectedSourceId;

private:

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (FieldComponent)
};

//==============================================================================
class MainFieldComponent : public FieldComponent
{
public:
    MainFieldComponent();
    ~MainFieldComponent();
    
    void paint (Graphics&);
	
	void mouseDown (const MouseEvent &event);
 	void mouseDrag (const MouseEvent &event);

    void setDrawElevation(bool shouldDrawElevation);

private:
    inline double degreeToRadian(float degree) { return (degree / 360.0 * 2.0 * M_PI); }
    Point <float> degreeToXy(Point <float> p, int p_iFieldWidth);
    Point <float> xyToDegree(Point <float> p, int p_iFieldWidth);

    bool m_drawElevation; // true for elevation, false for distance

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MainFieldComponent)
};

//==============================================================================
class ElevationFieldComponent : public FieldComponent
{
public:
    ElevationFieldComponent();
    ~ElevationFieldComponent();
    
    void paint (Graphics&);
	
	void mouseDown (const MouseEvent &event);
 	void mouseDrag (const MouseEvent &event);

private:

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ElevationFieldComponent)
};
