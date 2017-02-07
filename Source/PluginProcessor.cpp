/*
  ==============================================================================
  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

SpatialPodcastAudioProcessor::SpatialPodcastAudioProcessor()
: mAPVTS(*this, nullptr)
{
  auto contentTypeTextFormatter = [](float val)->String
  {
    return ContentTypes[int(floorf(val))];
  };
  
  auto pannerTypeTextFormatter = [](float val)->String
  {
    switch (static_cast<int>(floorf(val))) {
      case kBinaural: return "Out of head (Binaural)";
      case kSqrt: return "In head";
      default: return "";
    }
  };
  
  auto inputTypeTextFormatter = [](float val)->String
  {
    switch (static_cast<int>(floorf(val))) {
      case 0: return "Mono";
      case 1: return "Stereo";
      default: return "";
    }
  };
  
  mAPVTS.createAndAddParameter("ContentType", "Content Type", "", NormalisableRange<float>(0.f, 4.f), 0.f, contentTypeTextFormatter, nullptr);
  mAPVTS.createAndAddParameter("PannerType", "Panner Type", "", NormalisableRange<float>(0.f, 1.f), 0.f, pannerTypeTextFormatter, nullptr);
  mAPVTS.createAndAddParameter("InputType", "Input Type", "", NormalisableRange<float>(0.f, 1.f), 0.f, inputTypeTextFormatter, nullptr);
  mAPVTS.createAndAddParameter("Pan", "Pan", "", NormalisableRange<float>(-1.f, 1.f), 0.f, nullptr, nullptr);
  mAPVTS.createAndAddParameter("Elevation", "Elevation", "", NormalisableRange<float>(-90.f, 90.f), 0.f, nullptr, nullptr);
  mAPVTS.createAndAddParameter("Distance", "Distance", "", NormalisableRange<float>(0.f, 1.f), 0.f, nullptr, nullptr);
  
  mAPVTS.state = ValueTree("SpatialPodcast");
  mDistanceToDryMapping = ValueTree("DistanceToDryMapping");
  mDistanceToWetMapping = ValueTree("DistanceToWetMapping");
  mDistanceToFilteredMapping = ValueTree("DistanceToFilteredMapping");

  mDistanceToDryBPF = new BreakPointFunction(mDistanceToDryMapping, NormalisableRange<float>(0., 1., 0., 1.), NormalisableRange<float>(-70., 0., 1., 1.), "", "dB");
  mDistanceToWetBPF = new BreakPointFunction(mDistanceToWetMapping, NormalisableRange<float>(0., 1., 0., 1.), NormalisableRange<float>(-70., 0., 1., 1.), "", "dB");
  mDistanceToFilteredBPF = new BreakPointFunction(mDistanceToFilteredMapping);

  mAPVTS.addParameterListener("ContentType", this);

  //narration defaults
  mConvolutionReverb.loadNewIRFromMemory(BinaryData::koli_summer_site1_4way_mono_wav, BinaryData::koli_summer_site1_4way_mono_wavSize);
  mFilter.setFrequency(100.f);
  mFilter.setQ(0.5f);
  mFilter.setBellGain(1.f);
  mFilter.setType(TrapezoidalSVF::kLow);
  mMonoBinauralPanner.setCrossoverFreq(150.f);
  mStereoBinauralPanner.setCrossoverFreq(150.f);
  
  mDistanceToDryBPF->clearInitState();
  mDistanceToDryBPF->addPointToInitState(0., 0., 0, 1);
  mDistanceToDryBPF->addPointToInitState(0.26, -16.6, 0, 1);
  mDistanceToDryBPF->addPointToInitState(0.47, -38.3, 0, 1);
  mDistanceToDryBPF->addPointToInitState(0.72, -70, 0, 1);
  mDistanceToDryBPF->addPointToInitState(1., -70, 0, 1);
  mDistanceToDryBPF->initialize();
  
  mDistanceToWetBPF->clearInitState();
  mDistanceToWetBPF->addPointToInitState(0., -70., 0, 1);
  mDistanceToWetBPF->addPointToInitState(0.34, -46., 0, 1);
  mDistanceToWetBPF->addPointToInitState(0.60, -36.8, 0, 1);
  mDistanceToWetBPF->addPointToInitState(0.85, -33.1, 0, 1);
  mDistanceToWetBPF->addPointToInitState(1., -33.1, 0, 1);
  mDistanceToWetBPF->initialize();
  
  mDistanceToFilteredBPF->clearInitState();
  mDistanceToFilteredBPF->addPointToInitState(0., 0., 0, 1);
  mDistanceToFilteredBPF->addPointToInitState(1., 1., 0, 1);
  mDistanceToFilteredBPF->initialize();
}

SpatialPodcastAudioProcessor::~SpatialPodcastAudioProcessor()
{
}

void SpatialPodcastAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
  mConvolutionReverb.prepareToPlay(sampleRate, samplesPerBlock);
  mMonoBinauralPanner.prepareToPlay(sampleRate, samplesPerBlock);
  mStereoBinauralPanner.prepareToPlay(sampleRate, samplesPerBlock);
  mFilter.prepareToPlay(sampleRate, samplesPerBlock);
  mPanner.prepareToPlay(sampleRate, samplesPerBlock);
}

void SpatialPodcastAudioProcessor::releaseResources()
{
}

//bool SpatialPodcastAudioProcessor::setPreferredBusArrangement (bool isInput, int bus, const AudioChannelSet& preferredSet)
//{
//  const int numChannels = preferredSet.size();
//
//  if(!isInput && numChannels == 1)
//    return false;
//  
//  if (numChannels != 1 && numChannels != 2)
//      return false;
//
//  if (! AudioProcessor::setPreferredBusArrangement (!isInput, bus, preferredSet))
//      return false;
//
//  return AudioProcessor::setPreferredBusArrangement (isInput, bus, preferredSet);
//}

void SpatialPodcastAudioProcessor::processBlock (AudioSampleBuffer& buffer, MidiBuffer& midiMessages)
{
  //normalized
  const float distance = mAPVTS.getParameter("Distance")->getValue();
  const float pan = mAPVTS.getParameter("Pan")->getValue();

  const float elevation = (float) *mAPVTS.getRawParameterValue("Elevation");
  const int pannerType = (int) *mAPVTS.getRawParameterValue("PannerType");

  const int inputType = mAPVTS.getParameter("InputType")->getValue();
  const EContentType contentType = (EContentType) (int) *mAPVTS.getRawParameterValue("ContentType");

  mMonoBinauralPanner.setAzimuth((pan * 360.f)-180.f);
  mMonoBinauralPanner.setElevation(elevation);
  mStereoBinauralPanner.setWidth(pan * 180.f);
  mStereoBinauralPanner.setElevation(elevation);
  
  if(contentType != kMusic)
  {
    if(pannerType == kBinaural)
    {
      if(inputType == kMono || getTotalNumInputChannels() == 1)
        mMonoBinauralPanner.processBlock(buffer);
      else
        mStereoBinauralPanner.processBlock(buffer);
    }
    else if(pannerType == kSqrt)
    {
      mPanner.setPanPos(pan);
      
      if(inputType == kMono || getTotalNumInputChannels() == 1)
        mPanner.processBlock(buffer);
      else
        mPanner.processBlockBalance(buffer);
    }
  }
  
  //TODO: !! accessing the BPFs here is not thread safe, because it is accessing a valuetree
  mFilter.setMix(mDistanceToFilteredBPF->getYforX(distance));
  mFilter.processBlock(buffer);
  mConvolutionReverb.setDryLevel(mDistanceToDryBPF->getYforX(distance));
  mConvolutionReverb.setWetLevel(mDistanceToWetBPF->getYforX(distance));
  mConvolutionReverb.processBlock(buffer, midiMessages);
}

AudioProcessorEditor* SpatialPodcastAudioProcessor::createEditor()
{
  //valueTreeDebugger = new ValueTreeDebugger(mAPVTS.state);

  return new SpatialPodcastAudioProcessorEditor(*this);
}

void SpatialPodcastAudioProcessor::getStateInformation (MemoryBlock& destData)
{
  ScopedPointer<XmlElement> xml(mAPVTS.state.createXml());
  
  if(xml)
    copyXmlToBinary (*xml, destData);
}

void SpatialPodcastAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
  ScopedPointer<XmlElement> xmlState (getXmlFromBinary (data, sizeInBytes));
  mAPVTS.state = ValueTree::fromXml(*xmlState);
}

void SpatialPodcastAudioProcessor::parameterChanged (const String& parameterID, float newValue)
{
  if(parameterID == "ContentType")
  {
    const EContentType contentType = (EContentType) (int) *mAPVTS.getRawParameterValue("ContentType");

    switch (contentType) {
      case kNarration:
        mConvolutionReverb.loadNewIRFromMemoryAsync(BinaryData::koli_summer_site1_4way_mono_wav, BinaryData::koli_summer_site1_4way_mono_wavSize);
        mFilter.setFrequency(100.f);
        mFilter.setQ(0.5f);
        mFilter.setBellGain(1.f);
        mFilter.setType(TrapezoidalSVF::kLow);
        mMonoBinauralPanner.setCrossoverFreq(150.f);
        mStereoBinauralPanner.setCrossoverFreq(150.f);
        break;
      case kMusic:
        mConvolutionReverb.loadNewIRFromMemoryAsync(BinaryData::perth_city_hall_balcony_ir_edit_wav, BinaryData::perth_city_hall_balcony_ir_edit_wavSize);
        mFilter.setFrequency(150.f);
        mFilter.setQ(0.5f);
        mFilter.setBellGain(0.f);
        mFilter.setType(TrapezoidalSVF::kLow);
        mMonoBinauralPanner.setCrossoverFreq(938.f);
        mStereoBinauralPanner.setCrossoverFreq(938.f);
        break;
      case kDialogue:
        mConvolutionReverb.loadNewIRFromMemoryAsync(BinaryData::ir1_wav, BinaryData::ir1_wavSize);
        mFilter.setFrequency(100.f);
        mFilter.setQ(0.46f);
        mFilter.setBellGain(0.f);
        mFilter.setType(TrapezoidalSVF::kLow);
        mMonoBinauralPanner.setCrossoverFreq(559.5f);
        mStereoBinauralPanner.setCrossoverFreq(559.5f);
        break;
      case kAtmosphere:
        mConvolutionReverb.loadNewIRFromMemoryAsync(BinaryData::ir1_wav, BinaryData::ir1_wavSize);
        mFilter.setFrequency(150.f);
        mFilter.setQ(0.98f);
        mFilter.setBellGain(0.f);
        mFilter.setType(TrapezoidalSVF::kLow);
        mMonoBinauralPanner.setCrossoverFreq(237.6f);
        mStereoBinauralPanner.setCrossoverFreq(237.6f);
        break;
      case kSFX:
        mConvolutionReverb.loadNewIRFromMemoryAsync(BinaryData::ir1_wav, BinaryData::ir1_wavSize);
        mFilter.setFrequency(200.f);
        mFilter.setQ(0.7f);
        mFilter.setBellGain(0.f);
        mFilter.setType(TrapezoidalSVF::kLow);
        mMonoBinauralPanner.setCrossoverFreq(122.f);
        mStereoBinauralPanner.setCrossoverFreq(122.f);
        break;
      default:
        mConvolutionReverb.loadNewIRFromMemoryAsync(BinaryData::ir1_wav, BinaryData::ir1_wavSize);
        break;
    }
    
    
    switch (contentType) {
      case kNarration:
        
        mDistanceToDryBPF->clearInitState();
        mDistanceToDryBPF->addPointToInitState(0., 0., 0, 1);
        mDistanceToDryBPF->addPointToInitState(0.26, -16.6, 0, 1);
        mDistanceToDryBPF->addPointToInitState(0.47, -38.3, 0, 1);
        mDistanceToDryBPF->addPointToInitState(0.72, -70, 0, 1);
        mDistanceToDryBPF->addPointToInitState(1., -70, 0, 1);
        mDistanceToDryBPF->initialize();
        
        mDistanceToWetBPF->clearInitState();
        mDistanceToWetBPF->addPointToInitState(0., -70., 0, 1);
        mDistanceToWetBPF->addPointToInitState(0.34, -46., 0, 1);
        mDistanceToWetBPF->addPointToInitState(0.60, -36.8, 0, 1);
        mDistanceToWetBPF->addPointToInitState(0.85, -33.1, 0, 1);
        mDistanceToWetBPF->addPointToInitState(1., -33.1, 0, 1);
        mDistanceToWetBPF->initialize();
        
        mDistanceToFilteredBPF->clearInitState();
        mDistanceToFilteredBPF->addPointToInitState(0., 0., 0, 1);
        mDistanceToFilteredBPF->addPointToInitState(1., 1., 0, 1);
        mDistanceToFilteredBPF->initialize();
        
        break;
        
      case kDialogue:
        
        mDistanceToDryBPF->clearInitState();
        mDistanceToDryBPF->addPointToInitState(0., 0., 0, 1);
        mDistanceToDryBPF->addPointToInitState(0.2, -2.6, 0, 1);
        mDistanceToDryBPF->addPointToInitState(0.56, -22., 0, 1);
        mDistanceToDryBPF->addPointToInitState(0.75, -51., 0, 1);
        mDistanceToDryBPF->addPointToInitState(1., -70., 0, 1);
        mDistanceToDryBPF->initialize();
        
        mDistanceToWetBPF->clearInitState();
        mDistanceToWetBPF->addPointToInitState(0., -70., 0, 1);
        mDistanceToWetBPF->addPointToInitState(0.24, -53.05, 0, 1);
        mDistanceToWetBPF->addPointToInitState(0.59, -35.36, 0, 1);
        mDistanceToWetBPF->addPointToInitState(0.75, -32.79, 0, 1);
        mDistanceToWetBPF->addPointToInitState(1., -30.9, 0, 1);
        mDistanceToWetBPF->initialize();
        
        mDistanceToFilteredBPF->clearInitState();
        mDistanceToFilteredBPF->addPointToInitState(0., 0., 0, 1);
        mDistanceToFilteredBPF->addPointToInitState(1., 1., 0, 1);
        mDistanceToFilteredBPF->initialize();
        
        break;
        
      case kMusic:
        mDistanceToDryBPF->clearInitState();
        mDistanceToDryBPF->addPointToInitState(0., 0., 0, 1);
        mDistanceToDryBPF->addPointToInitState(0.36, -23., 0, 0);
        mDistanceToDryBPF->addPointToInitState(0.57, -43., 0, 0);
        mDistanceToDryBPF->addPointToInitState(0.72, -59., 0, 0);
        mDistanceToDryBPF->addPointToInitState(0.87, -67., 0, 0);
        mDistanceToDryBPF->addPointToInitState(1., -70., 0, 1);
        mDistanceToDryBPF->initialize();
        
        mDistanceToWetBPF->clearInitState();
        mDistanceToWetBPF->addPointToInitState(0., -70., 0, 1);
        mDistanceToWetBPF->addPointToInitState(0.025, -70., 0, 0);
        mDistanceToWetBPF->addPointToInitState(0.19, -56., 0, 0);
        mDistanceToWetBPF->addPointToInitState(0.32, -48., 0, 0);
        mDistanceToWetBPF->addPointToInitState(0.62, -41., 0, 0);
        mDistanceToWetBPF->addPointToInitState(1., -41., 0, 1);
        mDistanceToWetBPF->initialize();
        
        mDistanceToFilteredBPF->clearInitState();
        mDistanceToFilteredBPF->addPointToInitState(0., 0., 0, 1);
        mDistanceToFilteredBPF->addPointToInitState(0.37, 0.49, 0, 0);
        mDistanceToFilteredBPF->addPointToInitState(0.59, 0.73, 0, 0);
        mDistanceToFilteredBPF->addPointToInitState(0.89, 0.89, 0, 0);
        mDistanceToFilteredBPF->addPointToInitState(1., 0.9, 0, 1);
        mDistanceToFilteredBPF->initialize();
        
        break;
      case kAtmosphere:
        
        mDistanceToDryBPF->clearInitState();
        mDistanceToDryBPF->addPointToInitState(0., 0., 0, 1);
        mDistanceToDryBPF->addPointToInitState(0.2, -2.6, 0, 0);
        mDistanceToDryBPF->addPointToInitState(0.56, -21., 0, 0);
        mDistanceToDryBPF->addPointToInitState(0.75, -51.6, 0, 0);
        mDistanceToDryBPF->addPointToInitState(0.9, -70., 0, 0);
        mDistanceToDryBPF->addPointToInitState(1., -70., 0, 1);
        mDistanceToDryBPF->initialize();
        
        mDistanceToWetBPF->clearInitState();
        mDistanceToWetBPF->addPointToInitState(0., -70., 0, 1);
        mDistanceToWetBPF->addPointToInitState(0.25, -53., 0, 1);
        mDistanceToWetBPF->addPointToInitState(0.55, -27.7, 0, 1);
        mDistanceToWetBPF->addPointToInitState(0.66, -24, 0, 1);
        mDistanceToWetBPF->addPointToInitState(0.84, -21, 0, 1);
        mDistanceToWetBPF->addPointToInitState(1., -21.4, 0, 1);
        mDistanceToWetBPF->initialize();
        
        mDistanceToFilteredBPF->clearInitState();
        mDistanceToFilteredBPF->addPointToInitState(0., 0., 0, 1);
        mDistanceToFilteredBPF->addPointToInitState(1., 0.85, 0, 1);
        mDistanceToFilteredBPF->initialize();
        
        break;
      case kSFX:
        
        mDistanceToDryBPF->clearInitState();
        mDistanceToDryBPF->addPointToInitState(0., 0., 0, 1);
        mDistanceToDryBPF->addPointToInitState(0.2, -2.6, 0, 1);
        mDistanceToDryBPF->addPointToInitState(0.66, -21., 0, 1);
        mDistanceToDryBPF->addPointToInitState(0.81, -40., 0, 1);
        mDistanceToDryBPF->addPointToInitState(0.96, -70., 0, 1);
        mDistanceToDryBPF->addPointToInitState(1., -70., 0, 1);
        mDistanceToDryBPF->initialize();
        
        mDistanceToWetBPF->clearInitState();
        mDistanceToWetBPF->addPointToInitState(0., -70., 0, 1);
        mDistanceToWetBPF->addPointToInitState(0.2, -48., 0, 1);
        mDistanceToWetBPF->addPointToInitState(0.51, -30., 0, 1);
        mDistanceToWetBPF->addPointToInitState(0.77, -25., 0, 1);
        mDistanceToWetBPF->addPointToInitState(1., -27., 0, 1);
        mDistanceToWetBPF->initialize();
        
        mDistanceToFilteredBPF->clearInitState();
        mDistanceToFilteredBPF->addPointToInitState(0., 0., 0, 1);
        mDistanceToFilteredBPF->addPointToInitState(1., 0.9, 0, 1);
        mDistanceToFilteredBPF->initialize();
        
        break;
      default:
        break;
    }
  }
}


AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
  return new SpatialPodcastAudioProcessor();
}
