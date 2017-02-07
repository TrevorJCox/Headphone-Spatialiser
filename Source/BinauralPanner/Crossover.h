/*
  ==============================================================================

    Crossover.h
    Author:  Oliver Larkin

    Linkwitz Riley Crossover Filter, code by Will Pirkle.
 
  ==============================================================================
*/

#ifndef CROSSOVER_H_INCLUDED
#define CROSSOVER_H_INCLUDED

#include "JuceHeader.h"

#define FLT_MIN_PLUS          1.175494351e-38         /* min positive value */
#define FLT_MIN_MINUS        -1.175494351e-38         /* min negative value */

class Crossover
{
public:
  
  class CBiQuad
  {
  public:
    CBiQuad(void) { flushDelays(); };
    virtual ~CBiQuad(void) {};
    
  protected:
    float m_f_Xz_1; // x z-1 delay element
    float m_f_Xz_2; // x z-2 delay element
    float m_f_Yz_1; // y z-1 delay element
    float m_f_Yz_2; // y z-2 delay element
    
    // allow other objects to set these directly since we have no cooking
    // function or intelligence
    
  public:
    float m_f_a0;
    float m_f_a1;
    float m_f_a2;
    float m_f_b1;
    float m_f_b2;
    
    // flush Delays
    void flushDelays()
    {
      m_f_Xz_1 = 0;
      m_f_Xz_2 = 0;
      m_f_Yz_1 = 0;
      m_f_Yz_2 = 0;
    }
    
    // Do the filter: given input xn, calculate output yn and return it
    float doBiQuad(float f_xn)
    {
      // just do the difference equation: y(n) = a0x(n) + a1x(n-1) + a2x(n-2) - b1y(n-1) - b2y(n-2)
      float yn = m_f_a0*f_xn + m_f_a1*m_f_Xz_1 + m_f_a2*m_f_Xz_2 - m_f_b1*m_f_Yz_1 - m_f_b2*m_f_Yz_2;
      
      // underflow check
      if(yn > 0.0 && yn < FLT_MIN_PLUS) yn = 0;
      if(yn < 0.0 && yn > FLT_MIN_MINUS) yn = 0;
      
      // shuffle delays
      // Y delays
      m_f_Yz_2 = m_f_Yz_1;
      m_f_Yz_1 = yn;
      
      // X delays
      m_f_Xz_2 = m_f_Xz_1;
      m_f_Xz_1 = f_xn;
      
      return  yn;
    }
  };
  
  Crossover()
  : m_nSampleRate(44100.)
  {
  }
  
  ~Crossover()
  {
  }
  
  void prepareToPlay (double sampleRate)
  {
    m_nSampleRate = sampleRate;
  }
  
  void processBlock (AudioSampleBuffer& inputBuffer, AudioSampleBuffer& lfBuffer, AudioSampleBuffer& hfBuffer, int chanIdx)
  {
    const float *input1 = inputBuffer.getReadPointer(chanIdx);
    float *lfout1 = lfBuffer.getWritePointer(0);
    float *hfout1 = hfBuffer.getWritePointer(0);
   
    for(int i=0;i<inputBuffer.getNumSamples();i++)
    {
      lfout1[i] = mLPF.doBiQuad(input1[i]);
      hfout1[i] = mHPF.doBiQuad(input1[i]);
      
      // invert ONE of the outputs for proper recombination
      hfout1[i] *= -1.0;
    }
  }
  
  void setFrequency(float freq) { freq = jlimit<float>(10, 20000., freq); calculateFilterBankCoeffs(freq); }
  
private:
  void calculateFilterBankCoeffs(float fCutoffFreq)
  {
    // Shared Factors:
    float omega_c = pi*fCutoffFreq;
    float theta_c = pi*fCutoffFreq/(float)m_nSampleRate;
    
    float k = omega_c/tan(theta_c);
    float k_squared = k*k;
    
    float omega_c_squared = omega_c*omega_c;
    
    float fDenominator = k_squared + omega_c_squared + 2.0f*k*omega_c;
    
    float fb1_Num = -2.0f*k_squared + 2.0*omega_c_squared;
    float fb2_Num = -2.0f*k*omega_c + k_squared + omega_c_squared;
    
    // the LPF coeffs
    float a0 = omega_c_squared/fDenominator;
    float a1 = 2.0f*omega_c_squared/fDenominator;
    float a2 = a0;
    float b1 = fb1_Num/fDenominator;
    float b2 = fb2_Num/fDenominator;
    
    // set the LPFs
    mLPF.m_f_a0 = a0;
    mLPF.m_f_a1 = a1;
    mLPF.m_f_a2 = a2;
    mLPF.m_f_b1 = b1;
    mLPF.m_f_b2 = b2;
    
    // the HPF coeffs
    a0 = k_squared/fDenominator;
    a1 = -2.0f*k_squared/fDenominator;
    a2 = a0;
    b1 = fb1_Num/fDenominator;
    b2 = fb2_Num/fDenominator;
    
    // set the HPFs
    mHPF.m_f_a0 = a0;
    mHPF.m_f_a1 = a1;
    mHPF.m_f_a2 = a2;
    mHPF.m_f_b1 = b1;
    mHPF.m_f_b2 = b2;
  }
  
  CBiQuad mLPF;
  CBiQuad mHPF;
  
  double m_nSampleRate;
  static constexpr float pi = 3.141592653589793;
  
  JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (Crossover)
};



#endif  // CROSSOVER_H_INCLUDED
