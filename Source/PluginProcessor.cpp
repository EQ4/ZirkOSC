/*
 ==============================================================================
Copyright 2013 Ludovic LAFFINEUR ludovic.laffineur@gmail.com

 ==============================================================================
 */

//       lo_send(mOsc, "/pan/az", "i", ch);

#include "PluginProcessor.h"
#include "PluginEditor.h"
#include <string.h>
#include <sstream>
#include <regex.h>

// using stringstream constructors.
#include <iostream>

using namespace std;
//==============================================================================
void error(int num, const char *m, const char *path);
int receivePositionUpdate(const char *path, const char *types, lo_arg **argv, int argc,void *data, void *user_data);
int receiveBeginTouch(const char *path, const char *types, lo_arg **argv, int argc,void *data, void *user_data);
int receiveEndTouch(const char *path, const char *types, lo_arg **argv, int argc,void *data, void *user_data);
int receiveAzimuthSpanUpdate(const char *path, const char *types, lo_arg **argv, int argc, void *data, void *user_data);
int receiveAzimuthSpanBegin(const char *path, const char *types, lo_arg **argv, int argc, void *data, void *user_data);
int receiveAzimuthSpanEnd(const char *path, const char *types, lo_arg **argv, int argc, void *data, void *user_data);
int receiveElevationSpanUpdate(const char *path, const char *types, lo_arg **argv, int argc, void *data, void *user_data);
int receiveElevationSpanBegin(const char *path, const char *types, lo_arg **argv, int argc, void *data, void *user_data);
int receiveElevationSpanEnd(const char *path, const char *types, lo_arg **argv, int argc, void *data, void *user_data);

ZirkOscjuceAudioProcessor::ZirkOscjuceAudioProcessor()
{

    _NbrSources      = 1;
    _SelectedSource  = 0;

    for(int i=0; i<8; i++)
        _TabSource[i]=SoundSource(0.0+((float)i/10.0),0.0);
    _OscZirkonium   = lo_address_new("127.0.0.1", "10001");
    _OscIpad        = lo_address_new("10.0.1.3", "10114");
    _St             = lo_server_thread_new("10116", error);
    if(_St){
        lo_server_thread_add_method(_St, "/pan/az", "ifffff", receivePositionUpdate, this);
        lo_server_thread_add_method(_St, "/begintouch", "i", receiveBeginTouch, this);
        lo_server_thread_add_method(_St, "/endtouch", "i", receiveEndTouch, this);
        lo_server_thread_add_method(_St, "/moveAzimSpan", "if", receiveAzimuthSpanUpdate, this);
        lo_server_thread_add_method(_St, "/beginAzimSpanMove", "i", receiveAzimuthSpanBegin, this);
        lo_server_thread_add_method(_St, "/endAzimSpanMove", "i", receiveAzimuthSpanEnd, this);
        lo_server_thread_add_method(_St, "/moveElevSpan", "if", receiveElevationSpanUpdate, this);
        lo_server_thread_add_method(_St, "/beginElevSpanMove", "i", receiveElevationSpanBegin, this);
        lo_server_thread_add_method(_St, "/endElevSpanMove", "i", receiveElevationSpanEnd, this);
        
        lo_server_thread_start(_St);
    }
    
    //default values for ui dimensions
    _LastUiWidth = ZirkOSC_Window_Width;
    _LastUiHeight = ZirkOSC_Window_Height;
    
    startTimer (50);
}

void error(int num, const char *m, const char *path){
    printf("liblo server error %d in path %s: %s\n", num, path, m);
    fflush(stdout);
}

void ZirkOscjuceAudioProcessor::timerCallback(){
    const MessageManagerLock mmLock;
    sendOSCValues();
}


ZirkOscjuceAudioProcessor::~ZirkOscjuceAudioProcessor()
{
    if (_St){
        lo_server st2 = _St;
        lo_server_thread_stop(st2);
        lo_server_thread_free(st2);
        _St = NULL;
    }
    lo_address osc = _OscZirkonium;
    lo_address osc2 = _OscIpad;
    if (osc){
        lo_address_free(osc);
    }
    if (osc2){
        lo_address_free(osc2);
    }
    _OscZirkonium = NULL;
    _OscIpad = NULL;
}

//==============================================================================
const String ZirkOscjuceAudioProcessor::getName() const
{
    return JucePlugin_Name;
}

