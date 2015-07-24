/*
 ==============================================================================
 ZirkOSC2: VST and AU audio plug-in enabling spatial movement of sound sources in a dome of speakers.
 
 Copyright (C) 2015  GRIS-UdeM
 
 Trajectories.cpp
 Created: 3 Aug 2014 11:42:38am
 
 Developers: Antoine Missout, Vincent Berthiaume
 
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


#include "Trajectories.h"
#include "PluginProcessor.h"
#include "PluginEditor.h"
#include "ZirkConstants.h"



using namespace std;

// ==============================================================================
void Trajectory::start()
{
	spInit();
	mStarted = true;
    
    AudioPlayHead::CurrentPositionInfo cpi;
    ourProcessor->getPlayHead()->getCurrentPosition(cpi);
    
    m_dTrajectoryTimeDone = .0;
    m_dTrajectoryBeginTime = .0;
    
    if (m_bIsSyncWTempo) {
        //convert measure count to a duration
        double dMesureLength = cpi.timeSigNumerator * (4 / cpi.timeSigDenominator) *  60 / cpi.bpm;
        m_dTrajectoriesDurationBuffer = m_TotalTrajectoriesDuration * dMesureLength;
    } else {
        m_dTrajectoriesDurationBuffer = m_TotalTrajectoriesDuration;
    }
    
    m_dTrajectoriesDurationBuffer *= m_dTrajectoryCount;
    m_dTrajectorySingleLength = m_dTrajectoriesDurationBuffer / m_dTrajectoryCount;
    
    //get automation started on currently selected source
    m_iSelectedSourceForTrajectory = ourProcessor->getSelectedSource();
    
    //store initial parameter value

    //need to convert this to azim and elev
    m_fTrajectoryInitialX = ourProcessor->getParameter(ZirkOscAudioProcessor::ZirkOSC_X_ParamId + m_iSelectedSourceForTrajectory*5);
    m_fTrajectoryInitialX = m_fTrajectoryInitialX*2*ZirkOscAudioProcessor::s_iDomeRadius - ZirkOscAudioProcessor::s_iDomeRadius;
    m_fTrajectoryInitialY = ourProcessor->getParameter(ZirkOscAudioProcessor::ZirkOSC_Y_ParamId + m_iSelectedSourceForTrajectory*5);
    m_fTrajectoryInitialY = m_fTrajectoryInitialY*2*ZirkOscAudioProcessor::s_iDomeRadius - ZirkOscAudioProcessor::s_iDomeRadius;
    
    m_fTrajectoryInitialAzimuth01   = SoundSource::XYtoAzim01(m_fTrajectoryInitialX, m_fTrajectoryInitialY);
    m_fTrajectoryInitialElevation01 = SoundSource::XYtoElev01(m_fTrajectoryInitialX, m_fTrajectoryInitialY);
    ourProcessor->storeCurrentLocations();
    
    //convert current elevation as a radian offset for trajectories that use sin/cos
    //    _TrajectoriesPhiAsin = asin(m_fTrajectoryInitialElevation01);
    //    _TrajectoriesPhiAcos = acos(m_fTrajectoryInitialElevation01);
    
    ourProcessor->setIsRecordingAutomation(true);
    ourProcessor->beginParameterChangeGesture(ZirkOscAudioProcessor::ZirkOSC_X_ParamId + m_iSelectedSourceForTrajectory*5);
    ourProcessor->beginParameterChangeGesture(ZirkOscAudioProcessor::ZirkOSC_Y_ParamId + m_iSelectedSourceForTrajectory*5);
}

bool Trajectory::process(float seconds, float beats)
{
	if (mStopped) return true;
	if (!mStarted) start();
	if (mDone == m_TotalTrajectoriesDuration)
	{
		spProcess(0, 0);
		stop();
		return true;
	}

	float duration = m_bIsSyncWTempo ? beats : seconds;
	spProcess(duration, seconds);
	
	mDone += duration;
	if (mDone > m_TotalTrajectoriesDuration)
		mDone = m_TotalTrajectoriesDuration;
	
	return false;
}

float Trajectory::progress()
{
	return mDone / m_TotalTrajectoriesDuration;
}

void Trajectory::stop()
{
	if (!mStarted || mStopped) return;
	mStopped = true;

    ourProcessor->endParameterChangeGesture(ZirkOscAudioProcessor::ZirkOSC_X_ParamId + (m_iSelectedSourceForTrajectory*5));
    ourProcessor->endParameterChangeGesture(ZirkOscAudioProcessor::ZirkOSC_Y_ParamId + (m_iSelectedSourceForTrajectory*5));
    ourProcessor->setIsRecordingAutomation(false);
    
    //reset everything
    ourProcessor->restoreCurrentLocations();
    m_dTrajectoryTimeDone = .0;
    m_bIsWriteTrajectory = false;
    
    ourProcessor->askForGuiRefresh();
}

Trajectory::Trajectory(ZirkOscAudioProcessor *filter, float duration, bool syncWTempo, float times, int source)
:
	ourProcessor(filter),
	mStarted(false),
	mStopped(false),
	mDone(0),
	mDurationSingleTrajectory(duration),
    m_dTrajectoryCount(times),
	m_bIsSyncWTempo(syncWTempo)
{
	if (mDurationSingleTrajectory < 0.0001) mDurationSingleTrajectory = 0.0001;
	if (m_dTrajectoryCount < 0.0001) m_dTrajectoryCount = 0.0001;
    
	m_TotalTrajectoriesDuration = mDurationSingleTrajectory * m_dTrajectoryCount;
}

void Trajectory::move (const float &p_fNewAzimuth, const float &p_fNewElevation){
    float fX, fY;
    SoundSource::azimElev01toXY(p_fNewAzimuth, p_fNewElevation, fX, fY);
    ourProcessor->move(m_iSelectedSourceForTrajectory, fX, fY);
}

void Trajectory::moveXY (const float &p_fNewX, const float &p_fNewY){
    ourProcessor->move(m_iSelectedSourceForTrajectory, p_fNewX, p_fNewY);
}


// ==============================================================================
class CircleTrajectory : public Trajectory
{
public:
	CircleTrajectory(ZirkOscAudioProcessor *filter, float duration, bool beats, float times, int source, bool ccw)
	: Trajectory(filter, duration, beats, times, source), mCCW(ccw) {}
	
protected:
	void spProcess(float duration, float seconds)
	{
        float newAzimuth;
        float integralPart; //useless here
        
        newAzimuth = mDone / mDurationSingleTrajectory; //modf((m_dTrajectoryTimeDone - m_dTrajectoryBeginTime) / m_dTrajectorySingleLength, &integralPart);
        
        if (!mCCW) newAzimuth = - newAzimuth;
        newAzimuth = modf(m_fTrajectoryInitialAzimuth01 + newAzimuth, &integralPart);
        
        move(newAzimuth, m_fTrajectoryInitialElevation01);
	}
	
private:
	bool mCCW;
};

// ==============================================================================
//class PendulumTrajectory : public Trajectory
//{
//public:
//    PendulumTrajectory(ZirkOscAudioProcessor *filter, float duration, bool beats, float times, int source, bool in, bool rt, bool cross, const std::pair<int, int> &endPoint)
//	: Trajectory(filter, duration, beats, times, source)
//    , mIn(in)
//    , mRT(rt)
//    , mCross(cross)
//    , m_fEndPair(endPoint)
//    {
//        oldElevationBuffer = -1.f;
//        
//        if (mIn)    m_bTrajectoryElevationDecreasing = false;
//        else        m_bTrajectoryElevationDecreasing = true;
//    }
//	
//protected:
//	void spInit()
//	{
////		for (int i = 0; i < mFilter->getNumberOfSources(); i++)
////			mSourcesInitRT.add(mFilter->getSourceRT(i));
//	}
//	void spProcess(float duration, float seconds)
//	{
//        int multiple = mCross ? 2 : 1;
//        float newElevation = mDone / mDurationSingleTrajectory, integralPart; //this just grows linearly with time from 0 to m_dTrajectoryCount
//        
//        if (mRT){
//            if (mIn){
//                newElevation = abs( sin(newElevation * multiple * M_PI) );  //varies between [0,1]
//                newElevation = newElevation * (1-m_fTrajectoryInitialElevation01) + m_fTrajectoryInitialElevation01; //scale [0,1] to [m_fTrajectoryInitialElevation01, 1]
//
//                if (mCross && oldElevationBuffer != -1.f){
//                    //when we get to the top of the dome, we need to get back down
//                    if (!m_bTrajectoryElevationDecreasing && newElevation < oldElevationBuffer){
//                        (m_fTrajectoryInitialAzimuth01 > 0.5) ? m_fTrajectoryInitialAzimuth01 -= .5 : m_fTrajectoryInitialAzimuth01 += .5;
//                        m_bTrajectoryElevationDecreasing = true;
//                    }
//                    //reset flag m_bTrajectoryElevationDecreasing as we go down
//                    if (m_bTrajectoryElevationDecreasing && newElevation > oldElevationBuffer){
//                        m_bTrajectoryElevationDecreasing = false;
//                    }
//                }
//                
//            } else {
//                //new way, simply oscilates between m_fTrajectoryInitialElevation01 and 0
//                newElevation = abs( m_fTrajectoryInitialElevation01 * cos(newElevation * M_PI /*+ _TrajectoriesPhiAcos*/) );  //only positive cos wave with phase _TrajectoriesPhi
//            }
//            
//        } else {
//            
//            newElevation = modf(newElevation, &integralPart); //should be [0,1]
//            if (mIn){
//                
//                if (mCross){
//                    newElevation = abs( sin(newElevation * M_PI) );  //varies between [0,1]
//                    newElevation = newElevation * (1 - m_fTrajectoryInitialElevation01) + m_fTrajectoryInitialElevation01;
//                    if (oldElevationBuffer != -1.f){
//                        
//                        //when we get to the top of the dome, we need to get back down
//                        if (!m_bTrajectoryElevationDecreasing && newElevation < oldElevationBuffer){
//                            m_fTrajectoryInitialAzimuth01 > 0.5 ? m_fTrajectoryInitialAzimuth01 -= .5 : m_fTrajectoryInitialAzimuth01 += .5;
//                            m_bTrajectoryElevationDecreasing = true;
//                        }
//                        //reset flag m_bTrajectoryElevationDecreasing as we go down
//                        if (m_bTrajectoryElevationDecreasing && newElevation > oldElevationBuffer){
//                            m_fTrajectoryInitialAzimuth01 > 0.5 ? m_fTrajectoryInitialAzimuth01 -= .5 : m_fTrajectoryInitialAzimuth01 += .5;
//                            m_bTrajectoryElevationDecreasing = false;
//                        }
//                    }
//                } else {
//                    newElevation = newElevation * (1 - m_fTrajectoryInitialElevation01) + m_fTrajectoryInitialElevation01;
//                }
//                
//                
//            } else {
//                newElevation = m_fTrajectoryInitialElevation01*(1-newElevation);
//            }
//        }
//        
//        oldElevationBuffer = newElevation;
//        
//        move (m_fTrajectoryInitialAzimuth01, newElevation);
//	}
//	
//private:
//	bool mIn, mRT, mCross;
//    float oldElevationBuffer;
//    bool m_bTrajectoryElevationDecreasing;
//    std::pair<int, int> m_fEndPair;
//};

