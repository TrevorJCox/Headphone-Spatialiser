/*
  ==============================================================================

    BinauralPanner.h
    Based on binaural-vst by Tomasz Woźniak
    https://github.com/tmwoz/binaural-vst
  
    Author:  Oliver Larkin

  ==============================================================================
*/

#ifndef BINAURALPANNER_H_INCLUDED
#define BINAURALPANNER_H_INCLUDED

#include "../JuceLibraryCode/JuceHeader.h"
#include "HRTFContainer.h"
#include "HRIRFilter.h"
#include "Crossover.h"

class BinauralPanner
{
public:
  BinauralPanner();
  ~BinauralPanner();
  
  void prepareToPlay (double sampleRate, int estimatedSamplesPerBlock);
  void processBlock(AudioBuffer<float> &buffer, int chanIdx = 0);
  
  void setAzimuth(float azimuth) { mAzimuth = jlimit<float>(-180, 180., azimuth); }
  void setElevation(float elevation) { mElevation = jlimit<float>(-90., 90., elevation); }
  void setCrossoverFreq(float freq) { mCrossover.setFrequency(freq); }
  
private:
  float mAzimuth, mElevation;
  float mPrevAzimuth, mPrevElevation;
    
  float mSampleRate;
  
  bool mEnable;
  
  HRIRFilter hrirFilterL;
  HRIRFilter hrirFilterR;
  HRTFContainer mHRTFContainer;
  
  AudioSampleBuffer mLowFreqBuffer;
  AudioSampleBuffer mHighFreqBuffer;
  AudioSampleBuffer mScratchBuffer;
  Crossover mCrossover;
};

#endif  // BINAURALPANNER_H_INCLUDED
