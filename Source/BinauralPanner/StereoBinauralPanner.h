/*
  ==============================================================================

    StereoBinauralPanner.h
    Author:  Oliver Larkin

  ==============================================================================
*/

#ifndef STEREOBINAURALPANNER_H_INCLUDED
#define STEREOBINAURALPANNER_H_INCLUDED

#include "BinauralPanner.h"

class StereoBinauralPanner
{
public:
  StereoBinauralPanner()
  {
  }
  
  ~StereoBinauralPanner()
  {
  }
  
  void prepareToPlay (double sampleRate, int estimatedSamplesPerBlock)
  {
    mPannerLeft.prepareToPlay(sampleRate, estimatedSamplesPerBlock);
    mPannerRight.prepareToPlay(sampleRate, estimatedSamplesPerBlock);
    mScratchBuffer.setSize(2, estimatedSamplesPerBlock);
  }
  
  void processBlock(AudioBuffer<float> &buffer)
  {
    auto bufferLength = buffer.getNumSamples();
    
    mScratchBuffer.copyFrom(0, 0, buffer, 0, 0, bufferLength);
    mScratchBuffer.copyFrom(1, 0, buffer, 1, 0, bufferLength);
    
    mPannerLeft.processBlock(buffer, 0);
    mPannerRight.processBlock(mScratchBuffer, 1);
    
    buffer.addFrom(0, 0, mScratchBuffer, 0, 0, bufferLength);
    buffer.addFrom(1, 0, mScratchBuffer, 1, 0, bufferLength);
  }
  
  void setWidth(float width)
  {
    width = jlimit<float>(0., 180., width) * 0.5f;
    
    mPannerLeft.setAzimuth(0. - width);
    mPannerRight.setAzimuth(width);
  }

  void setElevation(float elevation)
  {
    mPannerLeft.setElevation(elevation);
    mPannerRight.setElevation(elevation);
  }
  
  void setCrossoverFreq(float freq)
  {
    mPannerLeft.setCrossoverFreq(freq);
    mPannerRight.setCrossoverFreq(freq);
  }
  
private:
  BinauralPanner mPannerLeft;
  BinauralPanner mPannerRight;
  AudioSampleBuffer mScratchBuffer;
};



#endif  // STEREOBINAURALPANNER_H_INCLUDED
