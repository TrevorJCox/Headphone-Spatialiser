#include "MainComponent.h"

MainComponent::MainComponent (SpatialPodcastAudioProcessor& p)
    : processor(p)
    , greyOut(false)
    , panOverlay(ImageCache::getFromMemory(BinaryData::PanAziWidth_png, BinaryData::PanAziWidth_pngSize))
{
  getLookAndFeel().setDefaultLookAndFeel(&lookAndFeel);

  addAndMakeVisible (panSlider = new Slider ("panSlider"));
  panSlider->setRange (-1, 1, 0.1);
  panSlider->setSliderStyle (Slider::RotaryVerticalDrag);
  panSlider->setTextBoxStyle (Slider::NoTextBox, false, 80, 20);
  panSlider->addListener (this);

//  addAndMakeVisible (bpf = new MultiBPFComponent (p));
//  bpf->setName ("Break Point Functions");

  addAndMakeVisible (distanceSlider = new Slider ("distanceSlider"));
  distanceSlider->setRange (0, 1, 0.01);
  distanceSlider->setSliderStyle (Slider::LinearHorizontal);
  distanceSlider->setTextBoxStyle (Slider::NoTextBox, false, 80, 20);
  distanceSlider->addListener (this);

  addAndMakeVisible (contentTypeComboBox = new ComboBox ("contentTypeComboBox"));
  contentTypeComboBox->setEditableText (false);
  contentTypeComboBox->setJustificationType (Justification::centredLeft);
  contentTypeComboBox->setTextWhenNothingSelected (TRANS("Narration"));
  contentTypeComboBox->setTextWhenNoChoicesAvailable (TRANS("(no choices)"));
  contentTypeComboBox->addItem (TRANS("Narration"), 1);
  contentTypeComboBox->addItem (TRANS("Dialogue"), 2);
  contentTypeComboBox->addItem (TRANS("SFX"), 3);
  contentTypeComboBox->addItem (TRANS("Atmosphere"), 4);
  contentTypeComboBox->addItem (TRANS("Music"), 5);
  contentTypeComboBox->addListener (this);

  addAndMakeVisible (elevationSlider = new Slider ("elevationSlider"));
  elevationSlider->setRange (-40, 90, 0.1);
  elevationSlider->setSliderStyle (Slider::LinearVertical);
  elevationSlider->setTextBoxStyle (Slider::NoTextBox, false, 80, 20);
  elevationSlider->addListener (this);

  addAndMakeVisible (inputTypeToggle = new ToggleButton ("inputTypeToggle"));
  inputTypeToggle->setButtonText (String());
  inputTypeToggle->addListener (this);

  addAndMakeVisible (pannerTypeToggle = new ToggleButton ("pannerTypeToggle"));
  pannerTypeToggle->setButtonText (String());
  pannerTypeToggle->addListener (this);

  background = Drawable::createFromImageData (BinaryData::Background_png, BinaryData::Background_pngSize);
  
  panSliderAttachment = new AudioProcessorValueTreeState::SliderAttachment (processor.mAPVTS, "Pan", *panSlider);
  eleSliderAttachment = new AudioProcessorValueTreeState::SliderAttachment (processor.mAPVTS, "Elevation", *elevationSlider);
  distanceSliderAttachment = new AudioProcessorValueTreeState::SliderAttachment (processor.mAPVTS, "Distance", *distanceSlider);
  pannerTypeToggleAttachment = new AudioProcessorValueTreeState::ButtonAttachment (processor.mAPVTS, "PannerType", *pannerTypeToggle);
  contentTypeComboBoxAttachment = new AudioProcessorValueTreeState::ComboBoxAttachment (processor.mAPVTS, "ContentType", *contentTypeComboBox);
  inputTypeToggleAttachment = new AudioProcessorValueTreeState::ButtonAttachment (processor.mAPVTS, "InputType", *inputTypeToggle);

//  azimuthSlider->setRotaryParameters(3.141592653589793, 3.141592653589793 * 3.f, true);

  setSize (1024, 768);
  startTimer(50);
}

