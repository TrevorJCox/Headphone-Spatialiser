#ifndef MAINCOMPONENT_H_INCLUDED
#define MAINCOMPONENT_H_INCLUDED

#include "../JuceLibraryCode/JuceHeader.h"
#include "PluginProcessor.h"
#include "MultiBPFComponent.h"
#include "BinauralPanner/BinauralPannerDisplay.h"

#include "LookAndFeel.h"

class MainComponent  : public Component,
                       public SliderListener,
                       public ComboBoxListener,
                       public ButtonListener,
                       public Timer
{
public:
  MainComponent (SpatialPodcastAudioProcessor& p);
  ~MainComponent();

  void showHidePanElements();

  void paint (Graphics& g) override;
  void paintOverChildren (Graphics& g) override;
  
  void resized() override;
  void sliderValueChanged (Slider* sliderThatWasMoved) override;
  void comboBoxChanged (ComboBox* comboBoxThatHasChanged) override;
  void buttonClicked (Button* buttonThatWasClicked) override;
  
  void timerCallback() override;

private:
  SpatialPodcastAudioProcessor& processor;
  ScopedPointer<AudioProcessorValueTreeState::SliderAttachment> panSliderAttachment;
  ScopedPointer<AudioProcessorValueTreeState::SliderAttachment> eleSliderAttachment;
  ScopedPointer<AudioProcessorValueTreeState::SliderAttachment> distanceSliderAttachment;
  ScopedPointer<AudioProcessorValueTreeState::ButtonAttachment> pannerTypeToggleAttachment;
  ScopedPointer<AudioProcessorValueTreeState::ButtonAttachment> inputTypeToggleAttachment;
  ScopedPointer<AudioProcessorValueTreeState::ComboBoxAttachment> contentTypeComboBoxAttachment;

  ScopedPointer<Slider> panSlider;
//  ScopedPointer<MultiBPFComponent> bpf;
  ScopedPointer<Slider> distanceSlider;
  ScopedPointer<ComboBox> contentTypeComboBox;
  ScopedPointer<Slider> elevationSlider;
  ScopedPointer<ToggleButton> inputTypeToggle;
  ScopedPointer<ToggleButton> pannerTypeToggle;
  ScopedPointer<Drawable> background;
  int panOverlayFrame = 0;
  
  SpatialPodcastLookAndFeel lookAndFeel;
  bool greyOut;
  Image panOverlay;
  
  JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MainComponent)
};

#endif
