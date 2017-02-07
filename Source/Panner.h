/*
  ==============================================================================

    Panner.h
    Created: 10 May 2016 5:12:20pm
    Author:  Oliver Larkin

  ==============================================================================
*/

#ifndef PANNER_H_INCLUDED
#define PANNER_H_INCLUDED

#include "JuceHeader.h"
#include "ParameterSmoother.h"

class Panner
{
public:
  Panner()
  : mPanPosition(0.)
  {
    for( int i=0; i<256; i++ )
    {
      mSqrtLUT[i] = sqrt( ((double)i/(double)255.));
      mSqrtLUT[511-i] = mSqrtLUT[i];
    }
  }
  
  inline float lerp(const float phase, const float* buffer)
  {
    const int intPart = (int) phase;
    const float fracPart = phase-intPart;
    
    const float a = buffer[intPart];
    const float b = buffer[(intPart+1)];
    
    return a + (b - a) * fracPart;
  }
  
  void prepareToPlay (double sampleRate, int estimatedSamplesPerBlock)
  {
    mPanSmoother.setTimeMS(cSmoothTime, sampleRate);
  }
  
  void setPanPos(float normalizedPosition)
  {
    mPanPosition = jlimit<float>(0.f, 1.f, normalizedPosition) * 0.5f;
  }
  
  void processBlock(AudioBuffer<float> &buffer)
  {
    float *out1 = buffer.getWritePointer(0);
    float *out2 = buffer.getWritePointer(1);

    for (int i = 0; i < buffer.getNumSamples(); i++)
    {
      const float input = out1[i];
      const float smoothedPanPos = mPanSmoother.process(mPanPosition);
      out1[i] = input * lerp((0.5f + smoothedPanPos) * 511.f, mSqrtLUT);
      out2[i] = input * lerp(smoothedPanPos * 511.f, mSqrtLUT);
    }
  }

  void processBlockBalance(AudioBuffer<float> &buffer)
  {
    float *out1 = buffer.getWritePointer(0);
    float *out2 = buffer.getWritePointer(1);
    
    for (int i = 0; i < buffer.getNumSamples(); i++)
    {
      const float in1 = out1[i];
      const float in2 = out2[i];

      const float smoothedPanPos = mPanSmoother.process(mPanPosition);
      out1[i] = in1 * lerp((0.5f + smoothedPanPos) * 511.f, mSqrtLUT);
      out2[i] = in2 * lerp(smoothedPanPos * 511.f, mSqrtLUT);
    }
  }
  
private:
  float mSqrtLUT[512];
  float mPanPosition;
  ParameterSmoother mPanSmoother;
  static constexpr int cSmoothTime = 1.;
};



#endif  // PANNER_H_INCLUDED