int ZirkOscjuceAudioProcessor::getNumParameters()
{
    return totalNumParams;
}

float ZirkOscjuceAudioProcessor::getParameter (int index)
{
    // This method will be called by the host, probably on the audio thread, so
    // it's absolutely time-critical. Don't use critical sections or anything
    // UI-related, or anything at all that may block in any way!
    if (ZirkOSC_MovementConstraint_ParamId == index){
        return _SelectedMovementConstraint;
    }
    
    for(int i = 0; i<8;i++){
        if      (ZirkOSC_Azim_ParamId + (i*7) == index)       return _TabSource[i].getAzimuth();
        else if (ZirkOSC_AzimSpan_ParamId + (i*7) == index)   return _TabSource[i].getAzimuthSpan();
        //else if (ZirkOSC_Channel_ParamId + (i*7) == index)    return tabSource[i].getChannel();
        else if (ZirkOSC_Elev_ParamId + (i*7) == index)       return _TabSource[i].getElevation();
        else if (ZirkOSC_ElevSpan_ParamId + (i*7) == index)   return _TabSource[i].getElevationSpan();
        else if (ZirkOSC_Gain_ParamId + (i*7) == index)       return _TabSource[i].getGain();
        else;
    }
    cout << endl << "wrong parameter id: " << index << "in ZirkOscjuceAudioProcessor::getParameter" << endl;
}


void ZirkOscjuceAudioProcessor::setParameter (int index, float newValue)
{
    // This method will be called by the host, probably on the audio thread, so
    // it's absolutely time-critical. Don't use critical sections or anything
    // UI-related, or anything at all that may block in any way!
    
    if (ZirkOSC_MovementConstraint_ParamId == index && newValue<7 && newValue>0){
        _SelectedMovementConstraint = newValue;
        return;
    }
    
    for(int i = 0; i<8;i++){
        if      (ZirkOSC_Azim_ParamId + (i*7) == index)       {_TabSource[i].setAzimuth(newValue); break;}
        else if (ZirkOSC_AzimSpan_ParamId + (i*7) == index)   {_TabSource[i].setAzimuthSpan(newValue); break;}
        // else if (ZirkOSC_Channel_ParamId + (i*7) == index)    {tabSource[i].setChannel(newValue); break;}
        else if (ZirkOSC_Elev_ParamId + (i*7) == index)       {_TabSource[i].setElevation(newValue); break;}
        else if (ZirkOSC_ElevSpan_ParamId + (i*7) == index)   {_TabSource[i].setElevationSpan(newValue); break;}
        else if (ZirkOSC_Gain_ParamId + (i*7) == index)       {_TabSource[i].setGain(newValue); break;}
        else;
    }
     cout << endl << "wrong parameter id: " << index << " in ZirkOscjuceAudioProcessor::setParameter" << endl;
}

const String ZirkOscjuceAudioProcessor::getParameterName (int index)
{
    for(int i = 0; i<8;i++){
        if      (ZirkOSC_Azim_ParamId + (i*7) == index)       return ZirkOSC_Azim_name[i];
        else if (ZirkOSC_AzimSpan_ParamId + (i*7) == index)   return ZirkOSC_AzimSpan_name[i];
        //else if (ZirkOSC_Channel_ParamId + (i*7) == index)    return ZirkOSC_Channel_name[i];
        else if (ZirkOSC_Elev_ParamId + (i*7) == index)       return ZirkOSC_Elev_name[i];
        else if (ZirkOSC_ElevSpan_ParamId + (i*7) == index)   return ZirkOSC_ElevSpan_name[i];
        else if (ZirkOSC_Gain_ParamId + (i*7) == index)       return ZirkOSC_Gain_name[i];
        else;
    }
    return String::empty;
}



