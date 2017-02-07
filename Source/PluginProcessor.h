/*
  ==============================================================================
  ==============================================================================
*/

#ifndef PLUGINPROCESSOR_H_INCLUDED
#define PLUGINPROCESSOR_H_INCLUDED

#include "../JuceLibraryCode/JuceHeader.h"

#include "ConvolutionReverb.h"
#include "Panner.h"
#include "BinauralPanner/BinauralPanner.h"
#include "BinauralPanner/StereoBinauralPanner.h"
#include "TrapezoidalSVF.h"
#include "value_tree_debugger.h"
#include "BreakPointFunction.h"

enum EContentType
{
  kNarration = 0,
  kDialogue,
  kSFX,
  kAtmosphere,
  kMusic,
  kNumContentTypes
};

enum EPannerType
{
  kSqrt = 0,
  kBinaural = 1,
  kNumPannerTypes
};

enum EInputType
{
  kMono = 0,
  kStereo = 1,
  kNumInputTypes
};

const String ContentTypes[5] =
{
  "Narration",
  "Dialogue",
  "SFX",
  "Atmosphere",
  "Music"
};

class SpatialPodcastAudioProcessor  : public AudioProcessor
                                    , public AudioProcessorValueTreeState::Listener
{
public:
  SpatialPodcastAudioProcessor();
  ~SpatialPodcastAudioProcessor();

  void prepareToPlay (double sampleRate, int samplesPerBlock) override;
  void releaseResources() override;

//  bool setPreferredBusArrangement (bool isInput, int bus, const AudioChannelSet& preferredSet) override;

  void processBlock (AudioSampleBuffer&, MidiBuffer&) override;

  AudioProcessorEditor* createEditor() override;
  bool hasEditor() const override { return true; };

  const String getName() const override { return JucePlugin_Name; };

  bool acceptsMidi() const override { return false; };
  bool producesMidi() const override  { return false; };
  double getTailLengthSeconds() const override { return 0.0; };

  int getNumPrograms() override { return 1; };
  int getCurrentProgram() override { return 0; };
  void setCurrentProgram (int index) override {}
  const String getProgramName (int index) override { return String(); }
  void changeProgramName (int index, const String& newName) override {}

  void getStateInformation (MemoryBlock& destData) override;
  void setStateInformation (const void* data, int sizeInBytes) override;
  
  void parameterChanged (const String& parameterID, float newValue) override;
    
  ConvolutionReverb mConvolutionReverb;
  BinauralPanner mMonoBinauralPanner;
  StereoBinauralPanner mStereoBinauralPanner;
  TrapezoidalSVF mFilter;
  AudioProcessorValueTreeState mAPVTS;
  ValueTree mDistanceToDryMapping = ValueTree("DistanceToDryMapping");
  ValueTree mDistanceToWetMapping = ValueTree("DistanceToWetMapping");
  ValueTree mDistanceToFilteredMapping = ValueTree("DistanceToFilteredMapping");
//  ValueTree mVisibilityOptions;
  ScopedPointer<ValueTreeDebugger> valueTreeDebugger;
  
  ScopedPointer<BreakPointFunction> mDistanceToDryBPF;
  ScopedPointer<BreakPointFunction> mDistanceToWetBPF;
  ScopedPointer<BreakPointFunction> mDistanceToFilteredBPF;
  
private:
  Panner mPanner;
  
  JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (SpatialPodcastAudioProcessor)
};


#endif  // PLUGINPROCESSOR_H_INCLUDED