class PendulumTrajectory : public Trajectory
{
public:
    PendulumTrajectory(ZirkOscAudioProcessor *filter, float duration, bool beats, float times, int source, bool in, bool rt, bool cross, const std::pair<float, float> &endPoint)
    : Trajectory(filter, duration, beats, times, source)
    , mIn(in)
    , mRT(rt)
    , mCross(cross)
    , m_fEndPair(endPoint)
    {
        oldElevationBuffer = -1.f;
        
        if (mIn)    m_bTrajectoryElevationDecreasing = false;
        else        m_bTrajectoryElevationDecreasing = true;
    }
    
protected:
    void spInit()
    {
        //		for (int i = 0; i < mFilter->getNumberOfSources(); i++)
        //			mSourcesInitRT.add(mFilter->getSourceRT(i));
    }
    void spProcess(float duration, float seconds)
    {
        int multiple = mCross ? 2 : 1;
        float newElevation = mDone / mDurationSingleTrajectory, integralPart; //this just grows linearly with time from 0 to m_dTrajectoryCount
        
        if (mRT){

                //new way, simply oscilates between m_fTrajectoryInitialElevation01 and 0
                newElevation = abs( m_fTrajectoryInitialElevation01 * cos(newElevation * M_PI /*+ _TrajectoriesPhiAcos*/) );  //only positive cos wave with phase _TrajectoriesPhi
            
        } else {
            
            newElevation = modf(newElevation, &integralPart); //should be [0,1]
            if (mIn){
                
                if (mCross){
                    newElevation = abs( sin(newElevation * M_PI) );  //varies between [0,1]
                    newElevation = newElevation * (1 - m_fTrajectoryInitialElevation01) + m_fTrajectoryInitialElevation01;
                    if (oldElevationBuffer != -1.f){
                        
                        //when we get to the top of the dome, we need to get back down
                        if (!m_bTrajectoryElevationDecreasing && newElevation < oldElevationBuffer){
                            m_fTrajectoryInitialAzimuth01 > 0.5 ? m_fTrajectoryInitialAzimuth01 -= .5 : m_fTrajectoryInitialAzimuth01 += .5;
                            m_bTrajectoryElevationDecreasing = true;
                        }
                        //reset flag m_bTrajectoryElevationDecreasing as we go down
                        if (m_bTrajectoryElevationDecreasing && newElevation > oldElevationBuffer){
                            m_fTrajectoryInitialAzimuth01 > 0.5 ? m_fTrajectoryInitialAzimuth01 -= .5 : m_fTrajectoryInitialAzimuth01 += .5;
                            m_bTrajectoryElevationDecreasing = false;
                        }
                    }
                } else {
                    newElevation = newElevation * (1 - m_fTrajectoryInitialElevation01) + m_fTrajectoryInitialElevation01;
                }
                
                
            } else {
                newElevation = m_fTrajectoryInitialElevation01*(1-newElevation);
            }
        }
        
        oldElevationBuffer = newElevation;
        
        move (m_fTrajectoryInitialAzimuth01, newElevation);
    }
    
private:
    bool mIn, mRT, mCross;
    float oldElevationBuffer;
    bool m_bTrajectoryElevationDecreasing;
    std::pair<float, float> m_fEndPair;
};

