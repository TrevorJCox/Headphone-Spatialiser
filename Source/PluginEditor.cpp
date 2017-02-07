/*
  ==============================================================================
  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

SpatialPodcastAudioProcessorEditor::SpatialPodcastAudioProcessorEditor (SpatialPodcastAudioProcessor& p)
: AudioProcessorEditor (&p)
, processor (p)
{
  addAndMakeVisible(mainComponent = new MainComponent(p));
  constrainer.setSizeLimits(512, 384, 1024, 768);
  constrainer.setFixedAspectRatio(1.333f);
  setConstrainer(&constrainer);

  setResizable (true, true);
  setSize (1024, 768);
}

SpatialPodcastAudioProcessorEditor::~SpatialPodcastAudioProcessorEditor()
{
}

void SpatialPodcastAudioProcessorEditor::paint (Graphics& g)
{
}

void SpatialPodcastAudioProcessorEditor::resized()
{
  
  juce::Rectangle<int> bounds = getBounds();
  
  AffineTransform transform = AffineTransform::scale (bounds.getWidth() / 1024.f, bounds.getHeight() / 768.f);
  mainComponent->setTransform(transform);
}
