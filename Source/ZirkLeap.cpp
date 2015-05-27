/*
 * ==============================================================================
 *
 *  OctoLeap.cpp
 *  Created: 4 Aug 2014 1:23:01pm
 *  Authors:  makira & Antoine L.
 *
 * ==============================================================================
 */

#include <iostream>
#include "ZirkLeap.h"
#include "ZirkConstants.h"




#if WIN32
Component * CreateLeapComponent(OctogrisAudioProcessor *filter, OctogrisAudioProcessorEditor *editor)
{
    /** not implemented yet on windows*/
    return NULL;
}
#else

#include "Leap.h"

/** ZirkLeap constructor taking two arguments and initializing its others components by default */

ZirkLeap::ZirkLeap(ZirkOscjuceAudioProcessor *filter, ZirkOscjuceAudioProcessorEditor *editor):
mFilter(filter),
mEditor(editor),
mController(NULL),
mPointableId(-1),
mLastPositionValid(0)
{
    
    
}
/** onConnect is called when Leap is connected this method is only changing the interface because the Leap Controller is handling everything */
void ZirkLeap::onConnect(const Leap::Controller& controller)
{
    const MessageManagerLock mmLock;
    mEditor->getmStateLeap()->setText("Leap connected", dontSendNotification);
}
/** onServiceDisconnected is only called if the Leap application on the computer has gone very wrong */
void ZirkLeap::onServiceDisconnect(const Leap::Controller& controller )
{
    printf("Service Leap Disconnected");
}
//Not dispatched when running in a debugger

/** onDisconnect is called when Leap is disconnected this method is only changing the interface because the Leap Controller is handling everything */
void ZirkLeap::onDisconnect(const Leap::Controller& controller)
{
    const MessageManagerLock mmLock;
    mEditor->getmStateLeap()->setText("Leap disconnected", dontSendNotification);
}

/** onFrame is called for each frame that the Leap Motion capture even if nothing is detected. If a hand is detected
 then it process the coord and move the selected source */

void ZirkLeap::onFrame(const Leap::Controller& controller)
{
    if(controller.hasFocus())
    {
        
        Leap::Frame frame = controller.frame();
        if (mPointableId >= 0)
        {
            mFilter->setSelectedSource(mEditor->getCBSelectedSource()-1);
            Leap::Pointable p = frame.pointable(mPointableId);
            if (!p.isValid() || !p.isExtended())
            {
                mPointableId = -1;
                mLastPositionValid = false;
            }
            else
            {
                Leap::Vector pos = p.tipPosition();
                const float zPlane1 = 50;	// 5 cm
                const float zPlane2 = 100;	// 10 cm
                
                if (pos.z < zPlane2)
                {
                    if (mLastPositionValid)
                    {
                        //Leap Motion mouvement are calculated from the last position in order to have something dynamic and ergonomic
                        Leap::Vector delta = pos- mLastPosition;
                        
                        float scale = 3;
                        if (pos.z > zPlane1)
                        {
                            float s = 1 - (pos.z - zPlane1) / (zPlane2 - zPlane1);
                            scale *= s;
                            
                        }
                        
                        int src = mEditor->getOscLeapSource()-1;
                        Point<float> sp = mFilter->getSources()[src].getPositionXY();
                        sp.x += delta.x * scale;
                        sp.y -= delta.y * scale;
                        int selectedConstraint = mFilter->getSelectedMovementConstraintAsInteger();
                        if(selectedConstraint == Independant)
                        {
                            
                            mFilter->getSources()[src].setPositionXY(sp);
                            mFilter->setParameterNotifyingHost (ZirkOscjuceAudioProcessor::ZirkOSC_Azim_ParamId + src*5, mFilter->getSources()[src].getAzimuth());
                            mFilter->setParameterNotifyingHost (ZirkOscjuceAudioProcessor::ZirkOSC_Elev_ParamId + src*5, mFilter->getSources()[src].getElevation());
                            //send source position by osc
                            mFilter->sendOSCValues();
                            
                        }
                        else if (selectedConstraint == FixedAngles){
                            mEditor->moveFixedAngles(sp);
                        }
                        else if (selectedConstraint == FixedRadius){
                            mEditor->moveCircularWithFixedRadius(sp);
                        }
                        else if (selectedConstraint == FullyFixed){
                            mEditor->moveFullyFixed(sp);
                        }
                        else if (selectedConstraint == DeltaLocked){
                            Point<float> DeltaMove = sp - mFilter->getSources()[src].getPositionXY();
                            mEditor->moveSourcesWithDelta(DeltaMove);
                        }
                        else if (selectedConstraint == Circular){
                            mEditor->moveCircular(sp);
                        }
                        
                        
                        mEditor->fieldChanged();
                    }
                    else
                    {
                        //std::cout << "pointable last pos not valid" << std::endl;
                        
                    }
                    
                    mLastPosition = pos;
                    mLastPositionValid = true;
                }
                else
                {
                    //std::cout << "pointable not touching plane" << std::endl;
                    mLastPositionValid = false;
                    
                }
            }
        }
        if (mPointableId < 0)
        {
            Leap::PointableList pl = frame.pointables().extended();
            if (pl.count() > 0)
            {
                mPointableId = pl[0].id();
                //std::cout << "got new pointable: " << mPointableId << std::endl;
            }
        }
    }
}


/** CreateLeapComponent is called to create a ZirkLeap instance through the ReferenceCountedObject so it is destroyed properly */
ZirkLeap::Ptr ZirkLeap::CreateLeapComponent(ZirkOscjuceAudioProcessor *filter, ZirkOscjuceAudioProcessorEditor *editor)
{
    return new ZirkLeap(filter, editor);
}


#endif