// ==============================================================================
class SpiralTrajectory : public Trajectory
{
public:
    SpiralTrajectory(ZirkOscAudioProcessor *filter, float duration, bool beats, float times, int source, bool ccw, bool in, bool rt, const std::pair<int, int> &endPoint)
    : Trajectory(filter, duration, beats, times, source), mCCW(ccw), mIn(in), mRT(rt) {}
    
protected:
    void spInit()
    {
        //		for (int i = 0; i < mFilter->getNumberOfSources(); i++)
        //			mSourcesInitRT.add(mFilter->getSourceRT(i));
    }
    void spProcess(float duration, float seconds)
    {
        float newAzimuth, newElevation, theta;
        float integralPart; //useless here
        
        newElevation = mDone / mDurationSingleTrajectory;
        theta = modf(newElevation, &integralPart);                                          //result from this modf is theta [0,1]
        
        //UP AND DOWN SPIRAL
        if (mRT){
            if (mIn){
                newElevation = abs( (1 - m_fTrajectoryInitialElevation01) * sin(newElevation * M_PI) ) + m_fTrajectoryInitialElevation01;
            } else {
                newElevation = abs( m_fTrajectoryInitialElevation01 * cos(newElevation * M_PI) );  //only positive cos wave with phase _TrajectoriesPhi
            }
            
            if (!mCCW) theta = -theta;
            theta *= 2;
            
        } else {
            
            //***** kinda like archimedian spiral r = a + b * theta , but azimuth does not reset at the top
            newElevation = theta * (1 - m_fTrajectoryInitialElevation01) + m_fTrajectoryInitialElevation01;         //newElevation is a mapping of theta[0,1] to [m_fTrajectoryInitialElevation01, 1]
            
            if (!mIn)
                newElevation = m_fTrajectoryInitialElevation01 * (1 - newElevation) / (1-m_fTrajectoryInitialElevation01);  //map newElevation from [m_fTrajectoryInitialElevation01, 1] to [m_fTrajectoryInitialElevation01, 0]
            
            if (!mCCW) theta = -theta;
        }
        newAzimuth = modf(m_fTrajectoryInitialAzimuth01 + theta, &integralPart);                        //this is like adding a to theta
        move(newAzimuth, newElevation);
    }
    
private:
    //	Array<FPoint> mSourcesInitRT;
    bool mCCW, mIn, mRT;
};