void ZirkOscjuceAudioProcessor::sendOSCValues(){
    for(int i=0;i<_NbrSources;i++){
        float azim_osc = PercentToHR(_TabSource[i].getAzimuth(), ZirkOSC_Azim_Min, ZirkOSC_Azim_Max) /180.;
        float elev_osc = PercentToHR(_TabSource[i].getElevation(), ZirkOSC_Elev_Min, ZirkOSC_Elev_Max)/180.;
        float azimspan_osc = PercentToHR(_TabSource[i].getAzimuthSpan(), ZirkOSC_AzimSpan_Min,ZirkOSC_AzimSpan_Max)/180.;
        float elevspan_osc = PercentToHR(_TabSource[i].getElevationSpan(), ZirkOSC_ElevSpan_Min, ZirkOSC_Elev_Max)/180.;
        int channel_osc = _TabSource[i].getChannel()-1;
        float gain_osc = _TabSource[i].getGain();
        lo_send(_OscZirkonium, "/pan/az", "ifffff", channel_osc, azim_osc, elev_osc, azimspan_osc, elevspan_osc, gain_osc);
        azim_osc = azim_osc * M_PI;
        elev_osc = elev_osc * M_PI;
        azimspan_osc = azimspan_osc * M_PI;
        elevspan_osc = elevspan_osc * M_PI;
        lo_send(_OscIpad, "/pan/az", "ifffff", channel_osc+1, azim_osc, elev_osc, azimspan_osc, elevspan_osc, gain_osc);
    }
}

void ZirkOscjuceAudioProcessor::sendOSCConfig(){
    lo_send(_OscIpad, "/maxsource", "iiiiiiiii", _NbrSources, _TabSource[0].getChannel(), _TabSource[1].getChannel(), _TabSource[2].getChannel(), _TabSource[3].getChannel(), _TabSource[4].getChannel(), _TabSource[5].getChannel(), _TabSource[6].getChannel(), _TabSource[7].getChannel());
    
}

void ZirkOscjuceAudioProcessor::changeOSCPort(int newPort){
    lo_address osc = _OscZirkonium;
    _OscPortZirkonium = newPort;
	_OscZirkonium = NULL;
    lo_address_free(osc);
	char port[32];
	snprintf(port, sizeof(port), "%d", newPort);
	_OscZirkonium = lo_address_new("127.0.0.1", port);

}

void ZirkOscjuceAudioProcessor::changeOSCSendIPad(int newPort, String newAddress){
    lo_address osc = _OscIpad;
    _OscPortIpadOutgoing = String(newPort);
    _OscIpad = NULL;
    lo_address_free(osc);
	char port[32];
	snprintf(port, sizeof(port), "%d", newPort);
    regex_t regex;
    int reti;
    reti = regcomp(&regex, "\b(?:(?:25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?).){3}(?:25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)\b", 0);
    reti = regexec(&regex, newAddress.toUTF8(), 0, NULL, 0);
    //if( !reti ){
         _OscAddressIpad = newAddress;
    //}
	_OscIpad = lo_address_new(_OscAddressIpad.toUTF8(), port);
}

void ZirkOscjuceAudioProcessor::changeOSCPortReceive(int newPort){
    if(_St){
        lo_server st2 = _St;
        lo_server_thread_stop(st2);
        lo_server_thread_free(st2);
    }
    _OscPortIpadIncoming = String(newPort);
    char port[32];
	snprintf(port, sizeof(port), "%d", newPort);
    _St = lo_server_thread_new(port, error);
    if (_St){
        lo_server_thread_add_method(_St, "/pan/az", "ifffff", receivePositionUpdate, this);
        lo_server_thread_add_method(_St, "/begintouch", "i", receiveBeginTouch, this);
        lo_server_thread_add_method(_St, "/endtouch", "i", receiveEndTouch, this);
        lo_server_thread_add_method(_St, "/moveAzimSpan", "if", receiveAzimuthSpanUpdate, this);
        lo_server_thread_add_method(_St, "/beginAzimSpanMove", "i", receiveAzimuthSpanBegin, this);
        lo_server_thread_add_method(_St, "/endAzimSpanMove", "i", receiveAzimuthSpanEnd, this);
        lo_server_thread_add_method(_St, "/moveElevSpan", "if", receiveElevationSpanUpdate, this);
        lo_server_thread_add_method(_St, "/beginElevSpanMove", "i", receiveElevationSpanBegin, this);
        lo_server_thread_add_method(_St, "/endElevSpanMove", "i", receiveElevationSpanEnd, this);
        lo_server_thread_start(_St);
    }
    
}
const String ZirkOscjuceAudioProcessor::getParameterText (int index)
{
    return String (getParameter (index), 2);
}

const String ZirkOscjuceAudioProcessor::getInputChannelName (int channelIndex) const
{
    return String (channelIndex + 1);
}

const String ZirkOscjuceAudioProcessor::getOutputChannelName (int channelIndex) const
{
    return String (channelIndex + 1);
}

