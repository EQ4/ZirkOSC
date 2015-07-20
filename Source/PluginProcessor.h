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

#ifndef __PLUGINPROCESSOR_H_F70DA35D__
#define __PLUGINPROCESSOR_H_F70DA35D__

#include "../JuceLibraryCode/JuceHeader.h"

//#include "ZirkConstants.h"
#include "lo.h"
#include "SoundSource.h"
#include "Trajectories.h"

/**
 The processor class of the plug in
 */
class ZirkOscjuceAudioProcessor  : public AudioProcessor,public Timer
{
public:
    
    //==============================================================================
    //! Builder
    ZirkOscjuceAudioProcessor();
    //! Destroyer
    ~ZirkOscjuceAudioProcessor();
    
    //! Called every 50ms;
    void timerCallback();
    
    void move(int p_iSource, float p_fX, float p_fY);
    
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
    // We don't have sound so we just return 0.0
    double getTailLengthSeconds() const { return 0.0; }
    
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
    //! The host will call this method when it wants to save the filter's internal state, ie SAVE PRESET.
    void getStateInformation (MemoryBlock& destData);
    //! This must restore the filter's state from a block of data previously created using getStateInformation() IE RESTORE PRESET.
    void setStateInformation (const void* data, int sizeInBytes);
    
    //! returns the sources
    inline SoundSource* getSources(){ return _AllSources; }
    //! returns the number of sources on the screen.
    int getNbrSources() { return _NbrSources; }
    //! Set the number of sources.
    void setNbrSources(int newValue) { if ( newValue >-1 && newValue < 9) _NbrSources= newValue; }
    
    //!set wheter plug is sending osc messages to zirkonium
    void setIsOscActive(bool isOscActive);
    //!wheter plug is sending osc messages to zirkonium
    bool getIsOscActive();

    //!set wheter plug is sending osc messages to zirkonium
    void setIsSpanLinked(bool isSpanLinked);
    //!wheter plug is sending osc messages to zirkonium
    bool getIsSpanLinked();

    //!set wheter programmed trajectories are written in sync with host tempo
    void setIsSyncWTempo(bool isSyncWTempo);
    //!get wheter programmed trajectories are written in sync with host tempo
    bool getIsSyncWTempo();
    
    //!set wheter to write programmed trajectory on next host play
    void setIsWriteTrajectory(bool isWriteTrajectory);
    //!get wheter to write programmed trajectory on next host play
    bool getIsWriteTrajectory();
    
    void setSelectedSourceForTrajectory(int iSelectedSource);
    
    int getSelectedSourceForTrajectory();
    
    //! return the tab position of the selectedSource 
    int getSelectedSource() { return _SelectedSource; }
    //! Set the selected source 
    void setSelectedSource(int selected){ if ( selected >-1 && selected < 8) _SelectedSource= selected;};
    //! Returns the Osc Port for the Zirkonium sending
    int getOscPortZirkonium(){return _OscPortZirkonium;}
    
    enum ParameterIds
    {
        ZirkOSC_X_ParamId = 0,
        ZirkOSC_Y_ParamId,
        ZirkOSC_AzimSpan_ParamId,
        ZirkOSC_ElevSpan_ParamId,
        ZirkOSC_Gain_ParamId,
        ZirkOSC_Azim_ParamId_1,
        ZirkOSC_Elev_ParamId_1,
        ZirkOSC_AzimSpan_ParamId_1,
        ZirkOSC_ElevSpan_ParamId_1,
        ZirkOSC_Gain_ParamId_1,
        ZirkOSC_Azim_ParamId_2,     //10
        ZirkOSC_Elev_ParamId_2,
        ZirkOSC_AzimSpan_ParamId_2,
        ZirkOSC_ElevSpan_ParamId_2,
        ZirkOSC_Gain_ParamId_2,
        ZirkOSC_Azim_ParamId_3,
        ZirkOSC_Elev_ParamId_3,
        ZirkOSC_AzimSpan_ParamId_3,
        ZirkOSC_ElevSpan_ParamId_3,
        ZirkOSC_Gain_ParamId_3,
        ZirkOSC_Azim_ParamId_4,     //20
        ZirkOSC_Elev_ParamId_4,
        ZirkOSC_AzimSpan_ParamId_4,
        ZirkOSC_ElevSpan_ParamId_4,
        ZirkOSC_Gain_ParamId_4,
        ZirkOSC_Azim_ParamId_5,
        ZirkOSC_Elev_ParamId_5,
        ZirkOSC_AzimSpan_ParamId_5,
        ZirkOSC_ElevSpan_ParamId_5,
        ZirkOSC_Gain_ParamId_5,
        ZirkOSC_Azim_ParamId_6,     //30
        ZirkOSC_Elev_ParamId_6,
        ZirkOSC_AzimSpan_ParamId_6,
        ZirkOSC_ElevSpan_ParamId_6,
        ZirkOSC_Gain_ParamId_6,
        ZirkOSC_Azim_ParamId_7,
        ZirkOSC_Elev_ParamId_7,
        ZirkOSC_AzimSpan_ParamId_7,
        ZirkOSC_ElevSpan_ParamId_7,
        ZirkOSC_Gain_ParamId_7,
        ZirkOSC_MovementConstraint_ParamId, //40
        ZirkOSC_isSpanLinked_ParamId,
        ZirkOSC_isOscActive_ParamId,
        ZirkOSC_SelectedTrajectory_ParamId,
        ZirkOSC_SelectedTrajectoryDirection_ParamId,
        ZirkOSC_SelectedTrajectoryReturn_ParamId,
        ZirkOSC_TrajectoryCount_ParamId,
        ZirkOSC_TrajectoriesDuration_ParamId,
        ZirkOSC_SyncWTempo_ParamId,
        ZirkOSC_WriteTrajectories_ParamId,
        totalNumParams                      //50
    };
    
