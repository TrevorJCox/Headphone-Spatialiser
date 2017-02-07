/*
  ==============================================================================

    ConvolutionReverb.h
    Author:  Oliver Larkin
 
    Based on code by Theo Niessink http://www.taletn.com/wdl/

  ==============================================================================
*/

#ifndef CONVOLUTIONREVERB_H_INCLUDED
#define CONVOLUTIONREVERB_H_INCLUDED

#include "../JuceLibraryCode/JuceHeader.h"

#include "WDL/convoengine.h"
#undef min
#undef max
#include "r8brain/CDSPResampler.h"
#include "nonblocking_call_queue.h"
#include "ParameterSmoother.h"

class ConvolutionReverb : public Thread
{
public:
  ConvolutionReverb();
  ~ConvolutionReverb();
  
  void prepareToPlay (double sampleRate, int estimatedSamplesPerBlock);  
  bool setPreferredBusArrangement (bool isInput, int bus, const AudioChannelSet& preferredSet);

  void processBlock(AudioBuffer<float> &buffer, MidiBuffer &midiMessages);
  
  //Thread
  void run() override;
  
  //Unique
  void loadNewIRAsync(File& audioFile); // call from message thread
  void loadNewIRFromMemoryAsync(const void* sourceData, size_t sourceDataSize); // call from message thread
  void loadNewIRFromMemory(const void* sourceData, size_t sourceDataSize); // call from message thread
  
  File getFile() { return mFile; }
  
  void setDryLevel(float leveldB) { mDryLevel = Decibels::decibelsToGain(leveldB); }
  void setWetLevel(float leveldB) { mWetLevel = Decibels::decibelsToGain(leveldB); }
  void setMix(float mix) { mDryLevel = cosf(mix*1.5708f); mWetLevel = sinf(mix*1.5708f); }
  
  
private:
  template <class I, class O> void resampleIR(const I* src, int srcLen, double srcRate, O* dest, int destLen, double destRate);
  
  inline int calcResampleLength(int srcLen, double srcRate, double destRate) const
  {
    return static_cast<int>(destRate / srcRate * static_cast<double>(srcLen) + 0.5);
  }
  
  void updateConvoEngineIR(int numChans, double IRSampleRate);
  
private:
  double mSampleRate;
  double mIRSampleRate;
  static constexpr int cBlockLength = 64;
  static constexpr int cSmoothTime = 1.;
  File mFile;
  const void* mSourceData = nullptr;
  size_t mSourceDataSize = 0;
  
  WDL_ImpulseBuffer mIR;
  WDL_ConvolutionEngine_Div mEngine;
  ScopedPointer<r8b::CDSPResampler16IR> mResampler;
  ScopedPointer<AudioSampleBuffer> mIRAudioSampleBuffer;
  LockFreeCallQueue mLoadThreadToAudioThreadCallQueue;
  ParameterSmoother mDryLevelSmoother;
  ParameterSmoother mWetLevelSmoother;
  
  float mDryLevel;
  float mWetLevel;
  
  bool mImpulseLoaded;
};

#endif  // CONVOLUTIONREVERB_H_INCLUDED
