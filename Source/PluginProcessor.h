/*
 ==============================================================================

 This file was auto-generated!

 It contains the basic startup code for a Juce application.
 Copyright 2013 Ludovic LAFFINEUR ludovic.laffineur@gmail.com
 ==============================================================================
 */

#ifndef __PLUGINPROCESSOR_H_F70DA35D__
#define __PLUGINPROCESSOR_H_F70DA35D__

#include "../JuceLibraryCode/JuceHeader.h"

#include "ZirkConstants.h"
#include "lo.h"     
#include "SoundSource.h"

//==============================================================================
/**
 The processor class of the plug in
 */
class ZirkOscjuceAudioProcessor  : public AudioProcessor,public Timer
{
public:
    //==============================================================================
    //! Constructeur
    ZirkOscjuceAudioProcessor();
    //! Destructeur
    ~ZirkOscjuceAudioProcessor();
    
    //! Called every 50ms;
    void timerCallback();
    
    
    //bool beginGesture =false;
    //==============================================================================
    //! Called before playback starts, to let the filter prepare itself. 
    void prepareToPlay (double sampleRate, int samplesPerBlock);
    //! Called after playback has stopped, to let the filter free up any resources it no longer needs. 
    void releaseResources();
    //static     int receivePositionUpdate(const char *path, const char *types, lo_arg **argv, int argc, void *data, void *user_data);
    
    //! Renders the next block. 
    void processBlock (AudioSampleBuffer& buffer, MidiBuffer& midiMessages);

    //==============================================================================
    //! Creates the editor and store it
    AudioProcessorEditor* createEditor();
    //! returns true if the processor has an editor
    bool hasEditor() const;

    //==============================================================================
    //! Returns the name of this processor. 
    const String getName() const;

    //! This must return the correct value immediately after the object has been created, and mustn't change the number of parameters later.
    int getNumParameters();
    //! Called by the host to find out the value of one of the filter's parameters.
    float getParameter (int index);
    //! The host will call this method to change the value of one of the filter's parameters. 
    void setParameter (int index, float newValue);
    //! Returns the name of a particular parameter. 
    const String getParameterName (int index);
    //! Returns the value of a parameter as a text string. 
    const String getParameterText (int index);
    
    //! Returns the number of input channels that the host will be sending the filter.
    const String getInputChannelName (int channelIndex) const;
    //! Returns the number of output channels that the host will be sending the filter. 
    const String getOutputChannelName (int channelIndex) const;
    //! Returns true if the specified channel is part of a stereo pair with its neighbour. 
    bool isInputChannelStereoPair (int index) const;
    //!Returns true if the specified channel is part of a stereo pair with its neighbour.
    bool isOutputChannelStereoPair (int index) const;

    //! Returns true if the processor wants midi messages. 
    bool acceptsMidi() const;
    //! Returns true if the processor produces midi messages. 
    bool producesMidi() const;
    //! Returns true if a silent input always produces a silent output.
    bool silenceInProducesSilenceOut() const;
    //! Projects sphere coords to polar coords 
    Point<float> domeToScreen(Point<float>);
    //==============================================================================
    //! Returns the number of preset programs the filter supports. 
    int getNumPrograms();
    //! Returns the number of the currently active program.
    int getCurrentProgram();
    //! Called by the host to change the current program. 
    void setCurrentProgram (int index);
    //! Must return the name of a given program. 
    const String getProgramName (int index);
    //! Called by the host to rename a program. 
    void changeProgramName (int index, const String& newName);


    //==============================================================================
    //! The host will call this method when it wants to save the filter's internal state. 
    void getStateInformation (MemoryBlock& destData);
    //! This must restore the filter's state from a block of data previously created using getStateInformation(). 
    void setStateInformation (const void* data, int sizeInBytes);
    
    //! returns the sources
    SoundSource* getSources(){ return _TabSource; }
    //! returns the number of sources on the screen.
    int getNbrSources() { return _NbrSources; }
    //! Set the number of sources.
    void setNbrSources(int newValue) { if ( newValue >-1 && newValue < 9) _NbrSources= newValue; }
    
    //! return the tab position of the selectedSource 
    int getSelectedSource() { return _SelectedSource; }
    //! Set the selected source 
    void setSelectedSource(int selected){ if ( selected >-1 && selected < 8) _SelectedSource= selected;};
    //! Returns the Osc Port for the Zirkonium sending
    int getOscPortZirkonium(){return _OscPortZirkonium;}
    //! Retunrs the Osc port where the iPad messages are received
    String getOscPortIpadIncoming(){ return _OscPortIpadIncoming;}
    //! Returns the Osc iPad port where we send messages
    String getOscPortIpadOutgoing(){ return _OscPortIpadOutgoing;}
    //! Returns the iPad's IP address.
    String getOscAddressIpad() {return _OscAddressIpad; }
    