// ==============================================================================
class EllipseTrajectory : public Trajectory
{
public:
	EllipseTrajectory(ZirkOscAudioProcessor *filter, float duration, bool beats, float times, int source, bool ccw)
	: Trajectory(filter, duration, beats, times, source), mCCW(ccw) {}
	
protected:
	void spInit()
	{
//		for (int i = 0; i < mFilter->getNumberOfSources(); i++)
//			mSourcesInitRT.add(mFilter->getSourceRT(i));
	}
    void spProcess(float duration, float seconds)
    {
        
        float integralPart; //useless here
        float theta = mDone / mDurationSingleTrajectory;   //goes from 0 to m_dTrajectoryCount
        theta = modf(theta, &integralPart); //does 0 -> 1 for m_dTrajectoryCount times
        if (!mCCW) theta = -theta;
        
        float newAzimuth = m_fTrajectoryInitialAzimuth01 + theta;
        
        float newElevation = m_fTrajectoryInitialElevation01 + (1-m_fTrajectoryInitialElevation01)/2 * abs(sin(theta * 2 * M_PI));
        
        move(newAzimuth, newElevation);
    }
	
private:
//	Array<FPoint> mSourcesInitRT;
	bool mCCW;
};

// ==============================================================================
// Mersenne Twister random number generator, this is now included in c++11, see here: http://en.cppreference.com/w/cpp/numeric/random
class MTRand_int32
{
public:
	MTRand_int32()
	{
		seed(rand());
	}
	uint32_t rand_uint32()
	{
		if (p == n) gen_state();
		unsigned long x = state[p++];
		x ^= (x >> 11);
		x ^= (x << 7) & 0x9D2C5680;
		x ^= (x << 15) & 0xEFC60000;
		return x ^ (x >> 18);
	}
	void seed(uint32_t s)
	{
		state[0] = s;
		for (int i = 1; i < n; ++i)
			state[i] = 1812433253 * (state[i - 1] ^ (state[i - 1] >> 30)) + i;

		p = n; // force gen_state() to be called for next random number
	}

private:
	static const int n = 624, m = 397;
	int p;
	unsigned long state[n];
	unsigned long twiddle(uint32_t u, uint32_t v)
	{
		return (((u & 0x80000000) | (v & 0x7FFFFFFF)) >> 1) ^ ((v & 1) * 0x9908B0DF);
	}
	void gen_state()
	{
		for (int i = 0; i < (n - m); ++i)
			state[i] = state[i + m] ^ twiddle(state[i], state[i + 1]);
		for (int i = n - m; i < (n - 1); ++i)
			state[i] = state[i + m - n] ^ twiddle(state[i], state[i + 1]);
		state[n - 1] = state[m - 1] ^ twiddle(state[n - 1], state[0]);
		
		p = 0; // reset position
	}
	// make copy constructor and assignment operator unavailable, they don't make sense
	MTRand_int32(const MTRand_int32&);
	void operator=(const MTRand_int32&);
};


