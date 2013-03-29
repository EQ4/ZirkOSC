/*
 ==============================================================================

 This file was auto-generated!

 It contains the basic startup code for a Juce application.

 ==============================================================================
 */

#ifndef __PLUGINPROCESSOR_H_F70DA35D__
#define __PLUGINPROCESSOR_H_F70DA35D__

#include "../JuceLibraryCode/JuceHeader.h"

#include "ZirkConstants.h"
#include "lo.h"
#include <list.h>

#include "SoundSource.h";

//==============================================================================
/**
 */
class ZirkOscjuceAudioProcessor  : public AudioProcessor
{
public:
    //==============================================================================
    ZirkOscjuceAudioProcessor();
    ~ZirkOscjuceAudioProcessor();



    list<SoundSource> listeSource;
    list<SoundSource>::iterator currentSource;

    bool refreshGui = false;
    SoundSource tabSource [8];
    int selectedSource;
    int nbrSources;
    //==============================================================================
    void prepareToPlay (double sampleRate, int samplesPerBlock);
    void releaseResources();

    void processBlock (AudioSampleBuffer& buffer, MidiBuffer& midiMessages);

    //==============================================================================
    AudioProcessorEditor* createEditor();
    bool hasEditor() const;

    //==============================================================================
    const String getName() const;

    int getNumParameters();


    float getParameter (int index);
    void setParameter (int index, float newValue);

    const String getParameterName (int index);
    const String getParameterText (int index);

    const String getInputChannelName (int channelIndex) const;
    const String getOutputChannelName (int channelIndex) const;
    bool isInputChannelStereoPair (int index) const;
    bool isOutputChannelStereoPair (int index) const;

    void changeOSCPort(int newPort);

    bool acceptsMidi() const;
    bool producesMidi() const;
    bool silenceInProducesSilenceOut() const;

    //==============================================================================
    int getNumPrograms();
    int getCurrentProgram();
    void setCurrentProgram (int index);
    const String getProgramName (int index);
    void changeProgramName (int index, const String& newName);

    //==============================================================================
    void getStateInformation (MemoryBlock& destData);
    void setStateInformation (const void* data, int sizeInBytes);

    AudioPlayHead::CurrentPositionInfo lastPosInfo;
    long int frame = 0;
    int moscPort;
    enum Parameters
    {
        ZirkOSC_Azim_Param,
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

    float gain, azimuth, elevation, azimuth_delta, elevation_delta, azimuth_span, elevation_span;
    void sendOSCValues();


private:
    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ZirkOscjuceAudioProcessor)
    lo_address mOsc;
    TextButton* button1;
    Slider* slider1;
    Label* label;

};

#endif  // __PLUGINPROCESSOR_H_F70DA35D__
