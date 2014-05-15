//
//  SoundSource.h
//  ZirkOSCJUCE
//
//  Created by Lud's on 26/02/13.
//  Copyright 2013 Ludovic LAFFINEUR ludovic.laffineur@gmail.com
//

#ifndef __ZirkOSCJUCE__SoundSource__
#define __ZirkOSCJUCE__SoundSource__

#include <iostream>
#include "../JuceLibraryCode/JuceHeader.h"
class SoundSource{
public:
    SoundSource();
    SoundSource(float,float);
    ~SoundSource();
    //! returns the XY position in a Point <float>.
    Point<float> getPositionXY();
    //! returns the channel (id in the Zirkonium)
    int     getChannel();
    //! sets the channel (id in the Zirkonium)
    void    setChannel(int);
    //! returns the X position
    float   getX();
    //! retunrs the Y position
    float   getY();
    //! sets the XY position (converts it in azimuth / elevation)
    void    setPositionXY(Point <float>);
    //! returns the gain [0,1]
    float   getGain();
    //! sets the gain 
    void    setGain(float);
    //! gets the Azimuth [0,1]
    float   getAzimuth();
    //! sets the azimuth
    void    setAzimuth(float);
    //! returns the azimuth span
    float   getAzimuthSpan();
    //! sets the azimuth span
    void    setAzimuthSpan(float);
    //! returns the elevation span
    float   getElevationSpan();
    //! sets the elevation span
    void    setElevationSpan(float);
    //! returns the elevation [0,1]
    float   getElevation();
    //! returns the elevation [-1,1] form memory.
    float   getElevationRawValue();
    //! set the elevation 
    void    setElevation(float);
    //! returns true if the point is inside the source. Point is relative to the center of the dome
    bool    contains(Point<float>);
    //! returns true if the azimuth has been reversed (elevation >1)
    bool    isAzimReverse();
    //! set true if the azimuth will be reversed
    void    setAzimReverse(bool);
    //! Check if the movement lets the source in the dome
    bool isStillInTheDome( Point<float> move);

        
private:
    //! If source Elevation is over 90° you have to reverse the azim
    bool _AzimReverse =false;
    //! Source channel id id send to Zirkonium
    int _Channel =0;
    //! Gain parameter stored in percent (see HRToPercent function).
    float _Gain=1;
    //! Azimuth parameter stored in percent (see HRToPercent function).
    float _Azimuth=0;
    //! Elevation parameter stored in percent (see HRToPercent function).
    float _Elevation=0;
    //! Azimuth Span parameter stored in percent (see HRToPercent function).
    float _AzimuthSpan=0;
    //! Elevation Span parameter stored in percent (see HRToPercent function).
    float _ElevationSpan=0;
    //! Convert degreeToRadian
    inline float degreeToRadian (float);
    //! Convert radianToDegree
    inline float radianToDegree (float);
    
};



#endif /* defined(__ZirkOSCJUCE__SoundSource__) */