class RandomTrajectory : public Trajectory
{
public:
	RandomTrajectory(ZirkOscAudioProcessor *filter, float duration, bool beats, float times, int source, float speed)
	: Trajectory(filter, duration, beats, times, source), mClock(0), mSpeed(speed) {}
	
protected:
//	void spProcess(float duration, float seconds)
//	{
//        mClock += seconds;
//        while(mClock > 0.01) {
//            float fAzimuth, fElevation;
//            mClock -= 0.01;
//            if (ZirkOscAudioProcessor::s_bUseXY){
//
//                float fX = ourProcessor->getParameter(ZirkOscAudioProcessor::ZirkOSC_X_ParamId + m_iSelectedSourceForTrajectory*5);
//                float fY = ourProcessor->getParameter(ZirkOscAudioProcessor::ZirkOSC_Y_ParamId + m_iSelectedSourceForTrajectory*5);
//                
//                SoundSource::XY01toAzimElev01(fX, fY, fAzimuth, fElevation);
//                
//            } else {
//                fAzimuth  = ourProcessor->getParameter(ZirkOscAudioProcessor::ZirkOSC_X_ParamId + m_iSelectedSourceForTrajectory*5);
//                fElevation= ourProcessor->getParameter(ZirkOscAudioProcessor::ZirkOSC_Y_ParamId + m_iSelectedSourceForTrajectory*5);
//            }
//            float r1 = mRNG.rand_uint32() / (float)0xFFFFFFFF;
//            float r2 = mRNG.rand_uint32() / (float)0xFFFFFFFF;
//            fAzimuth += (r1 - 0.5) * mSpeed;
//            fElevation += (r2 - 0.5) * mSpeed;
//            move(fAzimuth, fElevation);
//        }
//	}