    //! Send the current state to all the iPad and Zirkonium
    void sendOSCValues();
    
    //! Getter constrain type as integer, since parameters need to be stored as floats [0,1]
    int getSelectedMovementConstraint();
    
    //! Getter for trajectory as integer, since parameters need to be stored as floats [0,1]
    int getSelectedTrajectory();
    
    float getSelectedTrajectoryDirection();
    
    float getSelectedTrajectoryReturn();
    
    //! Retunrs true if the Editor has to refresh the Gui.
    bool hasToRefreshGui(){return _RefreshGui;};
    //! If you want to refresh the Gui from the Processor.
    void setRefreshGui(bool gui) { _RefreshGui = gui;};
    //! Change the sending OSC port of the zirkonium
    void changeZirkoniumOSCPort(int newPort);

    //! Returns the Editor.
    AudioProcessorEditor* getEditor() {return _Editor;};
    //! Set the width that the UI was last set to
    void setLastUiWidth(int lastUiWidth);
    //! Get the width that the UI was last set to
    int getLastUiWidth();
    //! Set the height that the UI was last set to
    void setLastUiHeight(int lastUiHeight);
    //! Return the height that the UI was last set to
    int getLastUiHeight();
    
    void storeCurrentLocations();
    
    void restoreCurrentLocations();
    
    //radius of the dome
    static int s_iDomeRadius;
      
    bool isTrajectoryDone();
    
    //NEW TRAJECTORY CLASS METHODS
    void setTrajectory(Trajectory::Ptr t) { mTrajectory = t; }
  	Trajectory::Ptr getTrajectory() { return mTrajectory; }
    
    void askForGuiRefresh(){_RefreshGui=true;};
    
    bool getIsJoystickEnabled() const { return _isJoystickEnabled; }
    
    void setIsJoystickEnabled(int s) { _isJoystickEnabled = s; }
    
    void setFixedAngle(bool fixedAngle);
    
    void setFollowSelectedSource (bool p_bFollowSelectedSource){
        m_bFollowSelectedSource = p_bFollowSelectedSource;
    }

    bool isFixedAngle();
    
    //! get the source order by the angle value
    std::vector<int> getOrderSources(int, SoundSource[], int nbrSources);

    
private:
    
    void initSources();
    
    void processTrajectories();
    
    void stopTrajectory();
    
    void moveCircular(const int &p_iSource, const float &p_fX, const float &p_fY, bool p_bIsElevFixed);

    void moveFixedAngles(const int &p_iSource, const float &p_fX, const float &p_fY);
    
    void moveFullyFixed(const int &p_iSource, const float &p_fX, const float &p_fY);
    
    void moveSourcesWithDelta(const int &p_iSource, const float &p_fX, const float &p_fY);
    
    void orderSourcesByAngle(int begin, SoundSource tab [] );
    
    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ZirkOscjuceAudioProcessor)
    //! Whether the editor has to refresh the GUI
    JUCE_COMPILER_WARNING("this flag should really be used and the gui should NOT refresh if this flag is false")
    bool _RefreshGui = false;
    //! Current number of sources on the screnn
    int _NbrSources;
    //! float ID of the selected movement constraint
    float _SelectedMovementConstraint;
    //! int ID of the selected movement constraint IMPORTANT: need to be set manually whenever float version of parameter is changed
    int m_iSelectedMovementConstraint;
    //! float ID of the selected trajectory
    float _SelectedTrajectory;
    
    float m_fSelectedTrajectoryDirection;
    
    float m_fSelectedTrajectoryReturn;
    
    //! Tab position of the selected source
    int _SelectedSource;
    //! Osc port to send to the Zirkonium 
    int _OscPortZirkonium;
    //! The editor
    AudioProcessorEditor* _Editor;
    //! Sources array
    SoundSource _AllSources [8];
    //Copy of all sources to be able to save and restore locations before and after a trajectory
    SoundSource _AllSourcesBuffer [8];
    
    //! Zirkonium OSC address (sending)
    lo_address _OscZirkonium;

    //! last saved ui width
    int _LastUiWidth;
    //! last saved ui height
    int _LastUiHeight;
    //! Whether we're sending OSC messages to the zirkonium
    bool _isOscActive;
    //! If the span are linked
    bool _isSpanLinked;
    
    PluginHostType host;

    bool _isJoystickEnabled;


    //OLD TRAJECTORIES
    
    //! Number of trajectories to draw in trajectory section
    double _TrajectoryCount;
    
    //! Duration of trajectory movement
    double _TrajectoriesDuration;
    
    //!Whether to sync trajectories with tempo
    bool _isSyncWTempo;
    
    //!Whether to write trajectory or not
    bool _isWriteTrajectory;
    
    int _SelectedSourceForTrajectory;
    
    Trajectory::Ptr mTrajectory;
    
    int64 mLastTimeInSamples;
    
    //the id of the source that was last changed
    int m_iSourceLocationChanged;   
    
    float m_fSourceOldX01[8];
    float m_fSourceOldY01[8];

    bool m_bNeedToSetFixedAngles;
    
    bool m_bFollowSelectedSource;
    
    bool m_bIsRadiusFixed;

};

#endif  // __PLUGINPROCESSOR_H_F70DA35D__
