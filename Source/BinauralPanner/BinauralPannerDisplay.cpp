/*
  ==============================================================================

    BinauralPannerDisplay.cpp
    Author:  Oliver Larkin

  ==============================================================================
*/

#include "BinauralPannerDisplay.h"

#include "BinauralPannerDisplay.h"
#include "../PluginEditor.h"


BinauralPannerDisplay::BinauralPannerDisplay(SpatialPodcastAudioProcessor& p)
: processor(p)
, mSourcePos({1.0, 0.0, 0.0})
{
//  mSourceImage = ImageFileFormat::loadFrom(source_icon_png, source_icon_png_size);
//  mHeadImage = ImageFileFormat::loadFrom(head_top_png, head_top_png_size);
}

BinauralPannerDisplay::~BinauralPannerDisplay()
{
}

void BinauralPannerDisplay::paint(Graphics& g)
{
  g.fillAll(Colours::transparentBlack);
  
  g.setOpacity(1.0f);
  g.setColour(Colours::black);
  auto w = getWidth();
  auto h = getHeight();
  g.fillEllipse(0.f, 0.f, static_cast<float>(w), static_cast<float>(h));
  
  if (mSourcePos.elevation < 0)
    drawSource(g);
  g.setOpacity(1.f);
  g.drawImageWithin(mHeadImage, 0, 0, w, h, RectanglePlacement::centred | RectanglePlacement::doNotResize);
  drawGridLines(g);
  if (mSourcePos.elevation >= 0)
    drawSource(g);
}

void BinauralPannerDisplay::mouseDrag(const MouseEvent& event)
{
  auto pos = event.getPosition();
  auto x = pos.x - getWidth() * 0.5f;
  auto y = pos.y - getHeight() * 0.5f;
  mSourcePos.azimuth = std::atan2(x, -y);
  updateHRTF();
}

void BinauralPannerDisplay::drawGridLines(Graphics& g)
{
  g.setColour(Colours::white);
  g.setOpacity(0.5f);
  auto w = static_cast<float>(getWidth());
  auto h = static_cast<float>(getHeight());
  auto lineHorizontal = Line<float>(0.f, h * 0.5f, w, h * 0.5f);
  auto lineVertical = Line<float>(w * 0.5f, 0.f, w * 0.5f, h);
  float dashes[] = {3, 2};
  g.drawDashedLine(lineHorizontal, dashes, 2);
  g.drawDashedLine(lineVertical, dashes, 2);
}

void BinauralPannerDisplay::drawSource(Graphics& g)
{
  auto w = static_cast<float>(getWidth());
  auto h = static_cast<float>(getHeight());
  auto radius = w * 0.5f - 30;
  auto x = radius * std::sin(mSourcePos.azimuth) * std::cos(mSourcePos.elevation);
  auto y = radius * std::cos(mSourcePos.azimuth) * std::cos(mSourcePos.elevation);
  x = w * 0.5f + x;
  y = h * 0.5f - y;
  auto color = Colours::white;
  if (mSourcePos.elevation < 0)
  {
    color = color.darker();
    g.setOpacity(0.8f);
  }
  ColourGradient grad(color, x, y, Colours::transparentBlack, x + w * 0.25f, y + h * 0.25f, true);
  g.setGradientFill(grad);
  g.fillEllipse(0.f, 0.f, w, h);
  auto scaleFactor = 0.75f * (std::sin(mSourcePos.elevation) * 0.25f + 1);
  auto sw = mSourceImage.getWidth();
  auto sh = mSourceImage.getHeight();
  g.drawImageWithin(mSourceImage,
                    static_cast<int>(x - sw * 0.5f * scaleFactor),
                    static_cast<int>(y - sh * 0.5f * scaleFactor),
                    static_cast<int>(sw * scaleFactor),
                    static_cast<int>(sh * scaleFactor),
                    RectanglePlacement::centred,
                    true);
}

void BinauralPannerDisplay::updateHRTF()
{
  processor.mAPVTS.getParameter("Azimuth")->setValueNotifyingHost((rad2deg(mSourcePos.azimuth) + 180)/360.);
  processor.mAPVTS.getParameter("Elevation")->setValueNotifyingHost((rad2deg(mSourcePos.elevation) + 90.)/180.);
  repaint();
}
