/*
  ==============================================================================

    MultiBPFComponent.h
    Author:  Oliver Larkin

  ==============================================================================
*/

#ifndef MULTIBPFCOMPONENT_H_INCLUDED
#define MULTIBPFCOMPONENT_H_INCLUDED

#include "../JuceLibraryCode/JuceHeader.h"

class MultiBPFComponent    : public Component
{
public:
  MultiBPFComponent(SpatialPodcastAudioProcessor& p)
  {
    addAndMakeVisible (tabbedComponent = new TabbedComponent (TabbedButtonBar::TabsAtTop));
    tabbedComponent->setTabBarDepth (23);
    tabbedComponent->addTab (TRANS("Distance To Direct Level"), Colours::lightgrey, p.mDistanceToDryBPF, false);
    tabbedComponent->addTab (TRANS("Distance To Diffuse Level"), Colours::lightgrey, p.mDistanceToWetBPF, false);
    tabbedComponent->addTab (TRANS("Distance To Filtered Mix"), Colours::lightgrey, p.mDistanceToFilteredBPF, false);
  }

  ~MultiBPFComponent()
  {
    tabbedComponent = nullptr;
  }

  void paint (Graphics& g) override
  {
  }

  void resized() override
  {
    tabbedComponent->setBounds (8, 8, getWidth() - 16, getHeight() - 16);
  }

private:
  ScopedPointer<TabbedComponent> tabbedComponent;
  JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MultiBPFComponent)
};


#endif  // MULTIBPFCOMPONENT_H_INCLUDED
