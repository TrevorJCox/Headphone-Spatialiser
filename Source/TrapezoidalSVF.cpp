/*
  ==============================================================================

    TrapezoidalSVF.cpp
    Author:  Oliver Larkin

  ==============================================================================
*/

#include "TrapezoidalSVF.h"

TrapezoidalSVF::TrapezoidalSVF(EFilterMode mode)
: a1(0.), a2(0.), a3(0.), m0(0.), m1(0.), m2(0.)
, mFilterMode(mode)
, mSampleRate(44100.)
, mFrequency(440.)
, mQ(0.1)
, mBellGain(1.)
, mMix(0.)
{
  v1[0] = 0.;
  v1[1] = 0.;
  v2[0] = 0.;
  v2[1] = 0.;
  v3[0] = 0.;
  v3[1] = 0.;
  ic1eq[0] = 0.;
  ic1eq[1] = 0.;
  ic2eq[0] = 0.;
  ic2eq[1] = 0.;
}

TrapezoidalSVF::~TrapezoidalSVF()
{
}

void TrapezoidalSVF::prepareToPlay (double sampleRate, int estimatedSamplesPerBlock)
{
  mSampleRate = sampleRate;
  mMixSmoother.setTimeMS(cSmoothTime, mSampleRate);
}

void TrapezoidalSVF::processBlock (AudioSampleBuffer& buffer)
{
  //TODO: !! coefficients don't need to be updated each block
  switch(mFilterMode)
  {
    case kLow:
    {
      double g = tan(3.141592653589793 * mFrequency/mSampleRate);
      double k = 1. / mQ;
      a1 = 1./(1. + g*(g + k));
      a2 = g*a1;
      a3 = g*a2;
      m0 = 0;
      m1 = 0;
      m2 = 1.;
      break;
    }
    case kBand:
    {
      double g = tan(3.141592653589793 * mFrequency/mSampleRate);
      double k = 1. / mQ;
      a1 = 1./(1. + g*(g + k));
      a2 = g*a1;
      a3 = g*a2;
      m0 = 0.;
      m1 = 1.;
      m2 = 0.;
      break;
    }
    case kHigh:
    {
      double g = tan(3.141592653589793 * mFrequency/mSampleRate);
      double k = 1. / mQ;
      a1 = 1./(1. + g*(g + k));
      a2 = g*a1;
      a3 = g*a2;
      m0 = 1.;
      m1 = -k;
      m2 = -1.;
      break;
    }
    case kNotch:
    {
      double g = tan(3.141592653589793 * mFrequency/mSampleRate);
      double k = 1. / mQ;
      a1 = 1./(1. + g*(g + k));
      a2 = g*a1;
      a3 = g*a2;
      m0 = 1.;
      m1 = -k;
      m2 = -2.;
      break;
    }
    case kPeak:
    {
      double g = tan(3.141592653589793 * mFrequency/mSampleRate);
      double k = 1. / mQ;
      a1 = 1./(1. + g*(g + k));
      a2 = g*a1;
      a3 = g*a2;
      m0 = 1.;
      m1 = -k;
      m2 = 0.;
      break;
    }
    case kBell:
    {
      double A = pow(10., mBellGain/40.);
      double g = tan(3.141592653589793 * mFrequency/mSampleRate);
      double k = 1 / mQ;
      a1 = 1./(1. + g*(g + k));
      a2 = g*a1;
      a3 = g*a2;
      m0 = 1.;
      m1 = k*(A*A - 1.);
      m2 = 0.;
      break;
    }
    case kLowShelf:
    {
      double A = pow(10., mBellGain/40.);
      double g = tan(3.141592653589793 * mFrequency/mSampleRate) / sqrt(A);
      double k = 1. / mQ;
      a1 = 1./(1. + g*(g + k));
      a2 = g*a1;
      a3 = g*a2;
      m0 = 1.;
      m1 = k*(A - 1.);
      m2 = (A*A - 1.);
      break;
    }
    case kHighShelf:
    {
      double A = pow(10., mBellGain/40.);
      double g = tan(3.141592653589793 * mFrequency/mSampleRate) / sqrt(A);
      double k = 1. / mQ;
      a1 = 1./(1. + g*(g + k));
      a2 = g*a1;
      a3 = g*a2;
      m0 = A*A;
      m1 = k*(1. - A)*A;
      m2 = (1. - A*A);
      break;
    }
    default:
      break;
  }
  
  const float* in1 = buffer.getReadPointer(0);
  const float* in2 = buffer.getReadPointer(1);

  float *out1 = buffer.getWritePointer(0);
  float *out2 = buffer.getWritePointer(1);
  
  for (int i = 0; i < buffer.getNumSamples(); i++)
  {
    const float smoothedMix = mMixSmoother.process(mMix);
    
    double v0 = (double) in1[i];
  
    v3[0] = v0 - ic2eq[0];
    v1[0] = a1*ic1eq[0] + a2*v3[0];
    v2[0] = ic2eq[0] + a2*ic1eq[0] + a3*v3[0];
    ic1eq[0] = 2*v1[0] - ic1eq[0];
    ic2eq[0] = 2*v2[0] - ic2eq[0];
  
    float leftOutput = (float) m0*v0 + m1*v1[0] + m2*v2[0];
    
    v0 = (double) in2[i];
    
    v3[1] = v0 - ic2eq[1];
    v1[1] = a1*ic1eq[1] + a2*v3[1];
    v2[1] = ic2eq[1] + a2*ic1eq[1] + a3*v3[1];
    ic1eq[1] = 2*v1[1] - ic1eq[1];
    ic2eq[1] = 2*v2[1] - ic2eq[1];
    
    float rightOutput = (float) m0*v0 + m1*v1[1] + m2*v2[1];
    
    out1[i] = (smoothedMix-1) * in1[i] + smoothedMix * leftOutput;
    out2[i] = (smoothedMix-1) * in2[i] + smoothedMix * rightOutput;
  }
}