MainComponent::~MainComponent()
{
  panSliderAttachment = nullptr;
  eleSliderAttachment = nullptr;
  distanceSliderAttachment = nullptr;
  pannerTypeToggleAttachment = nullptr;
  contentTypeComboBoxAttachment = nullptr;
  inputTypeToggleAttachment = nullptr;

  panSlider = nullptr;
//    bpf = nullptr;
  distanceSlider = nullptr;
  contentTypeComboBox = nullptr;
  elevationSlider = nullptr;
  inputTypeToggle = nullptr;
  pannerTypeToggle = nullptr;
  background = nullptr;
}

//==============================================================================
void MainComponent::paint (Graphics& g)
{
  g.fillAll (Colours::white);
  g.setColour (Colours::black);
  
  jassert (background != nullptr);
  
  if (background != nullptr)
    background->drawWithin (g, juce::Rectangle<float> (0, 0, getWidth(), getHeight()),
                           RectanglePlacement::stretchToFit, 1.000f);

  const float sf = getDesktopScaleFactor();
  
  g.drawImage(panOverlay, 590.f * sf, 140.f * sf, 234.f * sf, 37.5f * sf, 0, (panOverlayFrame * 75), 468, 75);
}

void MainComponent::paintOverChildren (Graphics& g)
{
  if(greyOut) {
    g.fillAll (Colours::white.withAlpha(0.5f));
    g.setColour(Colours::black);
    g.fillRect(getBounds().withTrimmedBottom(724));
    g.fillRect(0, 0, 129, 85);
    g.fillRect(406, 44, 620, 41);
  }
}

void MainComponent::resized()
{
  panSlider->setBounds (612, 185, 170, 170);
//    bpf->setBounds (10, 100, 400, 400);
  distanceSlider->setBounds (559, 618, 262, 56);
  contentTypeComboBox->setBounds (129, 44, 277, 44);
  elevationSlider->setBounds (228, 404, 128, 262);
  inputTypeToggle->setBounds (200, 227, 140, 70);
  pannerTypeToggle->setBounds (624, 406, 140,70);
}

void MainComponent::sliderValueChanged (Slider* sliderThatWasMoved)
{
}

void MainComponent::comboBoxChanged (ComboBox* comboBoxThatHasChanged)
{
  showHidePanElements();
}

void MainComponent::buttonClicked (Button* buttonThatWasClicked)
{
  showHidePanElements();
}

void MainComponent::showHidePanElements()
{
  panSlider->setVisible(false);
  elevationSlider->setVisible(false);

  bool inputType = inputTypeToggle->getToggleState();
  bool pannerType = pannerTypeToggle->getToggleState();
  bool monoTrack = processor.getTotalNumInputChannels() == 1;
  
  if((EContentType) contentTypeComboBox->getSelectedItemIndex() != kMusic)
  {
    if(inputType == kMono && pannerType == kBinaural) {
      elevationSlider->setVisible(true);
    }
    
    panSlider->setVisible(true);
    
    if(pannerType == kBinaural)
    {
      if(inputType == kMono || monoTrack)
        panOverlayFrame = 2;
      else if(inputType == kStereo)
        panOverlayFrame = 3;
    }
    else
    {
      panOverlayFrame = 1;
    }
    pannerTypeToggle->setVisible(true);
  }
  else
  {
    pannerTypeToggle->setVisible(false);
    panOverlayFrame = 0;
  }
  
  repaint();
}

void MainComponent::timerCallback()
{
  if(contentTypeComboBox->isPopupActive())
  {
    if(!greyOut)
    {
      greyOut = true;
      repaint();
    }
  }
  else
  {
    if(greyOut)
    {
      greyOut = false;
      repaint();
    }
  }
}