    void spProcess(float duration, float seconds)
    {
        mClock += seconds;
        while(mClock > 0.01) {

            mClock -= 0.01;
            
            float r1 = mRNG.rand_uint32() / (float)0xFFFFFFFF;
            float r2 = mRNG.rand_uint32() / (float)0xFFFFFFFF;

                
                float fX = ourProcessor->getParameter(ZirkOscAudioProcessor::ZirkOSC_X_ParamId + m_iSelectedSourceForTrajectory*5);
                float fY = ourProcessor->getParameter(ZirkOscAudioProcessor::ZirkOSC_Y_ParamId + m_iSelectedSourceForTrajectory*5);
                
                fX += (r1 - 0.5) * mSpeed;
                fY += (r2 - 0.5) * mSpeed;
                
                fX = PercentToHR(fX, -ZirkOscAudioProcessor::s_iDomeRadius, ZirkOscAudioProcessor::s_iDomeRadius);
                fY = PercentToHR(fY, -ZirkOscAudioProcessor::s_iDomeRadius, ZirkOscAudioProcessor::s_iDomeRadius);
                
                JUCE_COMPILER_WARNING("there has to be a way to have the random values be in the correct range without clamping...")
                fX = clamp(fX, static_cast<float>(-ZirkOscAudioProcessor::s_iDomeRadius), static_cast<float>(ZirkOscAudioProcessor::s_iDomeRadius));
                fY = clamp(fY, static_cast<float>(-ZirkOscAudioProcessor::s_iDomeRadius), static_cast<float>(ZirkOscAudioProcessor::s_iDomeRadius));
                
            move(fX, fY);


        }
    }
private:
	MTRand_int32 mRNG;
	float mClock;
	float mSpeed;
};

// ==============================================================================
class TargetTrajectory : public Trajectory
{
public:
	TargetTrajectory(ZirkOscAudioProcessor *filter, float duration, bool beats, float times, int source)
	: Trajectory(filter, duration, beats, times, source), mCycle(-1) {}
	
protected:
//	virtual FPoint destinationForSource(int s, FPoint o) = 0;

	void spProcess(float duration, float seconds)
	{
//		float p = mDone / mDurationSingleTrajectory;
//		
//		int cycle = (int)p;
//		if (mCycle != cycle)
//		{
//			mCycle = cycle;
//			mSourcesOrigins.clearQuick();
//			mSourcesDestinations.clearQuick();
//			
//			for (int i = 0; i < mFilter->getNumberOfSources(); i++)
//			if (mSource < 0 || mSource == i)
//			{
//				FPoint o = mFilter->getSourceXY(i);
//				mSourcesOrigins.add(o);
//				mSourcesDestinations.add(destinationForSource(i, o));
//			}
//		}
//
//		float d = fmodf(p, 1);
//		for (int i = 0; i < mFilter->getNumberOfSources(); i++)
//		if (mSource < 0 || mSource == i)
//		{
//			FPoint a = mSourcesOrigins.getUnchecked(i);
//			FPoint b = mSourcesDestinations.getUnchecked(i);
//			FPoint p = a + (b - a) * d;
//			mFilter->setSourceXY(i, p);
//		}
	}
	
private:
//	Array<FPoint> mSourcesOrigins;
//	Array<FPoint> mSourcesDestinations;
	int mCycle;
};