    enum Parameters
    {
        ZirkOSC_Azim_Param = 0,
        ZirkOSC_Elev_Param,
        ZirkOSC_Channel_Param,
        ZirkOSC_ElevDelta_Param,
        ZirkOSC_AzimSpan_Param,
        ZirkOSC_ElevSpan_Param,
        ZirkOSC_Gain_Param,
        ZirkOSC_Azim_Param_1,
        ZirkOSC_Elev_Param_1,
        ZirkOSC_Channel_Param_1,
        ZirkOSC_ElevDelta_Param_1,
        ZirkOSC_AzimSpan_Param_1,
        ZirkOSC_ElevSpan_Param_1,
        ZirkOSC_Gain_Param_1,
        ZirkOSC_Azim_Param_2,
        ZirkOSC_Elev_Param_2,
        ZirkOSC_Channel_Param_2,
        ZirkOSC_ElevDelta_Param_2,
        ZirkOSC_AzimSpan_Param_2,
        ZirkOSC_ElevSpan_Param_2,
        ZirkOSC_Gain_Param_2,
        ZirkOSC_Azim_Param_3,
        ZirkOSC_Elev_Param_3,
        ZirkOSC_Channel_Param_3,
        ZirkOSC_ElevDelta_Param_3,
        ZirkOSC_AzimSpan_Param_3,
        ZirkOSC_ElevSpan_Param_3,
        ZirkOSC_Gain_Param_3,
        ZirkOSC_Azim_Param_4,
        ZirkOSC_Elev_Param_4,
        ZirkOSC_Channel_Param_4,
        ZirkOSC_ElevDelta_Param_4,
        ZirkOSC_AzimSpan_Param_4,
        ZirkOSC_ElevSpan_Param_4,
        ZirkOSC_Gain_Param_4,
        ZirkOSC_Azim_Param_5,
        ZirkOSC_Elev_Param_5,
        ZirkOSC_Channel_Param_5,
        ZirkOSC_ElevDelta_Param_5,
        ZirkOSC_AzimSpan_Param_5,
        ZirkOSC_ElevSpan_Param_5,
        ZirkOSC_Gain_Param_5,
        ZirkOSC_Azim_Param_6,
        ZirkOSC_Elev_Param_6,
        ZirkOSC_Channel_Param_6,
        ZirkOSC_ElevDelta_Param_6,
        ZirkOSC_AzimSpan_Param_6,
        ZirkOSC_ElevSpan_Param_6,
        ZirkOSC_Gain_Param_6,
        ZirkOSC_Azim_Param_7,
        ZirkOSC_Elev_Param_7,
        ZirkOSC_Channel_Param_7,
        ZirkOSC_ElevDelta_Param_7,
        ZirkOSC_AzimSpan_Param_7,
        ZirkOSC_ElevSpan_Param_7,
        ZirkOSC_Gain_Param_7,
        totalNumParams
    };
    //! Send the current state to all the iPad and Zirkonium
    void sendOSCValues();
    //! Getter constrain type
    int getSelectedConstrain();
    //! Setter constrain type
    void setSelectedContrain(int constrain);
    //! Retunrs true if the Editor has to refresh the Gui.
    bool hasToRefreshGui(){return _RefreshGui;};
    //! If you want to refresh the Gui from the Processor.
    void setRefreshGui(bool gui) { _RefreshGui = gui;};
    //! Change the sending OSC port of the zirkonium
    void changeOSCPort(int newPort);
    //! Send the configuration to the iPad (assignment Position -> id, nbr source)
    void sendOSCConfig();
    //! Send the movement type, selected constrain.
    void sendOSCMovementType();
    //! Change the sending OSC port of the iPad
    void changeOSCSendIPad(int newPort, String newAddress);
    //! Change the receiving OSC port (server).
    void changeOSCPortReceive(int port);
    //! Returns the Editor.
    AudioProcessorEditor* getEditor() {return _Editor;};
    
private:
    
    

    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ZirkOscjuceAudioProcessor)
    //! If the editor has to refresh the GUI
    bool _RefreshGui = false;
    //! Current number of sources on the screnn
    int _NbrSources;
    //! ID of the selected constrain
    int _SelectedConstrain=1;
    //! Tab position of the selected source
    int _SelectedSource;
    //! Osc port to send to the Zirkonium 
    int _OscPortZirkonium;
    //! The editor
    AudioProcessorEditor* _Editor;
    //! Sources tab
    SoundSource _TabSource [8];
    //! Osc Sever thread (receiving)
    lo_server_thread _St;
    //! Zirkonium OSC address (sending)
    lo_address _OscZirkonium;
    //! Ipad OSC address (sending)
    lo_address _OscIpad;
    //! Outgoing port to the iPad 
    String _OscPortIpadOutgoing = "10112";
    //! iPad address
    String _OscAddressIpad = "10.0.1.3";
    //! Zirkonium incoming port
    String _OscPortIpadIncoming = "10114";

};

#endif  // __PLUGINPROCESSOR_H_F70DA35D__