bool ZirkOscjuceAudioProcessor::isInputChannelStereoPair (int index) const
{
    return true;
}

bool ZirkOscjuceAudioProcessor::isOutputChannelStereoPair (int index) const
{
    return true;
}

bool ZirkOscjuceAudioProcessor::acceptsMidi() const
{
#if JucePlugin_WantsMidiInput
    return true;
#else
    return false;
#endif
}

bool ZirkOscjuceAudioProcessor::producesMidi() const
{
#if JucePlugin_ProducesMidiOutput
    return true;
#else
    return false;
#endif
}

bool ZirkOscjuceAudioProcessor::silenceInProducesSilenceOut() const
{
    return false;
}

int ZirkOscjuceAudioProcessor::getNumPrograms()
{
    return 0;
}

int ZirkOscjuceAudioProcessor::getCurrentProgram()
{
    return 0;
}

void ZirkOscjuceAudioProcessor::setCurrentProgram (int index)
{
}

const String ZirkOscjuceAudioProcessor::getProgramName (int index)
{
    return String::empty;
}

void ZirkOscjuceAudioProcessor::changeProgramName (int index, const String& newName)
{
}

//==============================================================================
void ZirkOscjuceAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    // Use this method as the place to do any pre-playback
    // initialisation that you need..
}

void ZirkOscjuceAudioProcessor::releaseResources()
{
    // When playback stops, you can use this as an opportunity to free up any
    // spare memory, etc.
}

void ZirkOscjuceAudioProcessor::processBlock (AudioSampleBuffer& buffer, MidiBuffer& midiMessages)
{
    

}

//==============================================================================
bool ZirkOscjuceAudioProcessor::hasEditor() const
{
    return true; // (change this to false if you choose to not supply an editor)
}

AudioProcessorEditor* ZirkOscjuceAudioProcessor::createEditor()
{
    _Editor = new ZirkOscjuceAudioProcessorEditor (this);
    return _Editor;
}


void ZirkOscjuceAudioProcessor::setLastUiWidth(int lastUiWidth)
{
    _LastUiWidth = lastUiWidth;
}

int ZirkOscjuceAudioProcessor::getLastUiWidth()
{
    return _LastUiWidth;
}
void ZirkOscjuceAudioProcessor::setLastUiHeight(int lastUiHeight)
{
    _LastUiHeight = lastUiHeight;
}

int ZirkOscjuceAudioProcessor::getLastUiHeight()
{
    return _LastUiHeight;
}

//==============================================================================
void ZirkOscjuceAudioProcessor::getStateInformation (MemoryBlock& destData)
{
    // You should use this method to store your parameters in the memory block.
    // You could do that either as raw data, or use the XML or ValueTree classes
    // as intermediaries to make it easy to save and load complex data.
    XmlElement xml ("ZIRKOSCJUCESETTINGS");
    xml.setAttribute ("uiWidth", _LastUiWidth);
    xml.setAttribute ("uiHeight", _LastUiHeight);
    xml.setAttribute("PortOSC", _OscPortZirkonium);
    xml.setAttribute("IncPort", _OscPortIpadIncoming);
    xml.setAttribute("OutPort", _OscPortIpadOutgoing);
    xml.setAttribute("AddIpad", _OscAddressIpad);
    xml.setAttribute("NombreSources", _NbrSources);
    for(int i =0;i<8;i++){
        String channel = "Channel";
        String azimuth = "Azimuth";
        String elevation = "Elevation";
        String azimuthSpan = "AzimuthSpan";
        String elevationSpan = "ElevationSpan";
        String gain = "Gain";
        channel.append(String(i), 10);
        xml.setAttribute(channel, _TabSource[i].getChannel());
        azimuth.append(String(i), 10);
        xml.setAttribute(azimuth, _TabSource[i].getAzimuth());
        elevation.append(String(i), 10);
        xml.setAttribute(elevation, _TabSource[i].getElevationRawValue());
        azimuthSpan.append(String(i), 10);
        xml.setAttribute(azimuthSpan, _TabSource[i].getAzimuthSpan());
        elevationSpan.append(String(i), 10);
        xml.setAttribute(elevationSpan, _TabSource[i].getElevationSpan());
        gain.append(String(i), 10);
        xml.setAttribute(gain, _TabSource[i].getChannel());
        
    }
    copyXmlToBinary (xml, destData);
    
}



void ZirkOscjuceAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    // You should use this method to restore your parameters from this memory block,
    // whose contents will have been created by the getStateInformation() call.

    // This getXmlFromBinary() helper function retrieves our XML from the binary blob..
    ScopedPointer<XmlElement> xmlState (getXmlFromBinary (data, sizeInBytes));

    if (xmlState != nullptr)
    {
        // make sure that it's actually our type of XML object..
        if (xmlState->hasTagName ("ZIRKOSCJUCESETTINGS"))
        {
            // ok, now pull out our parameters..
            _LastUiWidth  = xmlState->getIntAttribute ("uiWidth", _LastUiWidth);
            _LastUiHeight = xmlState->getIntAttribute ("uiHeight", _LastUiHeight);
            _OscPortZirkonium = xmlState->getIntAttribute("PortOSC", 20000);
            if(_OscPortZirkonium<0 || _OscPortZirkonium>100000){
                _OscPortZirkonium = 20000;
            }
            _OscPortIpadIncoming = xmlState->getStringAttribute("IncPort", "10002");
            _OscPortIpadOutgoing = xmlState->getStringAttribute("OutPort", "10004");
            _OscAddressIpad = xmlState -> getStringAttribute("AddIpad", "10.0.1.3");
            _NbrSources = xmlState->getIntAttribute("NombreSources", 1);
            ;            for (int i=0;i<8;i++){
                String channel = "Channel";
                String azimuth = "Azimuth";
                String elevation = "Elevation";
                String azimuthSpan = "AzimuthSpan";
                String elevationSpan = "ElevationSpan";
                String gain = "Gain";
                channel.append(String(i), 10);
                azimuth.append(String(i), 10);
                elevation.append(String(i), 10);
                azimuthSpan.append(String(i), 10);
                elevationSpan.append(String(i), 10);
                gain.append(String(i), 10);
                _TabSource[i].setChannel(xmlState->getIntAttribute(channel , 0));
                _TabSource[i].setAzimuth((float) xmlState->getDoubleAttribute(azimuth,0));
                _TabSource[i].setElevation((float) xmlState->getDoubleAttribute(elevation,0));
                _TabSource[i].setAzimuthSpan((float) xmlState->getDoubleAttribute(azimuthSpan,0));
                _TabSource[i].setElevationSpan((float) xmlState->getDoubleAttribute(elevationSpan,0));
                _TabSource[i].setGain((float) xmlState->getDoubleAttribute(gain,1 ));
            }
            
            changeOSCPort(_OscPortZirkonium);
            changeOSCPortReceive(_OscPortIpadIncoming.getIntValue());
            changeOSCSendIPad(_OscPortIpadOutgoing.getIntValue(), _OscAddressIpad);
            sendOSCValues();
            _RefreshGui=true;
        }
    }
}

void ZirkOscjuceAudioProcessor::sendOSCMovementType(){ //should be void with no argument if movement is included in the processor!!!!!
    lo_send(_OscIpad, "/movementmode", "i", _SelectedMovementConstraint);
    
}


int receiveBeginTouch(const char *path, const char *types, lo_arg **argv, int argc,void *data, void *user_data){
    ZirkOscjuceAudioProcessor *processor = (ZirkOscjuceAudioProcessor*) user_data;
    ZirkOscjuceAudioProcessorEditor* editor = (ZirkOscjuceAudioProcessorEditor*) processor->getEditor();
    //printf("Receive BEGIN");
    int channel_osc = argv[0]->i;
    int i =0;
    for(i=0;i<processor->getNbrSources();i++){
        if (processor->getSources()[i].getChannel() == channel_osc) {
            break;
        }
    }
    if (i==processor->getNbrSources()){
        return 0;
    }
    
    if (processor->getSelectedMovementConstraint() == ZirkOscjuceAudioProcessorEditor::Independant){
        processor->beginParameterChangeGesture(ZirkOscjuceAudioProcessor::ZirkOSC_Azim_ParamId+ i*7);
        processor->beginParameterChangeGesture(ZirkOscjuceAudioProcessor::ZirkOSC_Elev_ParamId+ i*7);
    }
    else{
        for (int j = 0; j<processor->getNbrSources() ;j++){
            processor->beginParameterChangeGesture(ZirkOscjuceAudioProcessor::ZirkOSC_Azim_ParamId+ j*7);
            processor->beginParameterChangeGesture(ZirkOscjuceAudioProcessor::ZirkOSC_Elev_ParamId+ j*7);
            
        }
    }
    editor->setDraggableSource(true);
    
    return 0;
}

