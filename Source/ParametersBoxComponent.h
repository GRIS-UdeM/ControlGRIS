# pragma once

#include "../JuceLibraryCode/JuceHeader.h"
#include "GrisLookAndFeel.h"
#include "Source.h"

class ParametersBoxComponent : public Component,
                               public Button::Listener,
                               public Slider::Listener
{
public:
    ParametersBoxComponent();
    ~ParametersBoxComponent();

    void buttonClicked(Button *button) override;
    void sliderValueChanged(Slider *slider) override;
    void paint(Graphics&) override;
    void resized() override;

    void setSelectedSource(Source *source);
    void setDistanceEnabled(bool shouldBeEnabled);

    struct Listener
    {
        virtual ~Listener() {}

        virtual void parameterChanged(int parameterId, double value) = 0;
    };

    void addListener(Listener* l) { listeners.add (l); }
    void removeListener(Listener* l) { listeners.remove (l); }

private:
    ListenerList<Listener> listeners;

    GrisLookAndFeel mGrisFeel;

    bool m_distanceEnabled;

    Source *selectedSource;

    Label           azimuthLabel;
    ToggleButton    azimuthLinkButton;
    Slider          azimuthSlider;

    Label           elevationLabel;
    ToggleButton    elevationLinkButton;
    Slider          elevationSlider;

    Label           distanceLabel;
    ToggleButton    distanceLinkButton;
    Slider          distanceSlider;

    //-------------------------------------

    Label           xLabel;
    ToggleButton    xLinkButton;
    Slider          xSlider;

    Label           yLabel;
    ToggleButton    yLinkButton;
    Slider          ySlider;

    //-------------------------------------

    Label           azimuthSpanLabel;
    ToggleButton    azimuthSpanLinkButton;
    Slider          azimuthSpanSlider;

    Label           elevationSpanLabel;
    ToggleButton    elevationSpanLinkButton;
    Slider          elevationSpanSlider;

    ToggleButton    activatorXY;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ParametersBoxComponent)
};