// ==============================================================================
class RandomTargetTrajectory : public TargetTrajectory
{
public:
	RandomTargetTrajectory(ZirkOscAudioProcessor *filter, float duration, bool beats, float times, int source)
	: TargetTrajectory(filter, duration, beats, times, source) {}
	
protected:
//	FPoint destinationForSource(int s, FPoint o)
//	{
//		float r1 = mRNG.rand_uint32() / (float)0xFFFFFFFF;
//		float r2 = mRNG.rand_uint32() / (float)0xFFFFFFFF;
//		float x = r1 * (kRadiusMax*2) - kRadiusMax;
//		float y = r2 * (kRadiusMax*2) - kRadiusMax;
//		float r = hypotf(x, y);
//		if (r > kRadiusMax)
//		{
//			float c = kRadiusMax/r;
//			x *= c;
//			y *= c;
//		}
//		return FPoint(x,y);
//	}
	
private:
	MTRand_int32 mRNG;
};

// ==============================================================================
class SymXTargetTrajectory : public TargetTrajectory
{
public:
	SymXTargetTrajectory(ZirkOscAudioProcessor *filter, float duration, bool beats, float times, int source)
	: TargetTrajectory(filter, duration, beats, times, source) {}
	
protected:
//	FPoint destinationForSource(int s, FPoint o)
//	{
//		return FPoint(o.x,-o.y);
//	}
};

// ==============================================================================
class SymYTargetTrajectory : public TargetTrajectory
{
public:
	SymYTargetTrajectory(ZirkOscAudioProcessor *filter, float duration, bool beats, float times, int source)
	: TargetTrajectory(filter, duration, beats, times, source) {}
	
protected:
//	FPoint destinationForSource(int s, FPoint o)
//	{
//		return FPoint(-o.x,o.y);
//	}
};

// ==============================================================================
class ClosestSpeakerTargetTrajectory : public TargetTrajectory
{
public:
	ClosestSpeakerTargetTrajectory(ZirkOscAudioProcessor *filter, float duration, bool beats, float times, int source)
	: TargetTrajectory(filter, duration, beats, times, source) {}
	
protected:
//	FPoint destinationForSource(int s, FPoint o)
//	{
//		int bestSpeaker = 0;
//		float bestDistance = o.getDistanceFrom(mFilter->getSpeakerXY(0));
//		
//		for (int i = 1; i < mFilter->getNumberOfSpeakers(); i++)
//		{
//			float d = o.getDistanceFrom(mFilter->getSpeakerXY(i));
//			if (d < bestDistance)
//			{
//				bestSpeaker = i;
//				bestDistance = d;
//			}
//		}
//		
//		return mFilter->getSpeakerXY(bestSpeaker);
//	}
};

// ==============================================================================
int Trajectory::NumberOfTrajectories() { return TotalNumberTrajectories-1; }

String Trajectory::GetTrajectoryName(int i)
{
	switch(i)
	{
        case Circle: return "Circle";
        case Ellipse: return "Ellipse";
        case Spiral: return "Spiral";
        case Pendulum: return "Pendulum";
        case AllTrajectoryTypes::Random: return "Random";
	}
	jassert(0);
	return "";
}