int receiveEndTouch(const char *path, const char *types, lo_arg **argv, int argc,void *data, void *user_data){
    ZirkOscjuceAudioProcessor *processor = (ZirkOscjuceAudioProcessor*) user_data;
    ZirkOscjuceAudioProcessorEditor* theEditor = (ZirkOscjuceAudioProcessorEditor* ) processor->getEditor();
    int i =0;
    printf("Receive END");
    int channel_osc = argv[0]->i;
    for(i=0;i<processor->getNbrSources();i++){
        if (processor->getSources()[i].getChannel() == channel_osc) {
            break;
        }
    }
    if (i==processor->getNbrSources()){
        return 0;
    }
    if (processor->getSelectedMovementConstraint() != ZirkOscjuceAudioProcessorEditor::Independant){
        for (int j = 0; j<processor->getNbrSources() ;j++){
            processor->endParameterChangeGesture(ZirkOscjuceAudioProcessor::ZirkOSC_Azim_ParamId+ j*7);
            processor->endParameterChangeGesture(ZirkOscjuceAudioProcessor::ZirkOSC_Elev_ParamId+ j*7);
            
        }
        theEditor->setFixedAngle(false);
    }
    else{
        if (processor->getSelectedMovementConstraint() == ZirkOscjuceAudioProcessorEditor::Independant){
            processor->endParameterChangeGesture(ZirkOscjuceAudioProcessor::ZirkOSC_Azim_ParamId+ i*7);
            processor->endParameterChangeGesture(ZirkOscjuceAudioProcessor::ZirkOSC_Elev_ParamId+ i*7);
            printf("OUT");
        }
    }
    theEditor->setDraggableSource(false);
    return 0;
}

int receiveAzimuthSpanUpdate(const char *path, const char *types, lo_arg **argv, int argc, void *data, void *user_data){
    ZirkOscjuceAudioProcessor *processor = (ZirkOscjuceAudioProcessor*) user_data;
    int channel_osc = argv[0]->i;
    int i;
    float value = argv[1]->f;
    for(i=0;i<processor->getNbrSources();i++){
        if (processor->getSources()[i].getChannel() == channel_osc) {
            break;
        }
    }
    if (i==processor->getNbrSources()){
        return 0;
    }
    if (processor->getSelectedMovementConstraint() != ZirkOscjuceAudioProcessorEditor::Independant){
        
        for (int j = 0; j<processor->getNbrSources() ;j++){
            processor->setParameterNotifyingHost (ZirkOscjuceAudioProcessor::ZirkOSC_AzimSpan_ParamId + j*7,
                                                  HRToPercent(value, ZirkOSC_AzimSpan_Min, ZirkOSC_AzimSpan_Max));
            
        }
    }
    else{
        if (processor->getSelectedMovementConstraint() == ZirkOscjuceAudioProcessorEditor::Independant){
            processor->setParameterNotifyingHost (ZirkOscjuceAudioProcessor::ZirkOSC_AzimSpan_ParamId + i*7,
                                                  HRToPercent(value, ZirkOSC_AzimSpan_Min, ZirkOSC_AzimSpan_Max));
            printf("I am right there : %d, %f", channel_osc,value);
        }
    }
    return 0;
}

int receiveAzimuthSpanBegin(const char *path, const char *types, lo_arg **argv, int argc, void *data, void *user_data){
    ZirkOscjuceAudioProcessor *processor = (ZirkOscjuceAudioProcessor*) user_data;
    int channel_osc = argv[0]->i;
    int i;
    for(i=0;i<processor->getNbrSources();i++){
        if (processor->getSources()[i].getChannel() == channel_osc) {
            break;
        }
    }
    if (i==processor->getNbrSources()){
        return 0;
    }
    if (processor->getSelectedMovementConstraint() != ZirkOscjuceAudioProcessorEditor::Independant){
        
        for (int j = 0; j<processor->getNbrSources() ;j++){
            processor->beginParameterChangeGesture(ZirkOscjuceAudioProcessor::ZirkOSC_AzimSpan_ParamId + j*7);
            
        }
    }
    else{
        if (processor->getSelectedMovementConstraint() == ZirkOscjuceAudioProcessorEditor::Independant){
            processor->beginParameterChangeGesture(ZirkOscjuceAudioProcessor::ZirkOSC_AzimSpan_ParamId + i*7);
        }
    }
    return 0;
}

