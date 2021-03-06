//
//  Timer.h
//  ProjectRetro
//
//  Created by Alex Koukoulas on 29/03/2019.
//


////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

#ifndef Timer_h
#define Timer_h

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

#include <functional> 

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

class Timer final
{
public:    
    Timer(const float period)
        : mPeriod(period)
        , mTimeCounter(mPeriod)
        , mHasTicked(false)
        , mIsRunning(true)
    
    {
    }

    inline void Update(const float dt)
    {
        if (!mIsRunning)
        {
            return;
        }
        
        mTimeCounter -= dt;
        if (mTimeCounter <= 0.0f)
        {
            mHasTicked = true;
        }
    }
    
    inline float GetTimerValue() const
    {
        return mTimeCounter;
    }

    inline bool HasTicked() const
    {
        return mHasTicked;
    }

    inline void SetTimerValue(const float value)
    {
        mTimeCounter = value;
    }
    
    inline void Pause()
    {
        mIsRunning = false;
    }
    
    inline void Resume()
    {
        mIsRunning = true;
    }
    
    inline void Reset()
    {
        mTimeCounter = mPeriod;
        mHasTicked   = false;
    }
    
private:
    const float mPeriod;
    float       mTimeCounter;
    bool        mHasTicked;
    bool        mIsRunning;    
};

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

#endif /* Timer_h */