std::unique_ptr<vector<String>> Trajectory::getTrajectoryPossibleDirections(int p_iTrajectory){
    unique_ptr<vector<String>> vDirections (new vector<String>);
    
    switch(p_iTrajectory) {
        case Circle:
        case Ellipse:
            vDirections->push_back("Clockwise");
            vDirections->push_back("Counter Clockwise");
            break;
        case Spiral:
            vDirections->push_back("In, Clockwise");
            vDirections->push_back("In, Counter Clockwise");
            vDirections->push_back("Out, Clockwise");
            vDirections->push_back("Out, Counter Clockwise");
            break;
        case Pendulum:
            vDirections->push_back("In");
            vDirections->push_back("Out");
            vDirections->push_back("Crossover");
            break;
        case AllTrajectoryTypes::Random:
            vDirections->push_back("Slow");
            vDirections->push_back("Mid");
            vDirections->push_back("Fast");
            break;
        default:
            jassert(0);
    }

    return vDirections;
}

unique_ptr<AllTrajectoryDirections> Trajectory::getTrajectoryDirection(int p_iSelectedTrajectory, int p_iSelectedDirection){
    
    unique_ptr<AllTrajectoryDirections> pDirection (new AllTrajectoryDirections);
    
    switch (p_iSelectedTrajectory) {

        case Circle:
        case Ellipse:
            *pDirection = static_cast<AllTrajectoryDirections>(p_iSelectedDirection);
            break;
        case Spiral:
            *pDirection = static_cast<AllTrajectoryDirections>(p_iSelectedDirection+5);
            break;
        case Pendulum:
            *pDirection = static_cast<AllTrajectoryDirections>(p_iSelectedDirection+2);
            break;
        case AllTrajectoryTypes::Random:
            *pDirection = static_cast<AllTrajectoryDirections>(p_iSelectedDirection+9);
            break;
            
        default:
            break;
    }
    
    return pDirection;
}

std::unique_ptr<vector<String>> Trajectory::getTrajectoryPossibleReturns(int p_iTrajectory){

    unique_ptr<vector<String>> vReturns (new vector<String>);

    switch(p_iTrajectory) {
        case Circle:
        case Ellipse:
        case AllTrajectoryTypes::Random:
            return nullptr;
        case Spiral:
        case Pendulum:
            vReturns->push_back("One Way");
            vReturns->push_back("Return");
            break;
        default:
            jassert(0);
    }
    
    return vReturns;
}


Trajectory::Ptr Trajectory::CreateTrajectory(int type, ZirkOscAudioProcessor *filter, float duration, bool beats, AllTrajectoryDirections direction,
                                             bool bReturn, float times, int source, const std::pair<float, float> &endPair)
{
    
    bool ccw, in, cross;
    float speed;
    
    switch (direction) {
        case CW:
            ccw = false;
            break;
        case CCW:
            ccw = true;
            break;
        case In:
            in = true;
            cross = false;
            break;
        case Out:
            in = false;
            cross = false;
            break;
        case Crossover:
            in = true;
            cross = true;
            break;
        case InCW:
            in = true;
            ccw = false;
            break;
        case InCCW:
            in = true;
            ccw = true;
            break;
        case OutCW:
            in = false;
            ccw = false;
            break;
        case OutCCW:
            in = false;
            ccw = true;
            break;
        case Slow:
            speed = .02;
            break;
        case Mid:
            speed = .06;
            break;
        case Fast:
            speed = .1;
            break;
        default:
            break;
    }
    
    
    switch(type)
    {
        case Circle:                     return new CircleTrajectory(filter, duration, beats, times, source, ccw);
        case Ellipse:                    return new EllipseTrajectory(filter, duration, beats, times, source, ccw);
        case Spiral:                     return new SpiralTrajectory(filter, duration, beats, times, source, ccw, in, bReturn, endPair);
        case Pendulum:                   return new PendulumTrajectory(filter, duration, beats, times, source, in, bReturn, cross, endPair);
        case AllTrajectoryTypes::Random: return new RandomTrajectory(filter, duration, beats, times, source, speed);
            
            //      case 19: return new RandomTargetTrajectory(filter, duration, beats, times, source);
            //		case 20: return new SymXTargetTrajectory(filter, duration, beats, times, source);
            //		case 21: return new SymYTargetTrajectory(filter, duration, beats, times, source);
            //		case 22: return new ClosestSpeakerTargetTrajectory(filter, duration, beats, times, source);
    }
    jassert(0);
    return NULL;
}



