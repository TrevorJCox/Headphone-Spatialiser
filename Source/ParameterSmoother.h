/*
  ==============================================================================

    Smoother.h
    Author:  Oliver Larkin

  ==============================================================================
*/

#ifndef SMOOTHER_H_INCLUDED
#define SMOOTHER_H_INCLUDED

class ParameterSmoother
{
private:
  float mA, mB;
  float mOutM1;
  
public:
  ParameterSmoother(float timeMS = 5.f, float sr = 48000.f, float initalValue = 0.f)
  : mOutM1(initalValue)
  {
    setTimeMS(timeMS, sr);
  }
  
  void setTimeMS(float timeMS, float sr)
  {
    mA = exp(-1.f/((timeMS/1000.f)*sr));
    mB = 1.f - mA;
  }
  
  inline float process(float input)
  {
    mOutM1 = (input * mB) + (mOutM1 * mA);
    return mOutM1;
  }
};

#endif  // SMOOTHER_H_INCLUDED
