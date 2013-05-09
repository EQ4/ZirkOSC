/*
 ==============================================================================

 Developer : Ludovic LAFFINEUR (ludovic.laffineur@gmail.com)

 Lexic :
 - all parameter preceeded by HR are Human Readable
 - parameter without HR are in percent
 - Points in spheric coord  : x -> azimuth, y -> elevation

 ==============================================================================
 */

#include "PluginProcessor.h"
#include "PluginEditor.h"
#include <cstdlib>
#include <string>
#include <string.h>
#include <sstream>
#include <istream>
#include <math.h>

using namespace std;


/*!
*  \param ownerFilter : the processor processor
*/

ZirkOscjuceAudioProcessorEditor::ZirkOscjuceAudioProcessorEditor (ZirkOscjuceAudioProcessor* ownerFilter)
:   AudioProcessorEditor (ownerFilter),
_GainSlider (ZirkOSC_Gain_name[0]),
_AzimuthSlider(ZirkOSC_AzimSpan_name[0]),
_ElevationSlider(ZirkOSC_ElevSpan_name[0]),
_AzimuthLabel(ZirkOSC_Azim_name[0]),
_ElevationSpanSlider(ZirkOSC_ElevSpan_name[0]),
_AzimuthSpanLabel(ZirkOSC_AzimSpan_name[0]),
_ElevationLabel(ZirkOSC_Elev_name[0]),
_GainLabel(ZirkOSC_Gain_name[0]),
_OscPortTextEditor("OscPort"),
_NbrSourceTextEditor("NbrSource"),
_OscPortLabel("OscPort"),
_NbrSourceLabel("NbrSources"),
_ChannelNumberTextEditor("channelNbr"),
_ChannelNumberLabel("channelNbr"),
_MouvementConstrain("MouvemntContrain"),
_LinkSpanButton("Link Span"),
_OscPortIncomingIPadLabel("OSCIpadInco"),
_OscPortIncomingIPadTextEditor("OSCIpadIncoTE"),
_OscPortOutgoingIPadLabel("OSCPortOutgoingIPad"),
_OscPortOutgoingIPadTextEditor("OSCPortOutgoingIPadTE"),
_OscAdressIPadTextEditor("ipaddress"),
_OscAdressIPadTextLabel("ipadadressLabel")

