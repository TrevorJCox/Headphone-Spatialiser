/*
  ==============================================================================

    BinauralPanner.cpp
    Based on binaural-vst by Tomasz Woźniak
    https://github.com/tmwoz/binaural-vst
 
    Author:  Oliver Larkin

  ==============================================================================
*/

#include "BinauralPanner.h"

BinauralPanner::BinauralPanner()
: mAzimuth(0)
, mElevation(0)
, mPrevAzimuth(-1)
, mPrevElevation(-1)
, mSampleRate(44100.)
, mEnable(false)
{
  mHRTFContainer.loadHrir();
  mHRTFContainer.updateHRIR(0, 0);
}

BinauralPanner::~BinauralPanner()
{
}

void BinauralPanner::prepareToPlay (double sampleRate, int estimatedSamplesPerBlock)
{
  mSampleRate = (float) sampleRate;
  
  mCrossover.prepareToPlay(sampleRate);
  mLowFreqBuffer.setSize(1, estimatedSamplesPerBlock);
  mHighFreqBuffer.setSize(1, estimatedSamplesPerBlock);
  mScratchBuffer.setSize(2, estimatedSamplesPerBlock);
  
  if(isPowerOfTwo(estimatedSamplesPerBlock))
  {
    hrirFilterL.prepare(estimatedSamplesPerBlock);
    hrirFilterR.prepare(estimatedSamplesPerBlock);
    mEnable = true;
  }
  else
    mEnable = false;
}

void BinauralPanner::processBlock(AudioBuffer<float> &buffer, int chanIdx)
{
  if(!mEnable)
    return;
  
  if(mAzimuth != mPrevAzimuth || mElevation != mPrevElevation)
  {
    Point3DoublePolar<float> sourcePos;
    sourcePos.radius = 1.;
    sourcePos.azimuth = deg2rad(mAzimuth);
    sourcePos.elevation = deg2rad(mElevation);

    auto p = sphericalToInteraural(sourcePos);

    mHRTFContainer.updateHRIR(rad2deg(p.azimuth), rad2deg(p.elevation));
    mPrevAzimuth = mAzimuth;
    mPrevElevation = mElevation;
  }
  
  // get a pointer to the left or right channel data
  auto bufferLength = buffer.getNumSamples();

  // split the input signal into two bands, only freqs above crossover's f0 will be spatialized
  mCrossover.processBlock(buffer, mLowFreqBuffer, mHighFreqBuffer, chanIdx);
  
  // copy high freq buffer into scratch
  mScratchBuffer.copyFrom(0, 0, mHighFreqBuffer, 0, 0, bufferLength);
  mScratchBuffer.copyFrom(1, 0, mHighFreqBuffer, 0, 0, bufferLength);

  // actual hrir filtering
  const auto& hrir = mHRTFContainer.hrir();
  hrirFilterL.setImpulseResponse(hrir.leftEarIR);
  hrirFilterR.setImpulseResponse(hrir.rightEarIR);
  hrirFilterL.process(mScratchBuffer.getWritePointer(0), bufferLength);
  hrirFilterR.process(mScratchBuffer.getWritePointer(1), bufferLength);
  
  // copy to output
  float* outL = buffer.getWritePointer(0);
  float* outR = buffer.getWritePointer(1);
  
  for (int i = 0; i < bufferLength; i++)
  {
    outL[i] = (mLowFreqBuffer.getReadPointer(0)[i] + mScratchBuffer.getReadPointer(0)[i]) * 0.5f;
    outR[i] = (mLowFreqBuffer.getReadPointer(0)[i] + mScratchBuffer.getReadPointer(1)[i]) * 0.5f;
  }
}