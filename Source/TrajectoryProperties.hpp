//
//  TrajectoryProperties.hpp
//  ZirkOSC2
//
//  Created by Vincent Berthiaume on 2014-09-01.
//
//

#ifndef ZirkOSC2_TrajectoryProperties_hpp
#define ZirkOSC2_TrajectoryProperties_hpp

#include "../JuceLibraryCode/JuceHeader.h"
//#include "PluginProcessor.h"
class ZirkOscjuceAudioProcessor;

class TrajectoryComboBoxComponent : public ChoicePropertyComponent{
    
public:
    
//    TrajectoryGroupComponent(const Value &valueToControl, const String &propertyName, const StringArray &choices,
//                             const Array< var > &correspondingValues, ZirkOscjuceAudioProcessor* p_pProcessor)
//    :  ChoicePropertyComponent (valueToControl, propertyName, choices, correspondingValues) {
//        ourProcessor = p_pProcessor;
//    }

    TrajectoryComboBoxComponent(const String &propertyName, ZirkOscjuceAudioProcessor* p_pProcessor);
    
    
    void setIndex(int newIndex) override;
    
    int getIndex () const override;
    
    
private:

    ZirkOscjuceAudioProcessor* ourProcessor;
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (TrajectoryComboBoxComponent)
};



class TrajectoryButtonComponent : public BooleanPropertyComponent
{
public:
    
    TrajectoryButtonComponent (const String &propertyName, const String &buttonTextWhenTrue,
                               const String &buttonTextWhenFalse, ZirkOscjuceAudioProcessor* p_pProcessor, bool p_bIsTempo);
    
    void setState (const bool newState) override;
    
    bool getState () const override;
    
private:
    //bool m_bState;
    bool m_bIsTempo;
    ZirkOscjuceAudioProcessor* ourProcessor;
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (TrajectoryButtonComponent)
};



class TrajectoryTextComponent : public TextPropertyComponent
{
public:
    TrajectoryTextComponent (const Value &valueToControl, const String &propertyName, int maxNumChars,
                             bool isMultiLine, ZirkOscjuceAudioProcessor* p_pProcessor, bool p_bIsCount);
    
    void setText (const String &newText) override;
    
    String getText() const override;
    
private:
    //String text;
    bool m_bIsCount;
    ZirkOscjuceAudioProcessor* ourProcessor;
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (TrajectoryTextComponent)
};

#endif