{
    printf("ZirkOscjuceAudioProcessorEditor() constructor");
    // This is where our plugin's editor size is set.
    setSize (ZirkOSC_Window_Width, ZirkOSC_Window_Height);
    /*  button1 = new TextButton("Button1");*/

    // button1->setBounds (20, 70, 260, 20);*/
    //_SourcePoint.setX(0.0f);
    //_SourcePoint.setY (0.0f);


    setSliderAndLabel(20,340, 360, 20, "Gain", &_GainSlider,&_GainLabel, 0.0, 1.0);
    addAndMakeVisible(&_GainSlider);
    addAndMakeVisible(&_GainLabel);

    setSliderAndLabel(20,380, 360, 20, "Azimuth", &_AzimuthSlider ,&_AzimuthLabel, ZirkOSC_Azim_Min, ZirkOSC_Azim_Max);
    addAndMakeVisible(&_AzimuthSlider);
    addAndMakeVisible(&_AzimuthLabel);


    setSliderAndLabel(20, 420, 360, 20, "Elevation", &_ElevationSlider, &_ElevationLabel, ZirkOSC_Elev_Min, ZirkOSC_Elev_Max);
    addAndMakeVisible(&_ElevationSlider);
    addAndMakeVisible(&_ElevationLabel);


    setSliderAndLabel(20, 460, 360, 20, "Elev. Sp.", &_ElevationSpanSlider, &_ElevationSpanLabel, ZirkOSC_ElevSpan_Min, ZirkOSC_ElevSpan_Max);
    addAndMakeVisible(&_ElevationSpanSlider);
    addAndMakeVisible(&_ElevationSpanLabel);

    setSliderAndLabel(20, 500, 360, 20, "Azim. Sp.", &_AzimuthSpanSlider, &_AzimuthSpanLabel, ZirkOSC_AzimSpan_Min, ZirkOSC_AzimSpan_Max);
    addAndMakeVisible(&_AzimuthSpanSlider);
    addAndMakeVisible(&_AzimuthSpanLabel);

    _NbrSourceLabel.setText("Nbr Sources", false);
    _NbrSourceLabel.setBounds(ZirkOSC_Window_Width-80 , 10, 80, 25);
    _NbrSourceTextEditor.setBounds(ZirkOSC_Window_Width-75 , 30, 60, 25);
    _NbrSourceTextEditor.setText(String(getProcessor()->nbrSources));
    addAndMakeVisible(&_NbrSourceLabel);
    addAndMakeVisible(&_NbrSourceTextEditor);


    _OscPortLabel.setText("ZKM Port OSC", false);
    _OscPortLabel.setBounds(ZirkOSC_Window_Width-80 , 60, 80, 25);
    _OscPortTextEditor.setBounds(ZirkOSC_Window_Width-75 , 80, 60, 25);
    _OscPortTextEditor.setText(String(getProcessor()->moscPort));
    addAndMakeVisible(&_OscPortLabel);
    addAndMakeVisible(&_OscPortTextEditor);

    _ChannelNumberLabel.setText("Channel nbr", false);
    _ChannelNumberLabel.setBounds(ZirkOSC_Window_Width-80 , 110, 80, 25);
    _ChannelNumberTextEditor.setBounds(ZirkOSC_Window_Width-75 , 130, 60, 25);
    _ChannelNumberTextEditor.setText(String(getProcessor()->tabSource[getProcessor()->selectedSource].getChannel()));
    addAndMakeVisible(&_ChannelNumberLabel);
    addAndMakeVisible(&_ChannelNumberTextEditor);

    _OscPortIncomingIPadLabel.setText("Inc. port", false);
    _OscPortIncomingIPadLabel.setBounds(ZirkOSC_Window_Width-80 , 160, 80, 25);
    _OscPortIncomingIPadTextEditor.setBounds(ZirkOSC_Window_Width-75 , 180, 60, 25);
    _OscPortIncomingIPadTextEditor.setText(String(getProcessor()->mOscPortIpadIncoming));
    addAndMakeVisible(&_OscPortIncomingIPadLabel);
    addAndMakeVisible(&_OscPortIncomingIPadTextEditor);
    
    _OscPortOutgoingIPadLabel.setText("Out. port", false);
    _OscPortOutgoingIPadLabel.setBounds(ZirkOSC_Window_Width-80 , 210, 80, 25);
    _OscPortOutgoingIPadTextEditor.setBounds(ZirkOSC_Window_Width-75 , 230, 60, 25);
    _OscPortOutgoingIPadTextEditor.setText(String(getProcessor()->mOscPortIpadOutgoing));
    addAndMakeVisible(&_OscPortOutgoingIPadLabel);
    addAndMakeVisible(&_OscPortOutgoingIPadTextEditor);
    
    _OscAdressIPadTextLabel.setText("IP add. iPad", false);
    _OscAdressIPadTextLabel.setBounds(ZirkOSC_Window_Width-80 , 260, 80, 25);
    _OscAdressIPadTextEditor.setBounds(ZirkOSC_Window_Width-75 , 280, 60, 25);
    _OscAdressIPadTextEditor.setText(String(getProcessor()->mOscAddressIpad));
    addAndMakeVisible(&_OscAdressIPadTextLabel);
    addAndMakeVisible(&_OscAdressIPadTextEditor);
    
    _MouvementConstrain.setSize(50, 20);
    _MouvementConstrain.setBounds(80, 540, 220, 25);

    _MouvementConstrain.addItem("Independant",   Independant);
    _MouvementConstrain.addItem("Circular",      Circular);
    _MouvementConstrain.addItem("Fixed Radius",  FixedRadius);
    _MouvementConstrain.addItem("Fixed Angle",   FixedAngles);
    _MouvementConstrain.addItem("Fully fixed",   FullyFixed);
    _MouvementConstrain.addItem("Delta Lock",    DeltaLocked);

    _MouvementConstrain.setSelectedId(getProcessor()->getSelectedConstrain());
    //_selectedConstrain = Independant;
    _MouvementConstrain.addListener(this);
    addAndMakeVisible(&_MouvementConstrain);
    
    _LinkSpanButton.setBounds(300, 300, 80, 30);
    addAndMakeVisible(&_LinkSpanButton);
    _LinkSpanButton.addListener(this);

    _ChannelNumberTextEditor.addListener(this);
    _OscPortTextEditor.addListener(this);
    _NbrSourceTextEditor.addListener(this);
    _ElevationSlider.addListener(this);
    _AzimuthSlider.addListener(this);
    _GainSlider.addListener(this);
    _ElevationSpanSlider.addListener(this);
    _AzimuthSpanSlider.addListener(this);
    _OscPortOutgoingIPadTextEditor.addListener(this);
    _OscPortIncomingIPadTextEditor.addListener(this);
    _OscAdressIPadTextEditor.addListener(this);
    this->setFocusContainer(true);
    startTimer (50);
}

ZirkOscjuceAudioProcessorEditor::~ZirkOscjuceAudioProcessorEditor()
{
    //stopTimer();
}


//Automatic function to set label and Slider

/*!
* \param x : x position top left
* \param y : y position top left
* \param width : slider's and label's width
* \param height : slider's and label's heigth
* \param labelText : label's text
* \param slider : slider
* \param label : label
* \param min : minimum value of the slider
* \param max : maximum value of the slider
*/

void ZirkOscjuceAudioProcessorEditor::setSliderAndLabel(int x, int y, int width, int height, String labelText, Slider* slider, Label* label, float min, float max){
    slider->setBounds (x+60, y, width-60, height);
    slider->setTextBoxStyle(Slider::TextBoxRight, false, 80, 20);
    label->setText(labelText, false);
    label->setBounds(x, y, width-300, height);
    slider->setRange (min, max, 0.01);
}

void ZirkOscjuceAudioProcessorEditor::paint (Graphics& g)
{
    g.fillAll (Colours::lightgrey);
    paintWallCircle(g);
    paintCrosshairs(g);
    paintCoordLabels(g);
    paintCenterDot(g);
    paintSpanArc(g);
    paintSourcePoint(g);
    paintAzimuthLine(g);
    paintZenithCircle(g);
}


