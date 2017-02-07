/*
  ==============================================================================
  ==============================================================================
*/

#ifndef PLUGINEDITOR_H_INCLUDED
#define PLUGINEDITOR_H_INCLUDED

#include "../JuceLibraryCode/JuceHeader.h"
#include "PluginProcessor.h"
#include "MainComponent.h"

class SpatialPodcastAudioProcessorEditor  : public AudioProcessorEditor
{
public:
  SpatialPodcastAudioProcessorEditor (SpatialPodcastAudioProcessor&);
  ~SpatialPodcastAudioProcessorEditor();

  void paint (Graphics&) override;
  void resized() override;

private:
  SpatialPodcastAudioProcessor& processor;
  ScopedPointer<MainComponent> mainComponent;
  ComponentBoundsConstrainer constrainer;
  
  JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (SpatialPodcastAudioProcessorEditor)
};


#endif  // PLUGINEDITOR_H_INCLUDED
