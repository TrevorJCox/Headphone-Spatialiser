/*
  ==============================================================================

    TrapezoidalSVF.h
    Author:  Oliver Larkin
  
  Based on Andy Simper's code: http://www.cytomic.com/files/dsp/SvfLinearTrapOptimised2.pdf

  ==============================================================================
*/

#ifndef TRAPEZOIDALSVF_H_INCLUDED
#define TRAPEZOIDALSVF_H_INCLUDED

#include "JuceHeader.h"
#include "ParameterSmoother.h"

class TrapezoidalSVF
{
public:
  
  enum EFilterMode
  {
    kLow = 0,
    kBand,
    kHigh,
    kNotch,
    kPeak,
    kBell,
    kLowShelf,
    kHighShelf
  };
  
  TrapezoidalSVF(EFilterMode mode = kLow);
  ~TrapezoidalSVF();
  
  void prepareToPlay (double sampleRate, int samplesPerBlock);  
  void processBlock (AudioSampleBuffer& buffer);
  
  void setFrequency(double freq) { mFrequency = jlimit<double>(10, 20000., freq); }
  void setQ(double q) { mQ = jlimit<double>(0.1, 10., q); }
  void setBellGain(double gain) { mBellGain = jlimit<double>(-36, 36., gain); }
  void setType(EFilterMode mode) { mFilterMode = mode; }
  void setMix(float mix) { mMix = mix; }
  
private:
  double v1[2], v2[2], v3[2], ic1eq[2], ic2eq[2], a1, a2, a3, m0, m1, m2;
  EFilterMode mFilterMode;
  double mSampleRate;
  double mFrequency, mQ, mBellGain;
  float mMix;
  ParameterSmoother mMixSmoother;
  static constexpr int cSmoothTime = 1.;
  
  JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (TrapezoidalSVF)
};



#endif  // TRAPEZOIDALSVF_H_INCLUDED