//Drawing Span Arc
void ZirkOscjuceAudioProcessorEditor::paintSpanArc (Graphics& g){
    ZirkOscjuceAudioProcessor* ourProcessor = getProcessor();
    int selectedSource = ourProcessor->selectedSource;
    float HRAzim = PercentToHR(ourProcessor->tabSource[selectedSource].getAzimuth(), ZirkOSC_Azim_Min, ZirkOSC_Azim_Max),
    HRElev = PercentToHR(ourProcessor->tabSource[selectedSource].getElevation(), ZirkOSC_Elev_Min, ZirkOSC_Elev_Max),
    HRElevSpan = PercentToHR(ourProcessor->tabSource[selectedSource].getElevationSpan(), ZirkOSC_ElevSpan_Min, ZirkOSC_ElevSpan_Max),
    HRAzimSpan = PercentToHR(ourProcessor->tabSource[selectedSource].getAzimuthSpan(), ZirkOSC_AzimSpan_Min, ZirkOSC_AzimSpan_Max);

    Point<float> maxElev = {HRAzim, HRElev+HRElevSpan/2};
    Point<float> minElev = {HRAzim, HRElev-HRElevSpan/2};

    if(minElev.getY() < ZirkOSC_ElevSpan_Min){
        maxElev.setY(maxElev.getY()+ ZirkOSC_ElevSpan_Min-minElev.getY());
        minElev.setY(ZirkOSC_ElevSpan_Min);
    }

    Point<float> screenMaxElev = domeToScreen(maxElev);
    Point<float> screenMinElev = domeToScreen(minElev);
    float maxRadius = sqrtf(screenMaxElev.getX()*screenMaxElev.getX() + screenMaxElev.getY()*screenMaxElev.getY());
    float minRadius = sqrtf(screenMinElev.getX()*screenMinElev.getX() + screenMinElev.getY()*screenMinElev.getY());
    //drawing the path for spanning
    Path myPath;
    myPath.startNewSubPath(ZirkOSC_Center_X+screenMinElev.getX(),ZirkOSC_Center_Y+screenMinElev.getY());

    //half first arc center
    myPath.addCentredArc(ZirkOSC_Center_X, ZirkOSC_Center_Y, minRadius, minRadius, 0.0, degreeToRadian(-HRAzim), degreeToRadian(-HRAzim + HRAzimSpan/2 ));

    if (maxElev.getY()> ZirkOSC_ElevSpan_Max) { // if we are over the top of the dome we draw the adjacent angle

        myPath.addCentredArc(ZirkOSC_Center_X, ZirkOSC_Center_Y, maxRadius, maxRadius, 0.0, M_PI+degreeToRadian(-HRAzim + HRAzimSpan/2), M_PI+degreeToRadian(-HRAzim - HRAzimSpan/2 ));
    }
    else{
        myPath.addCentredArc(ZirkOSC_Center_X, ZirkOSC_Center_Y, maxRadius, maxRadius, 0.0, degreeToRadian(-HRAzim+HRAzimSpan/2), degreeToRadian(-HRAzim-HRAzimSpan/2 ));
    }
    myPath.addCentredArc(ZirkOSC_Center_X, ZirkOSC_Center_Y, minRadius, minRadius, 0.0, degreeToRadian(-HRAzim-HRAzimSpan/2), degreeToRadian(-HRAzim ));
    myPath.closeSubPath();
    g.setColour(Colours::lightgrey);
    g.fillPath(myPath);
    g.setColour(Colours::black);
    g.strokePath(myPath, PathStrokeType::curved);
}

void ZirkOscjuceAudioProcessorEditor::paintSourcePoint (Graphics& g){
    Point<float> screen;
    float HRAzim, HRElev;
    for (int i=0; i<getProcessor()->nbrSources; i++) {
        HRAzim = PercentToHR(getProcessor()->tabSource[i].getAzimuth(), ZirkOSC_Azim_Min, ZirkOSC_Azim_Max);
        HRElev = PercentToHR(getProcessor()->tabSource[i].getElevation(), ZirkOSC_Elev_Min, ZirkOSC_Elev_Max);
        screen = domeToScreen(Point<float> (HRAzim, HRElev));
        g.drawEllipse(ZirkOSC_Center_X + screen.getX()-4, ZirkOSC_Center_Y + screen.getY()-4, 8, 8,2);
        if(!_DraggableSource){
             g.drawText(String(getProcessor()->tabSource[i].getChannel()), ZirkOSC_Center_X + screen.getX()+6, ZirkOSC_Center_Y + screen.getY()-2, 40, 10, Justification::centred, false);
        }
       
    }
}

void ZirkOscjuceAudioProcessorEditor::paintWallCircle (Graphics& g){
    g.setColour(Colours::white);
    g.fillEllipse(10.0f, 30.0f, ZirkOSC_DomeRadius * 2, ZirkOSC_DomeRadius * 2);
    g.setColour(Colours::black);
    g.drawEllipse(10.0f, 30.0f, ZirkOSC_DomeRadius * 2, ZirkOSC_DomeRadius * 2, 1.0f);
}

void ZirkOscjuceAudioProcessorEditor::paintCenterDot (Graphics& g){
    g.setColour(Colours::red);
    g.fillEllipse(ZirkOSC_Center_X - 3.0f, ZirkOSC_Center_Y - 3.0f, 6.0f,6.0f );
}

void ZirkOscjuceAudioProcessorEditor::paintAzimuthLine (Graphics& g){
    ZirkOscjuceAudioProcessor *ourProcessor = getProcessor();
    int selectedSource = ourProcessor->selectedSource;
    g.setColour(Colours::red);
    float HRAzim = PercentToHR(ourProcessor->tabSource[selectedSource].getAzimuth(), ZirkOSC_Azim_Min, ZirkOSC_Azim_Max);
    float HRElev = PercentToHR(ourProcessor->tabSource[selectedSource].getElevation(), ZirkOSC_Elev_Min, ZirkOSC_Elev_Max);
    //float HRAzim = (float) _AzimuthSlider.getValue();
    //float HRElev  = (float) _ElevationSlider.getValue();
    Point <float> screen = domeToScreen(Point<float>(HRAzim,HRElev));
    g.drawLine(ZirkOSC_Center_X, ZirkOSC_Center_Y, ZirkOSC_Center_X + screen.getX(), ZirkOSC_Center_Y + screen.getY() );
}

