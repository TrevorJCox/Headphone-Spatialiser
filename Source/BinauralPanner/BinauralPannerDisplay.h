/*
  ==============================================================================

    BinauralPannerDisplay.h
    Author:  Oliver Larkin

  ==============================================================================
*/

#ifndef BINAURALPANNERSLIDER_H_INCLUDED
#define BINAURALPANNERSLIDER_H_INCLUDED

#include "JuceHeader.h"
#include "Util.h"
#include "../PluginProcessor.h"

class BinauralPannerDisplay : public Component
{
public:
  BinauralPannerDisplay(SpatialPodcastAudioProcessor& p);
  ~BinauralPannerDisplay();
  
  void paint(Graphics& g) override;
  void mouseDrag(const MouseEvent& event) override;
  
private:
  void drawGridLines(Graphics& g);
  void drawSource(Graphics& g);
  void updateHRTF();
  
  SpatialPodcastAudioProcessor& processor;
  
  Image mHeadImage;
  Image mSourceImage;
  Point3DoublePolar<float> mSourcePos;
  
  JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(BinauralPannerDisplay)
};


#endif  // BINAURALPANNERSLIDER_H_INCLUDED
