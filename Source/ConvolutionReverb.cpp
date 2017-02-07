/*
  ==============================================================================

   ConvolutionReverb.cpp
   Author:  Oliver Larkin

  ==============================================================================
*/

#include "ConvolutionReverb.h"

ConvolutionReverb::ConvolutionReverb()
: Thread("ConvolutionReverb Sample Loading Thread")
, mSampleRate(0.)
, mIRSampleRate(44100.)
, mLoadThreadToAudioThreadCallQueue(1024)
, mDryLevel(1.)
, mWetLevel(1.)
, mImpulseLoaded(false)
{
  //mEngine.EnableThread(true);
}

ConvolutionReverb::~ConvolutionReverb()
{
  if (isThreadRunning())
  {
    stopThread(-1);
  }
}

void ConvolutionReverb::prepareToPlay (double sampleRate, int estimatedSamplesPerBlock)
{
  // Detect a change in sample rate.
  if (sampleRate != mSampleRate)
  {
    mSampleRate = sampleRate;
    
    if(mIRAudioSampleBuffer != nullptr)
      updateConvoEngineIR(mIRAudioSampleBuffer->getNumChannels(), mIRSampleRate);
    
    mDryLevelSmoother.setTimeMS(cSmoothTime, mSampleRate);
    mWetLevelSmoother.setTimeMS(cSmoothTime, mSampleRate);
  }
}

void ConvolutionReverb::processBlock(AudioBuffer<float> &buffer, MidiBuffer &midiMessages)
{
  mLoadThreadToAudioThreadCallQueue.synchronize(); // will update IR if changed
  
  const int numSamples = buffer.getNumSamples();
  
  // Send input samples to the convolution engine.
  if(mImpulseLoaded)
    mEngine.Add(buffer.getArrayOfWritePointers(), numSamples, 2);
  
  int nAvail = std::min(mEngine.Avail( numSamples),  numSamples);
  
  const float* in1 = buffer.getReadPointer(0);
  const float* in2 = buffer.getReadPointer(1);
  
  float *out1 = buffer.getWritePointer(0);
  float *out2 = buffer.getWritePointer(1);
  
  // If not enough samples are available yet, then only output the dry
  // signal.
  for (int i = 0; i < numSamples - nAvail; ++i)
  {
    const float smoothedDryLevel = mDryLevelSmoother.process(mDryLevel);
    
    *out1++ = smoothedDryLevel * *in1++;
    *out2++ = smoothedDryLevel * *in2++;
  }
  
  // Output samples from the convolution engine.
  if (nAvail > 0)
  {
    WDL_FFT_REAL* convo1 = mEngine.Get()[0];
    WDL_FFT_REAL* convo2 = mEngine.Get()[1];
    
    for (int i = 0; i < nAvail; ++i)
    {
      const float smoothedDryLevel = mDryLevelSmoother.process(mDryLevel);
      const float smoothedWetLevel = mWetLevelSmoother.process(mWetLevel);
      
      *out1++ = smoothedDryLevel * *in1++ + smoothedWetLevel * *convo1++;
      *out2++ = smoothedDryLevel * *in2++ + smoothedWetLevel * *convo2++;
    }
    
    // Remove the sample block from the convolution engine's buffer.
    mEngine.Advance(nAvail);
  }
}

template <class I, class O>
void ConvolutionReverb::resampleIR(const I* src, int srcLen, double srcRate, O* dest, int destLen, double destRate)
{
  if (destLen == srcLen)
  {
    // Copy
    for (int i = 0; i < destLen; ++i)
      *dest++ = (O)*src++;
    
    return;
  }
  
  double scale = srcRate / destRate;
  
  while (destLen > 0)
  {
    double buf[cBlockLength], *p = buf;
    int n = cBlockLength;
    
    if (n > srcLen)
      n = srcLen;
    
    for (int i = 0; i < n; ++i)
      *p++ = (double)*src++;
    
    if (n < cBlockLength)
      memset(p, 0, (cBlockLength - n) * sizeof(double));
    
    srcLen -= n;
    
    n = mResampler->process(buf, cBlockLength, p);
    
    if (n > destLen)
      n = destLen;
    
    for (int i = 0; i < n; ++i)
      *dest++ = (O)(scale * *p++);
    
    destLen -= n;
  }
  
  mResampler->clear();
}

void ConvolutionReverb::run()
{
  if(mSourceData != nullptr)
  {
    WavAudioFormat wavFormat;
    
    ScopedPointer<AudioFormatReader> audioReader (wavFormat.createReaderFor (new MemoryInputStream(mSourceData, mSourceDataSize, false), true));
    
    mIRAudioSampleBuffer = new AudioSampleBuffer (jmax (1, (int) audioReader->numChannels), audioReader->lengthInSamples);
    
    audioReader->read (mIRAudioSampleBuffer, 0, audioReader->lengthInSamples, 0, true, true);
    
    mLoadThreadToAudioThreadCallQueue.callf(std::bind(&ConvolutionReverb::updateConvoEngineIR, this, audioReader->numChannels, audioReader->sampleRate));
    
    mSourceData = nullptr;
    mSourceDataSize = 0;
  }
    if(mFile.exists())
  {
    WavAudioFormat wavFormat;

    ScopedPointer<AudioFormatReader> audioReader (wavFormat.createReaderFor (new FileInputStream(mFile), true));

    mIRAudioSampleBuffer = new AudioSampleBuffer (jmax (1, (int) audioReader->numChannels), audioReader->lengthInSamples);

    audioReader->read (mIRAudioSampleBuffer, 0, audioReader->lengthInSamples, 0, true, true);
    
    mLoadThreadToAudioThreadCallQueue.callf(std::bind(&ConvolutionReverb::updateConvoEngineIR, this, audioReader->numChannels, audioReader->sampleRate));
  }
  else
    mFile = File::nonexistent;
}

void ConvolutionReverb::loadNewIRAsync(File& audioFile)
{
  mFile = audioFile;
  startThread();
}

void ConvolutionReverb::loadNewIRFromMemory(const void* sourceData, size_t sourceDataSize)
{
  mSourceData = sourceData;
  mSourceDataSize = sourceDataSize;
  run();
}

void ConvolutionReverb::loadNewIRFromMemoryAsync(const void* sourceData, size_t sourceDataSize)
{
  mSourceData = sourceData;
  mSourceDataSize = sourceDataSize;
  startThread();
}

void ConvolutionReverb::updateConvoEngineIR(int numChans, double IRSampleRate)
{
  mIRSampleRate = IRSampleRate;
  mIR.SetNumChannels(numChans);
  
  //scoped pointer so old one gets deleted
  mResampler = new r8b::CDSPResampler16IR(IRSampleRate, mSampleRate, cBlockLength);
  
  // Resample the impulse response.
  int impulseLength = mIR.SetLength(calcResampleLength(mIRAudioSampleBuffer->getNumSamples(), IRSampleRate, mSampleRate));
  
  if (impulseLength)
  {
    for(auto chan=0;chan<numChans;chan++)
      resampleIR(mIRAudioSampleBuffer->getReadPointer(chan), mIRAudioSampleBuffer->getNumSamples(), IRSampleRate, mIR.impulses[chan].Get(), impulseLength, mSampleRate);
  }
  
  // Tie the impulse response to the convolution engine.
  mEngine.SetImpulse(&mIR);
  
  mImpulseLoaded = true;
}