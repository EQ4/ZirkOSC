/*
 ==============================================================================
 ZirkOSC2: VST and AU audio plug-in enabling spatial movement of sound sources in a dome of speakers.
 
 Copyright (C) 2015  GRIS-UdeM
 
 Developers: Ludovic Laffineur, Vincent Berthiaume
 
 This program is free software: you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation, either version 3 of the License, or
 (at your option) any later version.
 
 This program is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.
 
 You should have received a copy of the GNU General Public License
 along with this program.  If not, see <http://www.gnu.org/licenses/>.
 ==============================================================================
 */

#ifndef DEBUG
#define DEBUG
#endif
#undef DEBUG

//lo_send(mOsc, "/pan/az", "i", ch);

#include "PluginEditor.h"
#include "ZirkConstants.h"
#include <string.h>
#include <sstream>
#include <regex.h>
#include <arpa/inet.h>  //for inet_pton

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

int ZirkOscjuceAudioProcessor::s_iDomeRadius = 172;//150;

ZirkOscjuceAudioProcessor::ZirkOscjuceAudioProcessor()
:_NbrSources(1),
_SelectedMovementConstraint(.0f),
m_iSelectedMovementConstraint(Independant),
_SelectedTrajectory(.0f),
m_fSelectedTrajectoryDirection(.0f),
m_fSelectedTrajectoryReturn(.0f),
_SelectedSource(0),
_OscPortZirkonium(18032),
_OscPortIpadOutgoing("10112"),
_OscAddressIpad("10.0.1.3"),
_OscPortIpadIncoming("10114"),
_isOscActive(true),
_isSpanLinked(true),
m_parameterBuffer(),
_TrajectoryCount(0),
_TrajectoriesDuration(0),
//_TrajectoriesPhiAsin(0),
//_TrajectoriesPhiAcos(0),
_isSyncWTempo(false),
_isWriteTrajectory(false),
_SelectedSourceForTrajectory(0)
{
    //this toggles everything related to the ipad
    m_bUseIpad = true;
    
    for(int i=0; i<8; ++i){
        _AllSources[i]=SoundSource(0.0+((float)i/8.0),0.0);
    }
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
    _LastUiWidth = ZirkOSC_Window_Default_Width;
    _LastUiHeight = ZirkOSC_Window_Default_Height;
    
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

void ZirkOscjuceAudioProcessor::processBlock (AudioSampleBuffer& buffer, MidiBuffer& midiMessages)
{
    double sampleRate = getSampleRate();
    unsigned int oriFramesToProcess = buffer.getNumSamples();
    
    Trajectory::Ptr trajectory = mTrajectory;
    if (trajectory)
    {
        AudioPlayHead::CurrentPositionInfo cpi;
        getPlayHead()->getCurrentPosition(cpi);
        
        if (cpi.isPlaying && cpi.timeInSamples != mLastTimeInSamples)
        {
            // we're playing!
            mLastTimeInSamples = cpi.timeInSamples;
            
            double bps = cpi.bpm / 60;
            float seconds = oriFramesToProcess / sampleRate;
            float beats = seconds * bps;
            
            bool done = trajectory->process(seconds, beats);
            if (done){
                mTrajectory = NULL;
                _isWriteTrajectory = false;
            }
        }
    }

}

void ZirkOscjuceAudioProcessor::storeCurrentLocations(){
    for (int iCurSource = 0; iCurSource<8; ++iCurSource){
        m_parameterBuffer._AllSources[iCurSource] = _AllSources[iCurSource];
    }
}

void ZirkOscjuceAudioProcessor::restoreCurrentLocations(){
    for (int iCurSource = 0; iCurSource<8; ++iCurSource){
        _AllSources[iCurSource] = m_parameterBuffer._AllSources[iCurSource];
    }
    //_AllSources[0].setAzimReverse(true);

}

void ZirkOscjuceAudioProcessor::moveTrajectoriesWithConstraints(Point<float> &newLocation){

    JUCE_COMPILER_WARNING("this is terrible. processor calls editor calls processor")
    ZirkOscjuceAudioProcessorEditor* editor = (ZirkOscjuceAudioProcessorEditor*) getEditor();
    //if we get here, we're not in independent mode
    if (m_iSelectedMovementConstraint == FixedAngles){
        editor->moveFixedAngles(newLocation);
    }
    else if (m_iSelectedMovementConstraint == FixedRadius){
        editor->moveCircularWithFixedRadius(newLocation);
    }
    else if (m_iSelectedMovementConstraint == FullyFixed){
        editor->moveFullyFixed(newLocation);
    }
    else if (m_iSelectedMovementConstraint == DeltaLocked){
        Point<float> DeltaMove = newLocation - getSources()[_SelectedSourceForTrajectory].getPositionXY();
        editor->moveSourcesWithDelta(DeltaMove);
    }
    else if (m_iSelectedMovementConstraint == Circular){
        editor->moveCircular(newLocation);
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
    return 1;
}

int ZirkOscjuceAudioProcessor::getCurrentProgram()
{
    return 1;
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

void ZirkOscjuceAudioProcessor::setSelectedSourceForTrajectory(int iSelectedSource){
    _SelectedSourceForTrajectory = iSelectedSource;
}

int ZirkOscjuceAudioProcessor::getSelectedSourceForTrajectory(){
    return _SelectedSourceForTrajectory;
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


//set wheter plug is sending osc messages to zirkonium
void ZirkOscjuceAudioProcessor::setIsOscActive(bool isOscActive){
    _isOscActive = isOscActive;
}

//wheter plug is sending osc messages to zirkonium
bool ZirkOscjuceAudioProcessor::getIsOscActive(){
    return _isOscActive;
}

void ZirkOscjuceAudioProcessor::setIsSyncWTempo(bool isSyncWTempo){
    _isSyncWTempo = isSyncWTempo;
}

bool ZirkOscjuceAudioProcessor::getIsSyncWTempo(){
    return _isSyncWTempo;
}


void ZirkOscjuceAudioProcessor::setIsSpanLinked(bool isSpanLinked){
    _isSpanLinked = isSpanLinked;
}

bool ZirkOscjuceAudioProcessor::getIsSpanLinked(){
    return _isSpanLinked;
}

void ZirkOscjuceAudioProcessor::setIsWriteTrajectory(bool isWriteTrajectory){
    _isWriteTrajectory = isWriteTrajectory;
}

bool ZirkOscjuceAudioProcessor::getIsWriteTrajectory(){
    return _isWriteTrajectory;
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

// This method will be called by the host, probably on the audio thread, so
// it's absolutely time-critical. Don't use critical sections or anything
// UI-related, or anything at all that may block in any way!
float ZirkOscjuceAudioProcessor::getParameter (int index)
{
    switch (index){
        case ZirkOSC_MovementConstraint_ParamId:
            return _SelectedMovementConstraint;
        case ZirkOSC_isOscActive_ParamId:
            if (_isOscActive)
                return 1.0f;
            else
                return 0.0f;
        case ZirkOSC_isSpanLinked_ParamId:
            if (_isSpanLinked)
                return 1.0f;
            else
                return 0.0f;
        case ZirkOSC_SelectedTrajectory_ParamId:
            return _SelectedTrajectory;
        case ZirkOSC_SelectedTrajectoryDirection_ParamId:
            return m_fSelectedTrajectoryDirection;
        case ZirkOSC_SelectedTrajectoryReturn_ParamId:
            return m_fSelectedTrajectoryReturn;
        case ZirkOSC_TrajectoryCount_ParamId:
            return _TrajectoryCount;
        case ZirkOSC_TrajectoriesDuration_ParamId:
            return _TrajectoriesDuration;
        case ZirkOSC_SyncWTempo_ParamId:
            if (_isSyncWTempo)
                return 1.0f;
            else
                return 0.0f;
        case ZirkOSC_WriteTrajectories_ParamId:
            if (_isWriteTrajectory)
                return 1.0f;
            else
                return 0.0f;
    }
    
    for(int i = 0; i<8;++i){
        if      (ZirkOSC_Azim_ParamId + (i*5) == index)       {
            if (g_bUseXY)
                return (_AllSources[i].getX() + s_iDomeRadius) / (2.f*s_iDomeRadius); //we normalize this value to [0,1]
            else
                return _AllSources[i].getAzimuth();
        }
        else if (ZirkOSC_AzimSpan_ParamId + (i*5) == index)   return _AllSources[i].getAzimuthSpan();
        else if (ZirkOSC_Elev_ParamId + (i*5) == index)       {
            
            if (g_bUseXY)
                return (_AllSources[i].getY() + s_iDomeRadius) / (2.f*s_iDomeRadius); //we normalize this value to [0,1]
            else
                return _AllSources[i].getElevation();
        }
        else if (ZirkOSC_ElevSpan_ParamId + (i*5) == index)   return _AllSources[i].getElevationSpan();
        else if (ZirkOSC_Gain_ParamId + (i*5) == index)       return _AllSources[i].getGain();
    }
    cerr << "\n" << "wrong parameter id: " << index << "in ZirkOscjuceAudioProcessor::getParameter" << "\n";
    return -1.f;
}

// This method will be called by the host, probably on the audio thread, so
// it's absolutely time-critical. Don't use critical sections or anything
// UI-related, or anything at all that may block in any way!
void ZirkOscjuceAudioProcessor::setParameter (int index, float newValue)
{
#if defined(DEBUG)
    cout << "setParameter() with index: " << index << " and newValue: " << newValue << "\n";
#endif
    switch (index){
        case ZirkOSC_MovementConstraint_ParamId:
            _SelectedMovementConstraint = newValue;
            //m_iSelectedMovementConstraint = _SelectedMovementConstraint * (TotalNumberConstraints-1) + 1;
            m_iSelectedMovementConstraint = PercentToIntStartsAtOne(_SelectedMovementConstraint, TotalNumberConstraints);
            return;
        case ZirkOSC_isOscActive_ParamId:
            if (newValue > .5f)
                _isOscActive = true;
            else
                _isOscActive = false;
            return;
        case ZirkOSC_isSpanLinked_ParamId:
            if (newValue > .5f)
                _isSpanLinked = true;
            else
                _isSpanLinked = false;
            return;
        case ZirkOSC_SelectedTrajectory_ParamId:
            _SelectedTrajectory = newValue;
            return;
            
        case ZirkOSC_SelectedTrajectoryDirection_ParamId:
            m_fSelectedTrajectoryDirection = newValue;
            return;

        case ZirkOSC_SelectedTrajectoryReturn_ParamId:
            m_fSelectedTrajectoryReturn = newValue;
            return;
            
        case ZirkOSC_TrajectoryCount_ParamId:
            _TrajectoryCount = newValue;
            return;
        case ZirkOSC_TrajectoriesDuration_ParamId:
            _TrajectoriesDuration = newValue;
            return;
        case ZirkOSC_SyncWTempo_ParamId:
            if (newValue > .5f)
                _isSyncWTempo = true;
            else
                _isSyncWTempo = false;
            return;
        case ZirkOSC_WriteTrajectories_ParamId:
            if (newValue > .5f)
                _isWriteTrajectory = true;
            else
                _isWriteTrajectory = false;
            return;
    }
    //cout << "setParameter: " << index << " with value: " << newValue << "\n";
    for(int i = 0; i<8; ++i){
        if      (ZirkOSC_Azim_ParamId + (i*5) == index)       {_AllSources[i].setAzimuth(newValue); return;}
        else if (ZirkOSC_AzimSpan_ParamId + (i*5) == index)   {_AllSources[i].setAzimuthSpan(newValue); return;}
        else if (ZirkOSC_Elev_ParamId + (i*5) == index)       {_AllSources[i].setElevation(newValue); return;}
        else if (ZirkOSC_ElevSpan_ParamId + (i*5) == index)   {_AllSources[i].setElevationSpan(newValue); return;}
        else if (ZirkOSC_Gain_ParamId + (i*5) == index)       {_AllSources[i].setGain(newValue); return;}
    }
    cerr << "\n" << "wrong parameter id: " << index << " in ZirkOscjuceAudioProcessor::setParameter" << "\n";
}

const String ZirkOscjuceAudioProcessor::getParameterName (int index)
{
    switch (index){
        case ZirkOSC_MovementConstraint_ParamId:
            return ZirkOSC_Movement_Constraint_name;
        case ZirkOSC_isOscActive_ParamId:
            return ZirkOSC_isOscActive_name;
        case ZirkOSC_isSpanLinked_ParamId:
            return ZirkOSC_isSpanLinked_name;
        case ZirkOSC_SelectedTrajectory_ParamId:
            return ZirkOSC_SelectedTrajectory_name;
        case ZirkOSC_SelectedTrajectoryDirection_ParamId:
            return ZirkOSC_SelectedTrajectoryDirection_name;
        case ZirkOSC_SelectedTrajectoryReturn_ParamId:
            return ZirkOSC_SelectedTrajectoryReturn_name;
        case ZirkOSC_TrajectoryCount_ParamId:
            return ZirkOSC_NbrTrajectories_name;
        case ZirkOSC_TrajectoriesDuration_ParamId:
            return ZirkOSC_DurationTrajectories_name;
        case ZirkOSC_SyncWTempo_ParamId:
            return ZirkOSC_isSyncWTempo_name;
        case ZirkOSC_WriteTrajectories_ParamId:
            return ZirkOSC_isWriteTrajectory_name;
    }
    
    
    for(int i = 0; i<8;++i){
        string strSourceId = " Src: " + std::to_string(getSources()[i].getChannel());
        if      (ZirkOSC_Azim_ParamId + (i*5) == index)       return ZirkOSC_Azim_name[i] + strSourceId;
        else if (ZirkOSC_AzimSpan_ParamId + (i*5) == index)   return ZirkOSC_AzimSpan_name[i] + strSourceId;
        else if (ZirkOSC_Elev_ParamId + (i*5) == index)       return ZirkOSC_Elev_name[i] + strSourceId;
        else if (ZirkOSC_ElevSpan_ParamId + (i*5) == index)   return ZirkOSC_ElevSpan_name[i] + strSourceId;
        else if (ZirkOSC_Gain_ParamId + (i*5) == index)       return ZirkOSC_Gain_name[i] + strSourceId;
    }
    return String::empty;
}

static const int g_kiDataVersion = 3;

//==============================================================================
void ZirkOscjuceAudioProcessor::getStateInformation (MemoryBlock& destData)
{
    // You should use this method to store your parameters in the memory block.
    // You could do that either as raw data, or use the XML or ValueTree classes
    // as intermediaries to make it easy to save and load complex data.
    XmlElement xml ("ZIRKOSCJUCESETTINGS");
    JUCE_COMPILER_WARNING("need to remove this condition after we've successfully moved to using XY")
    if (g_bUseXY){
        xml.setAttribute ("presetDataVersion", g_kiDataVersion);
    }
    xml.setAttribute ("uiWidth", _LastUiWidth);
    xml.setAttribute ("uiHeight", _LastUiHeight);
    xml.setAttribute("PortOSC", _OscPortZirkonium);
    xml.setAttribute("IncPort", _OscPortIpadIncoming);
    xml.setAttribute("OutPort", _OscPortIpadOutgoing);
    xml.setAttribute("AddIpad", _OscAddressIpad);
    xml.setAttribute("NombreSources", _NbrSources);
    xml.setAttribute("MovementConstraint", _SelectedMovementConstraint);
    xml.setAttribute("isSpanLinked", _isSpanLinked);
    xml.setAttribute("isOscActive", _isOscActive);
    xml.setAttribute("selectedTrajectory", _SelectedTrajectory);
    xml.setAttribute("nbrTrajectory", _TrajectoryCount);
    xml.setAttribute("durationTrajectory", _TrajectoriesDuration);
    xml.setAttribute("isSyncWTempo", _isSyncWTempo);
    xml.setAttribute("isWriteTrajectory", _isWriteTrajectory);
    
    for(int i =0;i<8;++i){
        String channel = "Channel";
        String azimuth = "Azimuth";
        String elevation = "Elevation";
        String azimuthSpan = "AzimuthSpan";
        String elevationSpan = "ElevationSpan";
        String gain = "Gain";
        channel.append(String(i), 10);
        xml.setAttribute(channel, _AllSources[i].getChannel());
        azimuth.append(String(i), 10);
        if (g_bUseXY){
            xml.setAttribute(azimuth, _AllSources[i].getX());//_AllSources[i].getAzimuth());
        } else {
            xml.setAttribute(azimuth, _AllSources[i].getAzimuth());
        }
        elevation.append(String(i), 10);
        if (g_bUseXY){
            xml.setAttribute(elevation, _AllSources[i].getY());//_AllSources[i].getElevationRawValue());
        } else {
            xml.setAttribute(elevation, _AllSources[i].getElevationRawValue());
        }
            
        azimuthSpan.append(String(i), 10);
        xml.setAttribute(azimuthSpan, _AllSources[i].getAzimuthSpan());
        elevationSpan.append(String(i), 10);
        xml.setAttribute(elevationSpan, _AllSources[i].getElevationSpan());
        gain.append(String(i), 10);
        xml.setAttribute(gain, _AllSources[i].getGain());
        
    }

    //version 2
    xml.setAttribute("selectedTrajectoryDirection", m_fSelectedTrajectoryDirection);
    xml.setAttribute("selectedTrajectoryReturn", m_fSelectedTrajectoryReturn);
    
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
            // ok, now pull out our parameters. format is getIntAttribute("AttributeName: defaultValue);

            int version = static_cast<int>(xmlState->getIntAttribute("presetDataVersion", 1));
            
            _LastUiWidth  = xmlState->getIntAttribute ("uiWidth", _LastUiWidth);
            _LastUiHeight = xmlState->getIntAttribute ("uiHeight", _LastUiHeight);
            _OscPortZirkonium = xmlState->getIntAttribute("PortOSC", 18032);
            _OscPortIpadIncoming = xmlState->getStringAttribute("IncPort", "10114");
            _OscPortIpadOutgoing = xmlState->getStringAttribute("OutPort", "10112");
            _OscAddressIpad = xmlState -> getStringAttribute("AddIpad", "10.0.1.3");
            _NbrSources = xmlState->getIntAttribute("NombreSources", 1);
            _SelectedMovementConstraint = static_cast<float>(xmlState->getDoubleAttribute("MovementConstraint", .2f));
            
            //m_iSelectedMovementConstraint = _SelectedMovementConstraint * (TotalNumberConstraints-1) + 1;
            m_iSelectedMovementConstraint = PercentToIntStartsAtOne(_SelectedMovementConstraint, TotalNumberConstraints);
            
            _isOscActive = xmlState->getBoolAttribute("isOscActive", true);
            _isSpanLinked = xmlState->getBoolAttribute("isSpanLinked", false);
            _SelectedTrajectory = static_cast<float>(xmlState->getDoubleAttribute("selectedTrajectory", .0f));
            _TrajectoryCount = xmlState->getIntAttribute("nbrTrajectory", 0);
            _TrajectoriesDuration = static_cast<float>(xmlState->getDoubleAttribute("durationTrajectory", .0f));
            _isSyncWTempo = xmlState->getBoolAttribute("isSyncWTempo", false);
            _isWriteTrajectory = xmlState->getBoolAttribute("isWriteTrajectory", false);
            
            
            for (int i=0;i<8;++i){
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
                _AllSources[i].setChannel(xmlState->getIntAttribute(channel , 0));
                if (version == 1 ){
                    //in version 1, we were storing azimuth and elevation instead of x and y
                    g_bUseXY = false;
                    _AllSources[i].setAzimuth((float) xmlState->getDoubleAttribute(azimuth,0));
                    _AllSources[i].setElevation((float) xmlState->getDoubleAttribute(elevation,0));
                } else {
                    g_bUseXY = true;
                    Point<float> p((float) xmlState->getDoubleAttribute(azimuth,0), (float) xmlState->getDoubleAttribute(elevation,0));
                    _AllSources[i].setPositionXY(p);
                }
                cout << "setState g_bUseXY = " << g_bUseXY << "\n";
                _AllSources[i].setAzimuthSpan((float) xmlState->getDoubleAttribute(azimuthSpan,0));
                _AllSources[i].setElevationSpan((float) xmlState->getDoubleAttribute(elevationSpan,0));
                float fGain = (float) xmlState->getDoubleAttribute(gain,1 );
                _AllSources[i].setGain(fGain);
            }
            
            m_fSelectedTrajectoryDirection = static_cast<float>(xmlState->getDoubleAttribute("selectedTrajectoryDirection", .0f));
            m_fSelectedTrajectoryReturn = static_cast<float>(xmlState->getDoubleAttribute("selectedTrajectoryReturn", .0f));
            
            changeZirkoniumOSCPort(_OscPortZirkonium);
            changeOSCReceiveIpad(_OscPortIpadIncoming.getIntValue());
            changeOSCSendIPad(_OscPortIpadOutgoing.getIntValue(), _OscAddressIpad);
            sendOSCValues();
            _RefreshGui=true;
        }
    }
}

void ZirkOscjuceAudioProcessor::sendOSCMovementType(){ //should be void with no argument if movement is included in the processor!!!!!
    //lo_send(_OscIpad, "/movementmode", "i", _SelectedMovementConstraint);
    if (m_bUseIpad){
        lo_send(_OscIpad, "/movementmode", "f", _SelectedMovementConstraint);
    }

}

void ZirkOscjuceAudioProcessor::sendOSCValues(){
    if (_isOscActive){
        for(int i=0;i<_NbrSources;++i){
            float azim_osc = PercentToHR(_AllSources[i].getAzimuth(), ZirkOSC_Azim_Min, ZirkOSC_Azim_Max) /180.;
            float elev_osc = PercentToHR(_AllSources[i].getElevation(), ZirkOSC_Elev_Min, ZirkOSC_Elev_Max)/180.;
            float azimspan_osc = PercentToHR(_AllSources[i].getAzimuthSpan(), ZirkOSC_AzimSpan_Min,ZirkOSC_AzimSpan_Max)/180.;
            float elevspan_osc = PercentToHR(_AllSources[i].getElevationSpan(), ZirkOSC_ElevSpan_Min, ZirkOSC_Elev_Max)/180.;
            int channel_osc = _AllSources[i].getChannel()-1;
            float gain_osc = _AllSources[i].getGain();
            lo_send(_OscZirkonium, "/pan/az", "ifffff", channel_osc, azim_osc, elev_osc, azimspan_osc, elevspan_osc, gain_osc);
            
            if (m_bUseIpad){
                azim_osc = azim_osc * M_PI;
                elev_osc = elev_osc * M_PI;
                azimspan_osc = azimspan_osc * M_PI;
                elevspan_osc = elevspan_osc * M_PI;
                lo_send(_OscIpad, "/pan/az", "ifffff", channel_osc+1, azim_osc, elev_osc, azimspan_osc, elevspan_osc, gain_osc);
            }
        }
    }
}

void ZirkOscjuceAudioProcessor::sendOSCConfig(){
    if (_isOscActive && m_bUseIpad){
        lo_send(_OscIpad, "/maxsource", "iiiiiiiii", _NbrSources, _AllSources[0].getChannel(), _AllSources[1].getChannel(), _AllSources[2].getChannel(), _AllSources[3].getChannel(), _AllSources[4].getChannel(), _AllSources[5].getChannel(), _AllSources[6].getChannel(), _AllSources[7].getChannel());
    }
}

void ZirkOscjuceAudioProcessor::changeZirkoniumOSCPort(int newPort){
    
    if(newPort<0 || newPort>100000){
        newPort = _OscPortZirkonium;//18032;
    }
    
    lo_address osc = _OscZirkonium;
    _OscPortZirkonium = newPort;
	_OscZirkonium = NULL;
    lo_address_free(osc);
	char port[32];
	snprintf(port, sizeof(port), "%d", newPort);
	_OscZirkonium = lo_address_new("127.0.0.1", port);
    
}

bool validateIpAddress(const string &ipAddress)
{
    struct sockaddr_in sa;
    int result = inet_pton(AF_INET, ipAddress.c_str(), &(sa.sin_addr));
    return result != 0;
}

void ZirkOscjuceAudioProcessor::changeOSCSendIPad(int newPort, String newAddress){
    
    if (m_bUseIpad){
        //if port is outside range, assign previous
        if(newPort<0 || newPort>100000){
            newPort = _OscPortIpadOutgoing.getIntValue();//10112;
        }
        
        //if address is invalid, assign previous
        if (!validateIpAddress(newAddress.toStdString())){
            newAddress = _OscAddressIpad;//"10.0.1.3";
        }
        
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
}

void ZirkOscjuceAudioProcessor::changeOSCReceiveIpad(int newPort){
    if (m_bUseIpad){
        if(newPort<0 || newPort>100000){
            newPort = _OscPortIpadIncoming.getIntValue();//10114;
        }
        
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
}

int receiveBeginTouch(const char *path, const char *types, lo_arg **argv, int argc,void *data, void *user_data){
    ZirkOscjuceAudioProcessor *processor = (ZirkOscjuceAudioProcessor*) user_data;
    ZirkOscjuceAudioProcessorEditor* editor = (ZirkOscjuceAudioProcessorEditor*) processor->getEditor();
    //printf("Receive BEGIN");
    int channel_osc = argv[0]->i;
    int i =0;
    for(i=0;i<processor->getNbrSources();++i){
        if (processor->getSources()[i].getChannel() == channel_osc) {
            break;
        }
    }
    if (i==processor->getNbrSources()){
        return 0;
    }
    
    if (processor->getSelectedMovementConstraintAsInteger() == Independant){
        processor->beginParameterChangeGesture(ZirkOscjuceAudioProcessor::ZirkOSC_Azim_ParamId+ i*5);
        processor->beginParameterChangeGesture(ZirkOscjuceAudioProcessor::ZirkOSC_Elev_ParamId+ i*5);
    }
    else{
        for (int j = 0; j<processor->getNbrSources() ;j++){
            processor->beginParameterChangeGesture(ZirkOscjuceAudioProcessor::ZirkOSC_Azim_ParamId+ j*5);
            processor->beginParameterChangeGesture(ZirkOscjuceAudioProcessor::ZirkOSC_Elev_ParamId+ j*5);
            
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
    for(i=0;i<processor->getNbrSources();++i){
        if (processor->getSources()[i].getChannel() == channel_osc) {
            break;
        }
    }
    if (i==processor->getNbrSources()){
        return 0;
    }
    if (processor->getSelectedMovementConstraintAsInteger() != Independant){
        for (int j = 0; j<processor->getNbrSources() ;j++){
            processor->endParameterChangeGesture(ZirkOscjuceAudioProcessor::ZirkOSC_Azim_ParamId+ j*5);
            processor->endParameterChangeGesture(ZirkOscjuceAudioProcessor::ZirkOSC_Elev_ParamId+ j*5);
            
        }
        theEditor->setFixedAngle(false);
    }
    else{
        if (processor->getSelectedMovementConstraintAsInteger() == Independant){
            processor->endParameterChangeGesture(ZirkOscjuceAudioProcessor::ZirkOSC_Azim_ParamId+ i*5);
            processor->endParameterChangeGesture(ZirkOscjuceAudioProcessor::ZirkOSC_Elev_ParamId+ i*5);
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
    for(i=0;i<processor->getNbrSources();++i){
        if (processor->getSources()[i].getChannel() == channel_osc) {
            break;
        }
    }
    if (i==processor->getNbrSources()){
        return 0;
    }
    if (processor->getSelectedMovementConstraintAsInteger() != Independant){
        
        for (int j = 0; j<processor->getNbrSources() ;j++){
            processor->setParameterNotifyingHost (ZirkOscjuceAudioProcessor::ZirkOSC_AzimSpan_ParamId + j*5,
                                                  HRToPercent(value, ZirkOSC_AzimSpan_Min, ZirkOSC_AzimSpan_Max));
            
        }
    }
    else{
        if (processor->getSelectedMovementConstraintAsInteger() == Independant){
            processor->setParameterNotifyingHost (ZirkOscjuceAudioProcessor::ZirkOSC_AzimSpan_ParamId + i*5,
                                                  HRToPercent(value, ZirkOSC_AzimSpan_Min, ZirkOSC_AzimSpan_Max));
            printf("I am right there : %d, %f", channel_osc,value);
        }
    }
    return 0;
}

int ZirkOscjuceAudioProcessor::getSelectedMovementConstraintAsInteger() {
    return m_iSelectedMovementConstraint;
}

int ZirkOscjuceAudioProcessor::getSelectedTrajectoryAsInteger() {
    int value = PercentToIntStartsAtOne(_SelectedTrajectory, TotalNumberTrajectories);
    return value;
}

int ZirkOscjuceAudioProcessor::getSelectedTrajectoryDirection() {
    int value = PercentToIntStartsAtOne(m_fSelectedTrajectoryDirection, TotalNumberTrajectories);
    return value;
}

int ZirkOscjuceAudioProcessor::getSelectedTrajectoryReturn() {
    int value = PercentToIntStartsAtOne(m_fSelectedTrajectoryReturn, TotalNumberTrajectories);
    return value;
}


int receiveAzimuthSpanBegin(const char *path, const char *types, lo_arg **argv, int argc, void *data, void *user_data){
    ZirkOscjuceAudioProcessor *processor = (ZirkOscjuceAudioProcessor*) user_data;
    int channel_osc = argv[0]->i;
    int i;
    for(i=0;i<processor->getNbrSources();++i){
        if (processor->getSources()[i].getChannel() == channel_osc) {
            break;
        }
    }
    if (i==processor->getNbrSources()){
        return 0;
    }
    if (processor->getSelectedMovementConstraintAsInteger() != Independant){
        
        for (int j = 0; j<processor->getNbrSources() ;j++){
            processor->beginParameterChangeGesture(ZirkOscjuceAudioProcessor::ZirkOSC_AzimSpan_ParamId + j*5);
            
        }
    }
    else{
        if (processor->getSelectedMovementConstraintAsInteger() == Independant){
            processor->beginParameterChangeGesture(ZirkOscjuceAudioProcessor::ZirkOSC_AzimSpan_ParamId + i*5);
        }
    }
    return 0;
}

int receiveAzimuthSpanEnd(const char *path, const char *types, lo_arg **argv, int argc, void *data, void *user_data){
    ZirkOscjuceAudioProcessor *processor = (ZirkOscjuceAudioProcessor*) user_data;
    int channel_osc = argv[0]->i;
    int i;
    for(i=0;i<processor->getNbrSources();++i){
        if (processor->getSources()[i].getChannel() == channel_osc) {
            break;
        }
    }
    if (i==processor->getNbrSources()){
        return 0;
    }
    if (processor->getSelectedMovementConstraintAsInteger() != Independant){
        
        for (int j = 0; j<processor->getNbrSources() ;j++){
            processor->endParameterChangeGesture(ZirkOscjuceAudioProcessor::ZirkOSC_AzimSpan_ParamId + j*5);
            
        }
    }
    else{
        if (processor->getSelectedMovementConstraintAsInteger() == Independant){
            processor->endParameterChangeGesture(ZirkOscjuceAudioProcessor::ZirkOSC_AzimSpan_ParamId + i*5);
        }
    }
    return 0;
}

int receiveElevationSpanUpdate(const char *path, const char *types, lo_arg **argv, int argc, void *data, void *user_data){
    ZirkOscjuceAudioProcessor *processor = (ZirkOscjuceAudioProcessor*) user_data;
    int channel_osc = argv[0]->i;
    int i;
    float value = argv[1]->f;
    for(i=0;i<processor->getNbrSources();++i){
        if (processor->getSources()[i].getChannel() == channel_osc) {
            break;
        }
    }
    if (i==processor->getNbrSources()){
        return 0;
    }
    if (processor->getSelectedMovementConstraintAsInteger() != Independant){
        
        for (int j = 0; j<processor->getNbrSources() ;j++){
            processor->setParameterNotifyingHost (ZirkOscjuceAudioProcessor::ZirkOSC_ElevSpan_ParamId + j*5,
                                                  HRToPercent(value, ZirkOSC_ElevSpan_Min, ZirkOSC_ElevSpan_Max));
            
        }
    }
    else{
        if (processor->getSelectedMovementConstraintAsInteger() == Independant){
            processor->setParameterNotifyingHost (ZirkOscjuceAudioProcessor::ZirkOSC_ElevSpan_ParamId + i*5,
                                                  HRToPercent(value, ZirkOSC_ElevSpan_Min, ZirkOSC_ElevSpan_Max));
        }
    }
    return 0;

}

int receiveElevationSpanBegin(const char *path, const char *types, lo_arg **argv, int argc, void *data, void *user_data){
    ZirkOscjuceAudioProcessor *processor = (ZirkOscjuceAudioProcessor*) user_data;
    int channel_osc = argv[0]->i;
    int i;
    for(i=0;i<processor->getNbrSources();++i){
        if (processor->getSources()[i].getChannel() == channel_osc) {
            break;
        }
    }
    if (i==processor->getNbrSources()){
        return 0;
    }
    if (processor->getSelectedMovementConstraintAsInteger() != Independant){
        
        for (int j = 0; j<processor->getNbrSources() ;j++){
            processor->beginParameterChangeGesture (ZirkOscjuceAudioProcessor::ZirkOSC_ElevSpan_ParamId + j*5);
        }
    }
    else{
        if (processor->getSelectedMovementConstraintAsInteger() == Independant){
            processor->beginParameterChangeGesture (ZirkOscjuceAudioProcessor::ZirkOSC_ElevSpan_ParamId + i*5);
        }
    }
    return 0;
    

}

int receiveElevationSpanEnd(const char *path, const char *types, lo_arg **argv, int argc, void *data, void *user_data){
    ZirkOscjuceAudioProcessor *processor = (ZirkOscjuceAudioProcessor*) user_data;
    int channel_osc = argv[0]->i;
    int i;
    for(i=0;i<processor->getNbrSources();++i){
        if (processor->getSources()[i].getChannel() == channel_osc) {
            break;
        }
    }
    if (i==processor->getNbrSources()){
        return 0;
    }
    if (processor->getSelectedMovementConstraintAsInteger() != Independant){
        
        for (int j = 0; j<processor->getNbrSources() ;j++){
            processor->endParameterChangeGesture (ZirkOscjuceAudioProcessor::ZirkOSC_ElevSpan_ParamId + j*5);
        }
    }
    else{
        if (processor->getSelectedMovementConstraintAsInteger() == Independant){
            processor->endParameterChangeGesture (ZirkOscjuceAudioProcessor::ZirkOSC_ElevSpan_ParamId + i*5);
        }
    }
    return 0;
}

int receivePositionUpdate(const char *path, const char *types, lo_arg **argv, int argc, void *data, void *user_data){
 
    
    //channel send ifffff", channel_osc, azim_osc, elev_osc, azimspan_osc, elevspan_osc, gain_osc); all angles in RADIAN
    ZirkOscjuceAudioProcessor *processor = (ZirkOscjuceAudioProcessor*) user_data;
    int channel_osc = argv[0]->i;
    int i =0;
    for(i=0;i<processor->getNbrSources();++i){
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
    if(processor->getSelectedMovementConstraintAsInteger() == Independant){
        processor->setParameterNotifyingHost (ZirkOscjuceAudioProcessor::ZirkOSC_Azim_ParamId + i*5,
                                              HRToPercent(azim_osc, -M_PI, M_PI));
        processor->setParameterNotifyingHost (ZirkOscjuceAudioProcessor::ZirkOSC_Elev_ParamId + i*5,
                                              HRToPercent(elev_osc, 0.0, M_PI/2.0));
    }
    else{
        processor->setSelectedSource(i);

        if(processor->getSelectedMovementConstraintAsInteger() == Circular){
            theEditor->moveCircular(pointRelativeCenter);
            
        }
        else if(processor->getSelectedMovementConstraintAsInteger()  == DeltaLocked){
            Point<float> DeltaMove = pointRelativeCenter - processor->getSources()[processor->getSelectedSource()].getPositionXY();
            theEditor->moveSourcesWithDelta(DeltaMove);
        }
        else if(processor->getSelectedMovementConstraintAsInteger()  == FixedAngles){
            theEditor->moveFixedAngles(pointRelativeCenter);
        }
        else if(processor->getSelectedMovementConstraintAsInteger() == FixedRadius){
            theEditor->moveCircularWithFixedRadius(pointRelativeCenter);
        }
        else if(processor->getSelectedMovementConstraintAsInteger()  == FullyFixed){
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
    x = -s_iDomeRadius * sinf((p.getX())) * cosf((p.getY()));
    y = -s_iDomeRadius * cosf((p.getX())) * cosf((p.getY()));
    return Point <float> (x, y);
}
//==============================================================================
// This creates new instances of the plugin..
AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new ZirkOscjuceAudioProcessor();

}