int receiveAzimuthSpanEnd(const char *path, const char *types, lo_arg **argv, int argc, void *data, void *user_data){
    ZirkOscjuceAudioProcessor *processor = (ZirkOscjuceAudioProcessor*) user_data;
    int channel_osc = argv[0]->i;
    int i;
    for(i=0;i<processor->getNbrSources();i++){
        if (processor->getSources()[i].getChannel() == channel_osc) {
            break;
        }
    }
    if (i==processor->getNbrSources()){
        return 0;
    }
    if (processor->getSelectedMovementConstraint() != ZirkOscjuceAudioProcessorEditor::Independant){
        
        for (int j = 0; j<processor->getNbrSources() ;j++){
            processor->endParameterChangeGesture(ZirkOscjuceAudioProcessor::ZirkOSC_AzimSpan_ParamId + j*7);
            
        }
    }
    else{
        if (processor->getSelectedMovementConstraint() == ZirkOscjuceAudioProcessorEditor::Independant){
            processor->endParameterChangeGesture(ZirkOscjuceAudioProcessor::ZirkOSC_AzimSpan_ParamId + i*7);
        }
    }
    return 0;
}

int receiveElevationSpanUpdate(const char *path, const char *types, lo_arg **argv, int argc, void *data, void *user_data){
    ZirkOscjuceAudioProcessor *processor = (ZirkOscjuceAudioProcessor*) user_data;
    int channel_osc = argv[0]->i;
    int i;
    float value = argv[1]->f;
    for(i=0;i<processor->getNbrSources();i++){
        if (processor->getSources()[i].getChannel() == channel_osc) {
            break;
        }
    }
    if (i==processor->getNbrSources()){
        return 0;
    }
    if (processor->getSelectedMovementConstraint() != ZirkOscjuceAudioProcessorEditor::Independant){
        
        for (int j = 0; j<processor->getNbrSources() ;j++){
            processor->setParameterNotifyingHost (ZirkOscjuceAudioProcessor::ZirkOSC_ElevSpan_ParamId + j*7,
                                                  HRToPercent(value, ZirkOSC_ElevSpan_Min, ZirkOSC_ElevSpan_Max));
            
        }
    }
    else{
        if (processor->getSelectedMovementConstraint() == ZirkOscjuceAudioProcessorEditor::Independant){
            processor->setParameterNotifyingHost (ZirkOscjuceAudioProcessor::ZirkOSC_ElevSpan_ParamId + i*7,
                                                  HRToPercent(value, ZirkOSC_ElevSpan_Min, ZirkOSC_ElevSpan_Max));
        }
    }
    return 0;

}

int receiveElevationSpanBegin(const char *path, const char *types, lo_arg **argv, int argc, void *data, void *user_data){
    ZirkOscjuceAudioProcessor *processor = (ZirkOscjuceAudioProcessor*) user_data;
    int channel_osc = argv[0]->i;
    int i;
    for(i=0;i<processor->getNbrSources();i++){
        if (processor->getSources()[i].getChannel() == channel_osc) {
            break;
        }
    }
    if (i==processor->getNbrSources()){
        return 0;
    }
    if (processor->getSelectedMovementConstraint() != ZirkOscjuceAudioProcessorEditor::Independant){
        
        for (int j = 0; j<processor->getNbrSources() ;j++){
            processor->beginParameterChangeGesture (ZirkOscjuceAudioProcessor::ZirkOSC_ElevSpan_ParamId + j*7);
        }
    }
    else{
        if (processor->getSelectedMovementConstraint() == ZirkOscjuceAudioProcessorEditor::Independant){
            processor->beginParameterChangeGesture (ZirkOscjuceAudioProcessor::ZirkOSC_ElevSpan_ParamId + i*7);
        }
    }
    return 0;
    

}