void ZirkOscjuceAudioProcessorEditor::paintZenithCircle (Graphics& g){
    ZirkOscjuceAudioProcessor *ourProcessor = getProcessor();
    int selectedSource = ourProcessor->selectedSource;
    float HRAzim = PercentToHR(ourProcessor->tabSource[selectedSource].getAzimuth(), ZirkOSC_Azim_Min, ZirkOSC_Azim_Max);
    float HRElev = PercentToHR(ourProcessor->tabSource[selectedSource].getElevation(), ZirkOSC_Elev_Min, ZirkOSC_Elev_Max);
    Point <float> screen = domeToScreen(Point<float>(HRAzim,HRElev));
    float raduisZenith = sqrtf(screen.getX()*screen.getX() + screen.getY()*screen.getY());
    g.drawEllipse(ZirkOSC_Center_X-raduisZenith, ZirkOSC_Center_Y-raduisZenith, raduisZenith*2, raduisZenith*2, 1.0);
}

void ZirkOscjuceAudioProcessorEditor::paintCrosshairs (Graphics& g){
    g.setColour(Colours::grey);
    float radianAngle=0.0f;
    float fraction = 0.9f;
    Point<float> axis = Point<float>();
    for (int i= 0; i<ZirkOSC_NumMarks; i++) {
        radianAngle = degreeToRadian(ZirkOSC_MarksAngles[i]);
        axis = {cosf(radianAngle), sinf(radianAngle)};
        g.drawLine(ZirkOSC_Center_X+(ZirkOSC_DomeRadius*fraction)*axis.getX(), ZirkOSC_Center_Y+(ZirkOSC_DomeRadius*fraction)*axis.getY(),ZirkOSC_Center_X+(ZirkOSC_DomeRadius)*axis.getX(), ZirkOSC_Center_Y+(ZirkOSC_DomeRadius)*axis.getY(),1.0f);
    }
}

void ZirkOscjuceAudioProcessorEditor::paintCoordLabels (Graphics& g){
    g.setColour(Colours::black);
    g.drawLine(ZirkOSC_Center_X - ZirkOSC_DomeRadius, ZirkOSC_Center_Y, ZirkOSC_Center_X + ZirkOSC_DomeRadius, ZirkOSC_Center_Y ,0.5f);
    g.drawLine(ZirkOSC_Center_X , ZirkOSC_Center_Y - ZirkOSC_DomeRadius, ZirkOSC_Center_X , ZirkOSC_Center_Y + ZirkOSC_DomeRadius,0.5f);
}


/*Conversion function*/

/*!
* \param p : Point <float> (Azimuth,Elevation) in degree
*/
Point <float> ZirkOscjuceAudioProcessorEditor::domeToScreen (Point <float> p){
    float x,y;
    x = -ZirkOSC_DomeRadius * sinf(degreeToRadian(p.getX())) * cosf(degreeToRadian(p.getY()));
    y = -ZirkOSC_DomeRadius * cosf(degreeToRadian(p.getX())) * cosf(degreeToRadian(p.getY()));
    return Point <float> (x, y);
}

/*!
 * \param degree : degree value.
 * \return radian value
 */

inline float ZirkOscjuceAudioProcessorEditor::degreeToRadian (float degree){
    return ((degree/360.0f)*2*3.1415);
}

/*!
 * \param radian : radian value.
 * \return degree value
 */
inline float ZirkOscjuceAudioProcessorEditor::radianToDegree(float radian){
    return (radian/(2*3.1415)*360.0f);
}

/*!
 * Function called every 50ms to refresh value from Host
 * repaint only if the user doesn't move any source (_DraggableSource)
 */
void ZirkOscjuceAudioProcessorEditor::timerCallback(){
    ZirkOscjuceAudioProcessor* ourProcessor = getProcessor();
    int selectedSource = ourProcessor->selectedSource;
    _GainSlider.setValue (ourProcessor->tabSource[selectedSource].getGain(), dontSendNotification);

    float HRValue = PercentToHR(ourProcessor->tabSource[selectedSource].getAzimuth(), ZirkOSC_Azim_Min, ZirkOSC_Azim_Max);
    _AzimuthSlider.setValue(HRValue,dontSendNotification);

    HRValue = PercentToHR(ourProcessor->tabSource[selectedSource].getElevation(), ZirkOSC_Elev_Min, ZirkOSC_Elev_Max);
    _ElevationSlider.setValue(HRValue,dontSendNotification);

    HRValue = PercentToHR(ourProcessor->tabSource[selectedSource].getAzimuthSpan(), ZirkOSC_AzimSpan_Min, ZirkOSC_AzimSpan_Max);
    _AzimuthSpanSlider.setValue(HRValue,dontSendNotification);

    HRValue = PercentToHR(ourProcessor->tabSource[selectedSource].getElevationSpan(), ZirkOSC_ElevSpan_Min, ZirkOSC_ElevSpan_Max);
    _ElevationSpanSlider.setValue(HRValue,dontSendNotification);
    if (ourProcessor->refreshGui){
        refreshGui();
        ourProcessor->refreshGui=false;
    }
    //getProcessor()->sendOSCValues();
    if(!_DraggableSource){
          repaint();
    }
}

void ZirkOscjuceAudioProcessorEditor::refreshGui(){
    ZirkOscjuceAudioProcessor* ourProcessor = getProcessor();
    int selectedSource = ourProcessor->selectedSource;
    _OscPortTextEditor.setText(String(ourProcessor->moscPort));
    _NbrSourceTextEditor.setText(String(ourProcessor->nbrSources));
    _ChannelNumberTextEditor.setText(String(ourProcessor->tabSource[selectedSource].getChannel()));
    _OscPortIncomingIPadTextEditor.setText(ourProcessor->mOscPortIpadIncoming);
    _OscPortOutgoingIPadTextEditor.setText(ourProcessor->mOscPortIpadOutgoing);
    _OscAdressIPadTextEditor.setText(ourProcessor->mOscAddressIpad);
}

