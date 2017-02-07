/*
  ==============================================================================

    BreakPointFunction.h
    Author:  Oliver Larkin

  ==============================================================================
*/

#ifndef BREAKPOINTFUNCTION_H_INCLUDED
#define BREAKPOINTFUNCTION_H_INCLUDED

#include "../JuceLibraryCode/JuceHeader.h"

class BreakPointFunction : public Component
                         , public ComponentListener
                         , public ValueTree::Listener
{
public:
  BreakPointFunction(ValueTree &state,
                     NormalisableRange<float> xrange = NormalisableRange<float>(0., 1., 0., 1.),
                     NormalisableRange<float> yrange = NormalisableRange<float>(0., 1., 0., 1.),
                     String xunits = String::empty,
                     String yunits = String::empty);
  
  ~BreakPointFunction();

public:
  
  class BreakPoint : public Component,
                     public ComponentDragger
  {
  public:
    enum Mode
    {
      kLinear = 0,
      kBezier,
      kNumModes
    };
    
    class BreakPointBoundsConstrainer : public juce::ComponentBoundsConstrainer
    {
    private:
      bool mEnableHorizontalDrag;
      bool mEnableVerticalDrag;
      
    public:
      BreakPointBoundsConstrainer(bool enableHorizontalDrag = true, bool enableVerticalDrag = true)
      : mEnableHorizontalDrag(enableHorizontalDrag)
      , mEnableVerticalDrag(enableVerticalDrag)
      {
      }
      
      void checkBounds (juce::Rectangle<int>& bounds,
                        const juce::Rectangle<int>& previousBounds,
                        const juce::Rectangle<int>& limits,
                        bool isStretchingTop,
                        bool isStretchingLeft,
                        bool isStretchingBottom,
                        bool isStretchingRight) override
      {
        
        if(!mEnableHorizontalDrag)
        {
          bounds.setX(previousBounds.getX());
          bounds.setWidth(previousBounds.getWidth());
        }
        
        if(!mEnableVerticalDrag)
        {
          bounds.setY(previousBounds.getY());
          bounds.setHeight(previousBounds.getHeight());
        }
        
        ComponentBoundsConstrainer::checkBounds(bounds, previousBounds, limits, isStretchingTop, isStretchingLeft, isStretchingBottom, isStretchingRight);
      }
      
      bool getHorizontalDragEnabled() { return mEnableHorizontalDrag; }
      bool getVerticalDragEnabled() { return mEnableVerticalDrag; }
    };
    
  private:
    BreakPointBoundsConstrainer mConstrainer;
    bool mMouseIsOver;
    Mode mMode;
    
  public:
    BreakPoint (bool enableHorizontalDrag = true, bool enableVerticalDrag = true)
    : mConstrainer(enableHorizontalDrag, enableVerticalDrag)
    , mMouseIsOver (false)
    , mMode(kLinear)
    {
    }
    
    void resized () override
    {
      mConstrainer.setMinimumOnscreenAmounts (getHeight(), getWidth(), getHeight(), getWidth());
    }
    
    void paint (Graphics& g) override
    {
      g.setColour (Colours::grey);
      g.fillAll();
      
      g.setColour (Colours::black);
      g.drawText(getComponentID(), 0, 0, getWidth(), getHeight(), Justification::centred);

      if (mMouseIsOver)
      {
        g.setColour (Colours::lightgrey);
        g.drawRect (0, 0, getWidth(), getHeight(), 2);
      }
    }
    
    void mouseDown (const MouseEvent& e) override;
    void mouseDrag (const MouseEvent& e) override { dragComponent (this, e, &mConstrainer); }
    void mouseEnter (const MouseEvent& e) override { mMouseIsOver = true; repaint(); }
    void mouseExit (const MouseEvent& e) override { mMouseIsOver = false; repaint(); }
    
    Point<float> getNormalizedPoint()
    {
      juce::Rectangle<int> bounds = getBounds();
      
      const int halfWidth = cBPSize/2;
            
      const float posX = static_cast<float>((bounds.getCentreX()-halfWidth)/static_cast<float>(getParentWidth()-cBPSize));
      const float posY = static_cast<float>((bounds.getCentreY()-halfWidth)/static_cast<float>(getParentHeight()-cBPSize));
      
      return Point<float>(posX, posY);
    }
    
    bool getHorizontalDragEnabled() { return mConstrainer.getHorizontalDragEnabled(); }
    bool getVerticalDragEnabled() { return mConstrainer.getVerticalDragEnabled(); }
  };
  
  void paint (Graphics&) override;
  void resized() override;

  void mouseDown (const MouseEvent& e) override;
  void mouseDrag (const MouseEvent& e) override;
  
  //ComponentListener
  void componentMovedOrResized (Component& component, bool wasMoved, bool wasResized) override;
  
  //ValueTree::Listener
  void valueTreePropertyChanged (ValueTree &treeWhosePropertyHasChanged, const juce::Identifier &property) override {}
  void valueTreeChildAdded (ValueTree &parentTree, ValueTree &childWhichHasBeenAdded) override {}
  void valueTreeChildRemoved (ValueTree &parentTree, ValueTree &childWhichHasBeenRemoved, int indexFromWhichChildWasRemoved) override {}
  void valueTreeChildOrderChanged (ValueTree &parentTreeWhoseChildrenHaveMoved, int oldIndex, int newIndex) override {}
  void valueTreeParentChanged (ValueTree &treeWhoseParentHasChanged) override {}
  void valueTreeRedirected (ValueTree &treeWhichHasBeenChanged) override
  {
    if(treeWhichHasBeenChanged.getType() == mState.getType())
    {
      mState = treeWhichHasBeenChanged;
    }
  }
  
  void updateState();
  void reloadFromState();
  Point<float> evaluatePathPos(const float normalizedPos);
  float getYforX(const float normalizedXPos);
  void initialize();
  void addPointToInitState(float x, float y, bool enableHorizontalDrag = true, bool enableVerticalDrag = true);
  void clearInitState() { mInitialPoints.clear(); }
  
  void deleteBreakPoint(StringRef componentID);
  
  void loadFromXMLString(const XmlElement& xml);
  
private:
  void addBreakPointFromClick(Point<float> pos);
  void refreshPath();
  
private:
  ValueTree mState;
  bool mNeedsResync;
  bool mInsertPointsSequentially;
  
  struct PointInfo
  {
    float normX;
    float normY;
    bool hDrag;
    bool vDrag;
    
    PointInfo(float x, float y, bool enableHorizontalDrag, bool enableVerticalDrag)
    : normX(x), normY(y), hDrag(enableHorizontalDrag), vDrag(enableVerticalDrag)
    {
    }
  };
  
  Array<PointInfo> mInitialPoints;
  
  Path mPath;
  
  OwnedArray<BreakPoint> mBreakPoints;

  Point<float> mLookupPos;
  NormalisableRange<float> mXRange;
  NormalisableRange<float> mYRange;
  String mXUnits, mYUnits;
  
  static constexpr int mMinimumNPoints = 2;
  static constexpr int cBPSize = 10;
  
  JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (BreakPointFunction)
};


#endif  // BREAKPOINTFUNCTION_H_INCLUDED
