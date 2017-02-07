/*
  ==============================================================================

    LookAndFeel.h
    Created: 16 Oct 2016 10:29:34pm
    Author:  Oliver Larkin

  ==============================================================================
*/

#ifndef LOOKANDFEEL_H_INCLUDED
#define LOOKANDFEEL_H_INCLUDED

class SpatialPodcastLookAndFeel : public LookAndFeel_V3
{
public:
  SpatialPodcastLookAndFeel()
  {
    typeFace = Typeface::createSystemTypefaceFor(BinaryData::ComfortaaRegular_ttf, BinaryData::ComfortaaRegular_ttfSize);
  }
  
  Typeface::Ptr getTypefaceForFont(const Font&f) override
  {
    return typeFace;
  }
  
  Font getPopupMenuFont() override
  {
    Font f;
    f.setHeight(24);
    
    return f;
  }
  
  Font getComboBoxFont (ComboBox& box) override
  {
    Font f;
    f.setHeight(24);
    
    return f;
  }
  
  void drawComboBox (Graphics& g, int width, int height, const bool /*isButtonDown*/,
                int buttonX, int buttonY, int buttonW, int buttonH, ComboBox& box) override
  {
    g.fillAll (box.findColour (ComboBox::backgroundColourId));

    const float arrowX = 0.3f;
    const float arrowH = 0.2f;
    
    Path p;
    
    p.addTriangle (buttonX + buttonW * 0.5f,            buttonY + buttonH * (0.4f + arrowH),
                   buttonX + buttonW * (1.0f - arrowX), buttonY + buttonH * 0.4f,
                   buttonX + buttonW * arrowX,          buttonY + buttonH * 0.4f);
    
    g.setColour (Colours::black);
    g.fillPath (p);
  }
  
  void drawPopupMenuBackground (Graphics& g, int width, int height) override
  {
    g.fillAll(Colours::black);
  }

  void drawPopupMenuItem (Graphics& g, const juce::Rectangle<int>& area,
                          bool isSeparator, bool isActive, bool isHighlighted,
                          bool isTicked, bool hasSubMenu,
                          const String& text,
                          const String& shortcutKeyText,
                          const Drawable* icon,
                          const Colour* textColourToUse) override
  {
    if (isSeparator)
    {
      juce::Rectangle<int> r (area.reduced (5, 0));
      r.removeFromTop (r.getHeight() / 2 - 1);
      
      g.setColour (Colour (0x33000000));
      g.fillRect (r.removeFromTop (1));
      
      g.setColour (Colour (0x66ffffff));
      g.fillRect (r.removeFromTop (1));
    }
    else
    {
      Colour textColour (Colours::white);
      
      if (textColourToUse != nullptr)
        textColour = *textColourToUse;
      
      juce::Rectangle<int> r (area.reduced (1));
      
      if (isHighlighted)
      {
        g.setColour (findColour (PopupMenu::highlightedBackgroundColourId));
        g.fillRect (r);
        
        g.setColour (findColour (PopupMenu::highlightedTextColourId));
      }
      else
      {
        g.setColour (textColour);
      }
      
      if (! isActive)
        g.setOpacity (0.3f);
      
      Font font (getPopupMenuFont());
      
      const float maxFontHeight = area.getHeight() / 1.3f;
      
      if (font.getHeight() > maxFontHeight)
        font.setHeight (maxFontHeight);
      
      g.setFont (font);
      
      juce::Rectangle<float> iconArea (r.removeFromLeft ((r.getHeight() * 5) / 4).reduced (3).toFloat());
      
      if (icon != nullptr)
      {
        icon->drawWithin (g, iconArea, RectanglePlacement::centred | RectanglePlacement::onlyReduceInSize, 1.0f);
      }
      else if (isTicked)
      {
        const Path tick (getTickShape (1.0f));
        g.fillPath (tick, tick.getTransformToScaleToFit (iconArea, true));
      }
      
      if (hasSubMenu)
      {
        const float arrowH = 0.6f * getPopupMenuFont().getAscent();
        
        const float x = (float) r.removeFromRight ((int) arrowH).getX();
        const float halfH = (float) r.getCentreY();
        
        Path p;
        p.addTriangle (x, halfH - arrowH * 0.5f,
                       x, halfH + arrowH * 0.5f,
                       x + arrowH * 0.6f, halfH);
        
        g.fillPath (p);
      }
      
      r.removeFromRight (3);
      g.drawFittedText (text, r, Justification::centredLeft, 1);
      
      if (shortcutKeyText.isNotEmpty())
      {
        Font f2 (font);
        f2.setHeight (f2.getHeight() * 0.75f);
        f2.setHorizontalScale (0.95f);
        g.setFont (f2);
        
        g.drawText (shortcutKeyText, r, Justification::centredRight, true);
      }
    }
  }
  