void ZirkOscjuceAudioProcessorEditor::buttonClicked (Button* button)
{
    if(button == &_LinkSpanButton){
        _LinkSpan = _LinkSpanButton.getToggleState();
    }
}

float PercentToHR(float percent, float min, float max){
    return percent*(max-min)+min;
}

float HRToPercent(float HRValue, float min, float max){
    return (HRValue-min)/(max-min);
}

void ZirkOscjuceAudioProcessorEditor::sliderValueChanged (Slider* slider)
{
    ZirkOscjuceAudioProcessor* ourProcessor = getProcessor();
    int selectedSource = ourProcessor->selectedSource;
    float percentValue=0;
    if (slider == &_GainSlider) {
        ourProcessor->beginParameterChangeGesture(ZirkOscjuceAudioProcessor::ZirkOSC_Gain_Param+ (selectedSource*7) );
        ourProcessor->setParameterNotifyingHost (ZirkOscjuceAudioProcessor::ZirkOSC_Gain_Param + (selectedSource*7),
                                                 (float) _GainSlider.getValue());
        ourProcessor->endParameterChangeGesture(ZirkOscjuceAudioProcessor::ZirkOSC_Gain_Param + (selectedSource*7));
    }
    else if (slider == &_AzimuthSlider) {
        percentValue = HRToPercent((float) _AzimuthSlider.getValue(), ZirkOSC_Azim_Min, ZirkOSC_Azim_Max);
        ourProcessor->beginParameterChangeGesture(ZirkOscjuceAudioProcessor::ZirkOSC_Azim_Param+ (selectedSource*7));
        ourProcessor->setParameterNotifyingHost (ZirkOscjuceAudioProcessor::ZirkOSC_Azim_Param + (selectedSource*7),
                                                 percentValue);
        //_SourcePoint.setX(_AzimuthSlider.getValue());
        ourProcessor->endParameterChangeGesture(ZirkOscjuceAudioProcessor::ZirkOSC_Azim_Param + (selectedSource*7));
    }
    else if (slider == &_ElevationSlider) {
        percentValue = HRToPercent((float) _ElevationSlider.getValue(), ZirkOSC_Elev_Min, ZirkOSC_Elev_Max);
        ourProcessor->beginParameterChangeGesture(ZirkOscjuceAudioProcessor::ZirkOSC_Elev_Param + (selectedSource*7));
        ourProcessor->setParameterNotifyingHost (ZirkOscjuceAudioProcessor::ZirkOSC_Elev_Param + (selectedSource*7),
                                                 percentValue);
        //_SourcePoint.setY(_ElevationSlider.getValue());
        ourProcessor->endParameterChangeGesture(ZirkOscjuceAudioProcessor::ZirkOSC_Elev_Param + (selectedSource*7));
    }
    else if (slider == &_ElevationSpanSlider) {
        percentValue = HRToPercent((float) _ElevationSpanSlider.getValue(), ZirkOSC_ElevSpan_Min, ZirkOSC_ElevSpan_Max);
        if(_LinkSpan){
            for(int i=0 ; i<ourProcessor->nbrSources; i++){
                ourProcessor->beginParameterChangeGesture(ZirkOscjuceAudioProcessor::ZirkOSC_ElevSpan_Param + (i*7));
                ourProcessor->setParameterNotifyingHost (ZirkOscjuceAudioProcessor::ZirkOSC_ElevSpan_Param + (i*7),
                                                         percentValue);
                ourProcessor->endParameterChangeGesture(ZirkOscjuceAudioProcessor::ZirkOSC_ElevSpan_Param +(i*7));
            }
        }
        else{
            ourProcessor->beginParameterChangeGesture(ZirkOscjuceAudioProcessor::ZirkOSC_ElevSpan_Param + (selectedSource*7));
            ourProcessor->setParameterNotifyingHost (ZirkOscjuceAudioProcessor::ZirkOSC_ElevSpan_Param + (selectedSource*7),
                                                 percentValue);
            ourProcessor->endParameterChangeGesture(ZirkOscjuceAudioProcessor::ZirkOSC_ElevSpan_Param +(selectedSource*7));
        }

    }else if (slider == &_AzimuthSpanSlider) {
        percentValue = HRToPercent((float) _AzimuthSpanSlider.getValue(), ZirkOSC_AzimSpan_Min, ZirkOSC_AzimSpan_Max);
        if(_LinkSpan){
            for(int i=0 ; i<ourProcessor->nbrSources; i++){
                ourProcessor->beginParameterChangeGesture(ZirkOscjuceAudioProcessor::ZirkOSC_AzimSpan_Param + (i*7));
                ourProcessor->setParameterNotifyingHost (ZirkOscjuceAudioProcessor::ZirkOSC_AzimSpan_Param + (i*7),
                                                         percentValue);
                ourProcessor->endParameterChangeGesture(ZirkOscjuceAudioProcessor::ZirkOSC_AzimSpan_Param +(i*7));
            }
        }
        else{
            ourProcessor->beginParameterChangeGesture(ZirkOscjuceAudioProcessor::ZirkOSC_AzimSpan_Param + (selectedSource*7));
            ourProcessor->setParameterNotifyingHost (ZirkOscjuceAudioProcessor::ZirkOSC_AzimSpan_Param + (selectedSource*7),
                                                 percentValue);
            ourProcessor->endParameterChangeGesture(ZirkOscjuceAudioProcessor::ZirkOSC_AzimSpan_Param + (selectedSource*7));
        }
    }else;
    //repaint();

}