int receiveElevationSpanEnd(const char *path, const char *types, lo_arg **argv, int argc, void *data, void *user_data){
    ZirkOscjuceAudioProcessor *processor = (ZirkOscjuceAudioProcessor*) user_data;
    int channel_osc = argv[0]->i;
    int i;
    for(i=0;i<processor->getNbrSources();i++){
        if (processor->getSources()[i].getChannel() == channel_osc) {
            break;
        }
    }
    if (i==processor->getNbrSources()){
        return 0;
    }
    if (processor->getSelectedMovementConstraint() != ZirkOscjuceAudioProcessorEditor::Independant){
        
        for (int j = 0; j<processor->getNbrSources() ;j++){
            processor->endParameterChangeGesture (ZirkOscjuceAudioProcessor::ZirkOSC_ElevSpan_ParamId + j*7);
        }
    }
    else{
        if (processor->getSelectedMovementConstraint() == ZirkOscjuceAudioProcessorEditor::Independant){
            processor->endParameterChangeGesture (ZirkOscjuceAudioProcessor::ZirkOSC_ElevSpan_ParamId + i*7);
        }
    }
    return 0;
}

int receivePositionUpdate(const char *path, const char *types, lo_arg **argv, int argc, void *data, void *user_data){
 
    
    //channel send ifffff", channel_osc, azim_osc, elev_osc, azimspan_osc, elevspan_osc, gain_osc); all angles in RADIAN
    ZirkOscjuceAudioProcessor *processor = (ZirkOscjuceAudioProcessor*) user_data;
    int channel_osc = argv[0]->i;
    int i =0;
    for(i=0;i<processor->getNbrSources();i++){
        if (processor->getSources()[i].getChannel() == channel_osc) {
            break;
        }
    }
    if (i==processor->getNbrSources()){
        return 0;
    }
    float azim_osc = argv[1]->f;
    float elev_osc = argv[2]->f;
    Point<float> pointRelativeCenter = Point<float>(processor->domeToScreen(Point<float>(azim_osc,elev_osc)));
    ZirkOscjuceAudioProcessorEditor* theEditor =(ZirkOscjuceAudioProcessorEditor*) (processor->getEditor());
    if(processor->getSelectedMovementConstraint() == ZirkOscjuceAudioProcessorEditor::Independant){
        processor->setParameterNotifyingHost (ZirkOscjuceAudioProcessor::ZirkOSC_Azim_ParamId + i*7,
                                              HRToPercent(azim_osc, -M_PI, M_PI));
        processor->setParameterNotifyingHost (ZirkOscjuceAudioProcessor::ZirkOSC_Elev_ParamId + i*7,
                                              HRToPercent(elev_osc, 0.0, M_PI/2.0));
    }
    else{
        processor->setSelectedSource(i);

        if(processor->getSelectedMovementConstraint() == ZirkOscjuceAudioProcessorEditor::Circular){
            theEditor->moveCircular(pointRelativeCenter);
            
        }
        else if(processor->getSelectedMovementConstraint()  == ZirkOscjuceAudioProcessorEditor::DeltaLocked){
            Point<float> DeltaMove = pointRelativeCenter - processor->getSources()[processor->getSelectedSource()].getPositionXY();
            theEditor->moveSourcesWithDelta(DeltaMove);
        }
        else if(processor->getSelectedMovementConstraint()  == ZirkOscjuceAudioProcessorEditor::FixedAngles){
            theEditor->moveFixedAngles(pointRelativeCenter);
        }
        else if(processor->getSelectedMovementConstraint() == ZirkOscjuceAudioProcessorEditor::FixedRadius){
            theEditor->moveCircularWithFixedRadius(pointRelativeCenter);
        }
        else if(processor->getSelectedMovementConstraint()  == ZirkOscjuceAudioProcessorEditor::FullyFixed){
            theEditor->moveFullyFixed(pointRelativeCenter); 
        }
        
    }
    const MessageManagerLock mmLock;
    theEditor->repaint();
    //processor->sendOSCValues();
    return 0;
}

Point <float> ZirkOscjuceAudioProcessor::domeToScreen (Point <float> p){
    float x,y;
    x = -ZirkOSC_DomeRadius * sinf((p.getX())) * cosf((p.getY()));
    y = -ZirkOSC_DomeRadius * cosf((p.getX())) * cosf((p.getY()));
    return Point <float> (x, y);
}
//==============================================================================
// This creates new instances of the plugin..
AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new ZirkOscjuceAudioProcessor();

}

int ZirkOscjuceAudioProcessor::getSelectedMovementConstraint(){
    return _SelectedMovementConstraint;
}
//
//void ZirkOscjuceAudioProcessor::setSelectedContrain(int constrain){
//    if (constrain<7 && constrain>0){
//        _SelectedConstrain = constrain;
//    }
//}
