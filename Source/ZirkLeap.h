/*
  ==============================================================================

    OctoLeap.h
    Created: 4 Aug 2014 1:23:01pm
    Author:  makira

  ==============================================================================
*/

#ifndef OCTOLEAP_H_INCLUDED
#define OCTOLEAP_H_INCLUDED
#include "Leap.h"


#include "PluginEditor.h"
class ZirkLeap : public ReferenceCountedObject , public Leap::Listener
{
public:
    typedef ReferenceCountedObjectPtr<ZirkLeap> Ptr;
    static ZirkLeap::Ptr CreateLeapComponent(ZirkOscjuceAudioProcessor *filter, ZirkOscjuceAudioProcessorEditor *editor);
    
    ZirkLeap(ZirkOscjuceAudioProcessor *filter, ZirkOscjuceAudioProcessorEditor *editor);
    
    
    virtual void onConnect(const Leap::Controller& controller);
    void onDisconnect(const Leap::Controller& controller);
    void onFrame(const Leap::Controller& controller);
    
    void onServiceDisconnect(const Leap::Controller& controller);
    virtual ~ZirkLeap(){
    }

    
private:
    ZirkOscjuceAudioProcessor *mFilter;
    ZirkOscjuceAudioProcessorEditor *mEditor;
        
    ScopedPointer<Leap::Controller> mController;
    
    int32_t mPointableId;
    bool mLastPositionValid;
    Leap::Vector mLastPosition;
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ZirkLeap)
    
};

void updateLeapComponent(Component * leapComponent);

#endif  // OCTOLEAP_H_INCLUDED