void ZirkOscjuceAudioProcessorEditor::mouseDown (const MouseEvent &event){
    int source=-1;

    if (event.x>10 && event.x <20+ZirkOSC_DomeRadius*2 && event.y>20 && event.y< 40+ZirkOSC_DomeRadius*2) {
        source=getSourceFromPosition(Point<float>(event.x-ZirkOSC_Center_X, event.y-ZirkOSC_Center_Y));

    }
    _DraggableSource = (source!=-1);
    if(_DraggableSource){

        getProcessor()->selectedSource = source;
        int selectedConstrain = getProcessor()->getSelectedConstrain();
        _ChannelNumberTextEditor.setText(String(getProcessor()->tabSource[source].getChannel()));
        if (selectedConstrain == Independant) {
            //_SourcePoint.setX(getProcessor()->tabSource[source].getAzimuth());
            //_SourcePoint.setY(getProcessor()->tabSource[source].getElevation());
            getProcessor()->beginParameterChangeGesture(ZirkOscjuceAudioProcessor::ZirkOSC_Azim_Param + source*7);
            getProcessor()->beginParameterChangeGesture(ZirkOscjuceAudioProcessor::ZirkOSC_Elev_Param + source*7);
        }
        else if (selectedConstrain == DeltaLocked || selectedConstrain == Circular || selectedConstrain == FixedRadius || selectedConstrain == FixedAngles || selectedConstrain == FullyFixed){
            for(int i = 0;i<getProcessor()->nbrSources; i++){
                getProcessor()->beginParameterChangeGesture(ZirkOscjuceAudioProcessor::ZirkOSC_Azim_Param + i*7);
                getProcessor()->beginParameterChangeGesture(ZirkOscjuceAudioProcessor::ZirkOSC_Elev_Param + i*7);
                getProcessor()->tabSource[i].setAzimReverse(false);
            }
        }
        //repaint();
    }
    _GainSlider.grabKeyboardFocus();
}

int ZirkOscjuceAudioProcessorEditor::getSourceFromPosition(Point<float> p ){
    for (int i=0; i<getProcessor()->nbrSources ; i++){
        if (getProcessor()->tabSource[i].contains(p)){
            return i;
        }
    }
    return -1;
}

void ZirkOscjuceAudioProcessorEditor::orderSourcesByAngle (int selected, SoundSource tab[]){
    int nbrSources = getProcessor()->nbrSources;
    int* order = getOrderSources(selected, tab);
    int count = 0;
    for(int i= 1; i != nbrSources ; i++){
        getProcessor()->setParameterNotifyingHost (ZirkOscjuceAudioProcessor::ZirkOSC_Azim_Param + (order[i]*7), tab[order[0]].getAzimuth()+ (float)(++count)/(float) nbrSources);
    }
}

int* ZirkOscjuceAudioProcessorEditor::getOrderSources(int selected, SoundSource tab []){
    int nbrSources = getProcessor()->nbrSources;
    int * order = new int [nbrSources];
    int firstItem = selected;
    order[0] = selected;
    int count  = 1;
    do{
        int current = (selected + 1)%nbrSources;

        int bestItem = current;
        float bestDelta = tab[current].getAzimuth() - tab[selected].getAzimuth();
        if (bestDelta<0){
            bestDelta+=1;
        }

        while (current != selected) {
            float currentAzimuth;
            if (tab[current].getAzimuth() - tab[selected].getAzimuth()>0 ){
                currentAzimuth = tab[current].getAzimuth();
            }
            else{
                currentAzimuth = tab[current].getAzimuth()+1;
            }
            // gainLabel.setText(String(count++),false);
            if (currentAzimuth - tab[selected].getAzimuth() < bestDelta) {
                bestItem = current;
                bestDelta = currentAzimuth - tab[selected].getAzimuth();

            }
            current = (current +1) % nbrSources;
        }

        order[count++]=bestItem;
        selected = bestItem;
    }
    while (selected != firstItem);
    return order;
}

void ZirkOscjuceAudioProcessorEditor::mouseDrag (const MouseEvent &event){
    if(_DraggableSource){
        ZirkOscjuceAudioProcessor* ourProcessor = getProcessor();
        int selectedSource = ourProcessor->selectedSource;
        Point<float> pointRelativeCenter = Point<float>(event.x-ZirkOSC_Center_X, event.y-ZirkOSC_Center_Y);
        int selectedConstrain = ourProcessor->getSelectedConstrain();
        if (selectedConstrain == Independant) {
            ourProcessor->tabSource[selectedSource].setPositionXY(pointRelativeCenter);
            float HRAzimuth = PercentToHR(ourProcessor->tabSource[selectedSource].getAzimuth(), ZirkOSC_Azim_Min, ZirkOSC_Azim_Max);
            //_SourcePoint.setX(HRAzimuth);

            ourProcessor->setParameterNotifyingHost (ZirkOscjuceAudioProcessor::ZirkOSC_Azim_Param + selectedSource*7,
                                                     ourProcessor->tabSource[selectedSource].getAzimuth());
            float HRElevation = PercentToHR(ourProcessor->tabSource[selectedSource].getElevation(), ZirkOSC_Elev_Min, ZirkOSC_Elev_Min);
            //_SourcePoint.setY(HRElevation);
            ourProcessor->setParameterNotifyingHost (ZirkOscjuceAudioProcessor::ZirkOSC_Elev_Param + selectedSource*7,
                                                     ourProcessor->tabSource[selectedSource].getElevation());
            ourProcessor->sendOSCValues();

            //repaint();
        }
        else if (selectedConstrain == FixedAngles){
             moveFixedAngles(pointRelativeCenter);
        }
        else if (selectedConstrain == FixedRadius){
            moveCircularWithFixedRadius(pointRelativeCenter);
        }
        else if (selectedConstrain == FullyFixed){
            moveFullyFixed(pointRelativeCenter);
        }
        else if (selectedConstrain == DeltaLocked){
            Point<float> DeltaMove = pointRelativeCenter - ourProcessor->tabSource[selectedSource].getPositionXY();
            moveSourcesWithDelta(DeltaMove);
        }
        else if (selectedConstrain == Circular){
            moveCircular(pointRelativeCenter);
        }
        repaint();
    }
    getProcessor()->sendOSCValues();
    _GainSlider.grabKeyboardFocus();
}

