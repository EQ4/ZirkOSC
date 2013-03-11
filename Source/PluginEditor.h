/*
  ==============================================================================

    This file was auto-generated by the Introjucer!

    It contains the basic startup code for a Juce application.

  ==============================================================================
*/

#ifndef __PLUGINEDITOR_H_4624BC76__
#define __PLUGINEDITOR_H_4624BC76__

#include "../JuceLibraryCode/JuceHeader.h"
#include "PluginProcessor.h"

//==============================================================================
/**
*/
class ZirkOscjuceAudioProcessorEditor  : public AudioProcessorEditor,
                                         public ButtonListener,
                                         public SliderListener,
                                         public Timer,
                                         public MouseListener,
                                         public TextEditorListener,
                                         public ComboBoxListener
                                         
{
public:
    ZirkOscjuceAudioProcessorEditor (ZirkOscjuceAudioProcessor* ownerFilter);
    ~ZirkOscjuceAudioProcessorEditor();

    //==============================================================================
    // This is just a standard Juce paint method...
    void paint (Graphics& g);

    void buttonClicked (Button* button);
    void sliderValueChanged (Slider* slider);
    void timerCallback();
    void mouseDown (const MouseEvent &event);
 	void mouseDrag (const MouseEvent &event);
 	void mouseUp (const MouseEvent &event);
    void textEditorReturnKeyPressed (TextEditor &editor);
    void textEditorFocusLost (TextEditor &editor);
    void comboBoxChanged (ComboBox* comboBoxThatHasChanged);

private:
    
    enum ConstrainType
    {
        Independant = 1,
        FixedRadius = 2,
        FixedAngles = 3,
        FullyFixed  = 4,
        DeltaLocked = 5
    };
    AudioPlayHead::CurrentPositionInfo lastDisplayedPosition;
    int getSourceFromPosition(Point<float> p );
    Point <float> mSourcePoint;
	uint32_t mChannel;
	uint32_t mChannelCount;
	uint32_t mOscPort;
	uint32_t mActive;

    int selectedConstrain;

    TextButton* button1;
    Slider gainSlider;
    Slider azimuthSlider;
    Slider azimuthSpanSlider;
    Slider elevationSlider;
    Slider elevationSpanSlider;

    Label azimuthSpanLabel;
    Label elevationSpanLabel;
    Label azimuthLabel;
    Label gainLabel;
    Label elevationLabel;
    Label OSCPortLabel;
    Label NbrSourceLabel;
    Label channelNumberLabel;

    TextEditor OSCPortTextEditor;
    TextEditor NbrSourceTextEditor;
    TextEditor channelNumberTextEditor;
    
    ComboBox mouvementConstrain;

    bool draggableSource;

    ZirkOscjuceAudioProcessor* getProcessor() const
    {
        return static_cast <ZirkOscjuceAudioProcessor*> (getAudioProcessor());
    }

    /*Painting functions*/

    void paintSpanArc (Graphics& g);
    void paintSourcePoint (Graphics& g);
    void paintWallCircle (Graphics& g);
    void paintCenterDot (Graphics& g);
    void paintAzimuthLine (Graphics& g);
    void paintZenithCircle (Graphics& g);
    void paintCrosshairs (Graphics& g);
    void paintCoordLabels (Graphics& g);

    /*Conversion functions*/

    Point <float> domeToScreen (Point <float>);
    Point <float> screenToDome (Point <float>);
    inline float degreeToRadian (float);
    inline float radianToDegree (float);
    void moveSourcesWithDelta(Point<float>);

};

float PercentToHR(float , float , float );
float HRToPercent(float , float , float );

#endif  // __PLUGINEDITOR_H_4624BC76__