  void drawRotarySlider (Graphics& g, int x, int y, int width, int height, float sliderPos,
                         const float rotaryStartAngle, const float rotaryEndAngle, Slider& slider) override
  {
    const float radius = jmin (width / 2, height / 2) - 2.0f;
    const float centreX = x + width * 0.5f;
    const float centreY = y + height * 0.5f;
    
    const float angle = rotaryStartAngle + sliderPos * (rotaryEndAngle - rotaryStartAngle);
    const bool isMouseOver = slider.isMouseOverOrDragging() && slider.isEnabled();
    
    const float centreSize = 0.5f;
    const float innerWidth = (width * centreSize);
    const float innerHeight = (height * centreSize);

    const float innerX = centreX - (radius * 0.5f);
    const float innerY = centreY - (radius * 0.5f);
    
    
    const float thickness = 0.7f;
    
    {
      g.setColour(Colours::black);

      const float innerRadius = radius * 0.2f;
      Path p;
      p.addTriangle (-innerRadius, -40.0f,
                     0.0f, -radius * thickness * 1.1f,
                     innerRadius, -40.0f);
      
      g.fillPath (p, AffineTransform::rotation (angle).translated (centreX, centreY));
    }

    g.setColour(Colours::white);
    g.fillEllipse(innerX, innerY, width * centreSize, height * centreSize);
    g.setColour(Colours::black);
    g.drawEllipse(innerX + 4, innerY + 4, innerWidth - 8, innerHeight - 8, 7);
    
    if(isMouseOver)
      g.drawEllipse(innerX + 6, innerY + 6, innerWidth - 12, innerHeight - 12, 7);
      
    g.setColour(Colours::black);
    g.fillEllipse(innerX + 16, innerY + 16, innerWidth - 32, innerHeight - 32);
  }
  
  void drawToggleButton (Graphics& g, ToggleButton& button,
                         bool isMouseOverButton, bool isButtonDown) override
  {
    int xoffs = 0;
    int width = button.getWidth() / 2;
    
    if (button.getToggleState())
      xoffs += width;
    
    g.setColour(Colours::white);
    g.fillEllipse(xoffs + 0, 0, width, button.getHeight());
    g.setColour(Colours::black);
    g.drawEllipse(xoffs + 4, 4, width-8, button.getHeight()-8, 7);
    
    if(isMouseOverButton)
      g.drawEllipse(xoffs + 6, 6, width-12, button.getHeight()-12, 7);

    g.setColour(Colours::black);
    g.fillEllipse(xoffs + 16, 16, width - 32, button.getHeight() - 32);
  }
  
  void drawLinearSliderBackground (Graphics&, int x, int y, int width, int height,
                                   float sliderPos, float minSliderPos, float maxSliderPos,
                                   const Slider::SliderStyle, Slider&) override
  {
    
  }
  
  void drawLinearSliderThumb (Graphics& g, int x, int y, int width, int height,
                                              float sliderPos, float minSliderPos, float maxSliderPos,
                                              const Slider::SliderStyle style, Slider& slider) override
  {
    float sliderRadius = (float) (getSliderThumbRadius (slider));
    
    if (style == Slider::LinearHorizontal || style == Slider::LinearVertical)
    {
      float kx, ky;
      
      if (style == Slider::LinearVertical)
      {
        kx = x + width * 0.5f;
        ky = sliderPos;
      }
      else
      {
        kx = sliderPos;
        ky = y + height * 0.5f;
      }
      
      g.setColour(Colours::black);
      
      Path p;
      if(style == Slider::LinearHorizontal)
        p.addTriangle(kx - sliderRadius, ky + sliderRadius, kx, ky, kx + sliderRadius, ky + sliderRadius);
      else
        p.addTriangle(kx + sliderRadius, ky - sliderRadius, kx + sliderRadius, ky + sliderRadius, kx, ky);
        
      g.fillPath(p);
        
      if(slider.isMouseOverOrDragging())
      {
        Path p2;
        if(style == Slider::LinearHorizontal)
          p2.addTriangle(kx - sliderRadius, ky + sliderRadius, kx, ky, kx + sliderRadius, ky + sliderRadius);
        else
          p2.addTriangle(kx + sliderRadius, ky - sliderRadius, kx + sliderRadius, ky + sliderRadius, kx, ky);
            
        g.fillPath(p2);
      }
    }
  }
  
  int getSliderThumbRadius (Slider& s) override
  {
    return 15;
  }
  
private:
  Typeface::Ptr typeFace;
};




#endif  // LOOKANDFEEL_H_INCLUDED