void ZirkOscjuceAudioProcessorEditor::moveFixedAngles(Point<float> p){
    int selectedSource = getProcessor()->selectedSource;
    if (!_FixedAngle){
        orderSourcesByAngle(selectedSource,getProcessor()->tabSource);
        _FixedAngle=true;
    }
    moveCircular(p);
}

void ZirkOscjuceAudioProcessorEditor::moveFullyFixed(Point<float> p){
    if (!_FixedAngle){
        orderSourcesByAngle(getProcessor()->selectedSource,getProcessor()->tabSource);
        _FixedAngle=true;
    }
    moveCircularWithFixedRadius(p);
}

void ZirkOscjuceAudioProcessorEditor::moveCircularWithFixedRadius(Point<float> pointRelativeCenter){
    ZirkOscjuceAudioProcessor* ourProcessor = getProcessor();
    int selectedSource = ourProcessor->selectedSource;
    SoundSource DomeNewPoint = SoundSource();
    DomeNewPoint.setPositionXY(pointRelativeCenter);
    float HRElevation = PercentToHR(ourProcessor->tabSource[selectedSource].getElevation(), ZirkOSC_Elev_Min, ZirkOSC_Elev_Max);
    float HRAzimuth = 180+PercentToHR(ourProcessor->tabSource[selectedSource].getAzimuth(), ZirkOSC_Azim_Min, ZirkOSC_Azim_Max);

    float HRNewElevation = PercentToHR(DomeNewPoint.getElevation(), ZirkOSC_Elev_Min, ZirkOSC_Elev_Max);
    float HRNewAzimuth = PercentToHR(DomeNewPoint.getAzimuth(), ZirkOSC_Azim_Min, ZirkOSC_Azim_Max);

    Point<float> deltaCircularMove = Point<float>(HRNewAzimuth-HRAzimuth,HRNewElevation-HRElevation);
    //azimuthLabel.setText(String(deltaCircularMove.getY()),false);

    ourProcessor->tabSource[selectedSource].setElevation(ourProcessor->tabSource[selectedSource].getElevation()+HRToPercent(deltaCircularMove.getY(), ZirkOSC_Elev_Min, ZirkOSC_Elev_Max));
    for (int i = 0; i<ourProcessor->nbrSources; i++) {
        ourProcessor->setParameterNotifyingHost (ZirkOscjuceAudioProcessor::ZirkOSC_Azim_Param + i*7,
                                                 ourProcessor->tabSource[i].getAzimuth()+HRToPercent(deltaCircularMove.getX(), ZirkOSC_Azim_Min, ZirkOSC_Azim_Max));
        ourProcessor->setParameterNotifyingHost (ZirkOscjuceAudioProcessor::ZirkOSC_Elev_Param + i*7,
                                                 ourProcessor->tabSource[selectedSource].getElevation());
    }
}

void ZirkOscjuceAudioProcessorEditor::moveCircular(Point<float> pointRelativeCenter){
    SoundSource DomeNewPoint = SoundSource();
    ZirkOscjuceAudioProcessor* ourProcessor = getProcessor();
    int selectedSource = ourProcessor->selectedSource;
    DomeNewPoint.setPositionXY(pointRelativeCenter);
    float HRElevation = PercentToHR(ourProcessor->tabSource[selectedSource].getElevation(), ZirkOSC_Elev_Min, ZirkOSC_Elev_Max);
    float HRAzimuth = 180+PercentToHR(ourProcessor->tabSource[selectedSource].getAzimuth(), ZirkOSC_Azim_Min, ZirkOSC_Azim_Max);

    float HRNewElevation = PercentToHR(DomeNewPoint.getElevation(), ZirkOSC_Elev_Min, ZirkOSC_Elev_Max);
    float HRNewAzimuth = PercentToHR(DomeNewPoint.getAzimuth(), ZirkOSC_Azim_Min, ZirkOSC_Azim_Max);

    Point<float> deltaCircularMove = Point<float>(HRNewAzimuth-HRAzimuth,HRNewElevation-HRElevation);
    //azimuthLabel.setText(String(deltaCircularMove.getY()),false);
    for (int i = 0; i<ourProcessor->nbrSources; i++) {
        ourProcessor->setParameterNotifyingHost (ZirkOscjuceAudioProcessor::ZirkOSC_Azim_Param + i*7,
                                                 ourProcessor->tabSource[i].getAzimuth()+HRToPercent(deltaCircularMove.getX(), ZirkOSC_Azim_Min, ZirkOSC_Azim_Max));
        if(!ourProcessor->tabSource[i].isAzimReverse()){
            ourProcessor->setParameterNotifyingHost (ZirkOscjuceAudioProcessor::ZirkOSC_Elev_Param + i*7,
                                                     ourProcessor->tabSource[i].getElevationRawValue()+HRToPercent(deltaCircularMove.getY(), ZirkOSC_Elev_Min, ZirkOSC_Elev_Max));
        }
        else{
            ourProcessor->setParameterNotifyingHost (ZirkOscjuceAudioProcessor::ZirkOSC_Elev_Param + i*7,
                                                     ourProcessor->tabSource[i].getElevationRawValue()-HRToPercent(deltaCircularMove.getY(), ZirkOSC_Elev_Min, ZirkOSC_Elev_Max));
        }
    }
}

void ZirkOscjuceAudioProcessorEditor::moveSourcesWithDelta(Point<float> DeltaMove){
    ZirkOscjuceAudioProcessor* ourProcessor = getProcessor();
    int nbrSources = ourProcessor->nbrSources;
    Point<float> currentPosition;
    for(int i=0;i<nbrSources;i++){
        currentPosition = ourProcessor->tabSource[i].getPositionXY();
        ourProcessor->tabSource[i].setPositionXY(currentPosition + DeltaMove);
        ourProcessor->setParameterNotifyingHost (ZirkOscjuceAudioProcessor::ZirkOSC_Azim_Param + i*7,
                                                 ourProcessor->tabSource[i].getAzimuth());
        ourProcessor->setParameterNotifyingHost (ZirkOscjuceAudioProcessor::ZirkOSC_Elev_Param + i*7,
                                                 ourProcessor->tabSource[i].getElevationRawValue());
        //azimuthLabel.setText(String(ourProcessor->tabSource[i].getElevation()), false);
        ourProcessor->sendOSCValues();
    }
    //repaint();
}

void ZirkOscjuceAudioProcessorEditor::mouseUp (const MouseEvent &event){
    if(_DraggableSource){
        int selectedConstrain = getProcessor()->getSelectedConstrain();
        if(selectedConstrain == Independant){
            int selectedSource = getProcessor()->selectedSource;
            getProcessor()->endParameterChangeGesture(ZirkOscjuceAudioProcessor::ZirkOSC_Azim_Param+ selectedSource*7);
            getProcessor()->endParameterChangeGesture(ZirkOscjuceAudioProcessor::ZirkOSC_Elev_Param + selectedSource*7);
        }
        else if (selectedConstrain == DeltaLocked || selectedConstrain == Circular || selectedConstrain == FixedRadius || selectedConstrain == FixedAngles || selectedConstrain == FullyFixed){
            for(int i = 0;i<getProcessor()->nbrSources; i++){
                getProcessor()->endParameterChangeGesture(ZirkOscjuceAudioProcessor::ZirkOSC_Azim_Param + i*7);
                getProcessor()->endParameterChangeGesture(ZirkOscjuceAudioProcessor::ZirkOSC_Elev_Param + i*7);
            }
            _FixedAngle=false;
        }
        _DraggableSource=false;
    }
    _GainSlider.grabKeyboardFocus();
}

void ZirkOscjuceAudioProcessorEditor::textEditorReturnKeyPressed (TextEditor &editor){
    ZirkOscjuceAudioProcessor* ourProcessor = getProcessor();

    if(&_OscPortTextEditor == &editor )
    {
        int newPort = _OscPortTextEditor.getText().getIntValue();
        ourProcessor->changeOSCPort(newPort);
        _OscPortTextEditor.setText(String(ourProcessor->moscPort));
    }
    if(&_NbrSourceTextEditor == &editor )
    {
        int newNbrSources = _NbrSourceTextEditor.getText().getIntValue();
        if(newNbrSources >0 && newNbrSources < 9){
            ourProcessor->nbrSources = newNbrSources;
        }
        else{
            _NbrSourceTextEditor.setText(String(ourProcessor->nbrSources));
        }
    }
    if(&_ChannelNumberTextEditor == &editor ){
        int newChannel = _ChannelNumberTextEditor.getText().getIntValue();
        ourProcessor->tabSource[ourProcessor->selectedSource].setChannel(newChannel);
        ourProcessor->sendOSCValues();
    }
    if (&_OscPortOutgoingIPadTextEditor ==&editor) {
        int newPort = _OscPortOutgoingIPadTextEditor.getText().getIntValue();
        ourProcessor->changeOSCSendIPad(newPort, ourProcessor->mOscAddressIpad);
    }
    if (&_OscAdressIPadTextEditor ==&editor) {
        String oscAddress = _OscAdressIPadTextEditor.getText();
        ourProcessor->changeOSCSendIPad(ourProcessor->mOscPortIpadOutgoing.getIntValue(), oscAddress);
    }
    if (&_OscPortIncomingIPadTextEditor ==&editor) {
        int newPort = _OscPortIncomingIPadTextEditor.getText().getIntValue();
        ourProcessor->changeOSCPortReceive(newPort);
    }
    ourProcessor->sendOSCConfig();
    ourProcessor->sendOSCValues();
    ourProcessor->sendOSCMovementType();
    _GainSlider.grabKeyboardFocus();
}

void ZirkOscjuceAudioProcessorEditor::textEditorFocusLost (TextEditor &editor){
    //textEditorReturnKeyPressed(editor);
}

void ZirkOscjuceAudioProcessorEditor::comboBoxChanged (ComboBox* comboBoxThatHasChanged){
    getProcessor()->setSelectedContrain(comboBoxThatHasChanged->getSelectedId());
    getProcessor()->sendOSCMovementType();
    _GainSlider.grabKeyboardFocus();

}

bool ZirkOscjuceAudioProcessorEditor::isFixedAngle(){
    return _FixedAngle;
}

void ZirkOscjuceAudioProcessorEditor::setFixedAngle(bool fixedAngle){
    _FixedAngle = fixedAngle;
}

void ZirkOscjuceAudioProcessorEditor::setDraggableSource(bool drag){
    _DraggableSource = drag;
}

bool ZirkOscjuceAudioProcessorEditor::isDraggableSource(){
    return _DraggableSource;
}
