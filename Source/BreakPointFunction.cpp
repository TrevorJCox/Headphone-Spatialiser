/*
  ==============================================================================

    BreakPointFunction.cpp
    Author:  Oliver Larkin

  ==============================================================================
*/

#include "../JuceLibraryCode/JuceHeader.h"
#include "BreakPointFunction.h"

BreakPointFunction::BreakPointFunction(ValueTree &state,
                                       NormalisableRange<float> xrange,
                                       NormalisableRange<float> yrange,
                                       String xunits,
                                       String yunits)
: mState(state)
, mNeedsResync(false)
, mInsertPointsSequentially(false)
, mXRange(xrange)
, mYRange(yrange)
, mXUnits(xunits)
, mYUnits(yunits)
{
  state.addListener(this);
}

BreakPointFunction::~BreakPointFunction()
{
}

void BreakPointFunction::paint (Graphics& g)
{
  if(mNeedsResync)
    reloadFromState();
  
  g.fillAll(Colours::white);
  g.setColour (Colours::black);
  g.strokePath (mPath, PathStrokeType (2.0f));
  
  g.setColour (Colours::red);
  
  for (int i = 0; i < mBreakPoints.size(); ++i)
  {
    Point<int> topRight = mBreakPoints[i]->getBounds().getTopRight();
    Point<float> normPoint = mBreakPoints[i]->getNormalizedPoint();
    int nonNormalizedValue = (int) mYRange.convertFrom0to1(1.-normPoint.getY());
    g.drawText(String(nonNormalizedValue) + mYUnits, topRight.getX() + 5, topRight.getY() + 5, 60, 10, Justification::left);
  }
  
  //g.fillEllipse(mLookupPos.getX() * (getWidth()-cBPSize), mLookupPos.getY() * (getHeight()-cBPSize), 10., 10.);
}

void BreakPointFunction::resized()
{
}

void BreakPointFunction::mouseDown (const MouseEvent& e)
{
  if (e.mods.isPopupMenu())
  {
    PopupMenu m;
    m.addItem(1, "Add Break Point");
    m.addItem(2, "Save XML");
    m.addItem(3, "Load XML");
    m.addItem(4, "Reset");
    
    int result = m.show();
    
    switch(result)
    {
      case 1:
      {
        const float posX = static_cast<float>(e.getMouseDownX()/static_cast<float>(getWidth()));
        const float posY = static_cast<float>(e.getMouseDownY()/static_cast<float>(getHeight()));
        Point<float> pos(posX, posY);
        addBreakPointFromClick(pos);
      }
      break;
      case 2:
      {
        FileChooser fc ("Save XML...",
                        File::getSpecialLocation(File::userDesktopDirectory),
                        "xml",
                        true);
        
        if (fc.browseForFileToSave(true))
        {
          File chosen = fc.getResults().getLast();

          ScopedPointer<XmlElement> xml = mState.createXml();

          if(xml)
            xml->writeToFile(chosen, "");
        }
      }
      break;
      case 3:
      {
        FileChooser fc ("Load XML...",
                        File::getSpecialLocation(File::userDesktopDirectory),
                        "*.xml",
                        true);
        
        if (fc.browseForFileToOpen())
        {
          File chosen = fc.getResults().getLast();
          
          ScopedPointer<XmlElement> xml = XmlDocument(chosen).getDocumentElement();
          mState = ValueTree::fromXml(*xml);
          
          reloadFromState();
        }
      }
      break;
      case 4:
        initialize();
        break;
      default:
        break;
    }
  }
}

void BreakPointFunction::mouseDrag (const MouseEvent& e)
{
}

void BreakPointFunction::componentMovedOrResized (Component& component, bool wasMoved, bool wasResized)
{
  updateState();
  refreshPath();
}

void BreakPointFunction::addBreakPointFromClick(Point<float> pos)
{
  BreakPoint* newBP = new BreakPoint();

  if(mInsertPointsSequentially)
  {
    newBP->setComponentID(String(mBreakPoints.size()));
    mBreakPoints.add (newBP);
  }
  else
  {
    int numPoints = mBreakPoints.size();
    // work out points to the left and right of the x position
    int pointIndex = numPoints;
    int lowPointIndex = numPoints-1;
    
    while(pointIndex--)
    {
      float bpNormalizedXPosition = mBreakPoints[pointIndex]->getNormalizedPoint().getX();
      
      if(pos.getX() <= bpNormalizedXPosition)
        lowPointIndex--;
    }
    
    mBreakPoints.insert(lowPointIndex+1, newBP);
    
    for (int bpIdx = 0; bpIdx < mBreakPoints.size(); bpIdx++)
    {
      mBreakPoints[bpIdx]->setComponentID(String(bpIdx));
    }
  }
  
  newBP->addComponentListener (this);
  newBP->setBounds (pos.getX() * (getWidth()-cBPSize), pos.getY() * (getHeight()-cBPSize), cBPSize, cBPSize);
  addAndMakeVisible (newBP);
  
  updateState();
}

void BreakPointFunction::refreshPath()
{
  if(!mBreakPoints.size())
    return;
  
  mPath.clear();
  
  Rectangle<int> bounds = mBreakPoints[0]->getBounds();

  mPath.startNewSubPath (bounds.getCentreX(), bounds.getCentreY());
  
  for (int i = 1; i < mBreakPoints.size(); ++i)
  {
    Rectangle<int> bounds = mBreakPoints[i]->getBounds();
    mPath.lineTo (bounds.getCentreX(), bounds.getCentreY());
  }
  
  repaint();
}

void BreakPointFunction::updateState()
{
  for (int bpIdx = 0; bpIdx < mBreakPoints.size(); bpIdx++)
  {
    Point<float> normalizedPosition = mBreakPoints[bpIdx]->getNormalizedPoint();
    ValueTree child = mState.getOrCreateChildWithName(Identifier(String("bp_") + String(bpIdx)), nullptr);
    child.setProperty("x", mXRange.convertFrom0to1(normalizedPosition.getX()), nullptr);
    child.setProperty("y", mYRange.convertFrom0to1(1.f-normalizedPosition.getY()), nullptr); // inverted because top left is 0,0 in coordinate space
    child.setProperty("hdrag", mBreakPoints[bpIdx]->getHorizontalDragEnabled(), nullptr);
    child.setProperty("vdrag", mBreakPoints[bpIdx]->getVerticalDragEnabled(), nullptr);
  }
}

void BreakPointFunction::reloadFromState()
{
  removeAllChildren();
  mBreakPoints.clear();
  
  for (int chIdx = 0; chIdx < mState.getNumChildren(); chIdx++)
  {
    ValueTree child = mState.getChild(chIdx);
    float normalizedX = mXRange.convertTo0to1(child.getProperty("x"));
    float normalizedY = 1.f - mYRange.convertTo0to1(child.getProperty("y"));  // inverted because top left is 0,0 in coordinate space
    
    BreakPoint* newBP = new BreakPoint(child.getProperty("hdrag"), child.getProperty("vdrag"));
    newBP->addComponentListener (this);
    newBP->setBounds (normalizedX * (getWidth()-cBPSize), normalizedY * (getHeight()-cBPSize), cBPSize, cBPSize);
    newBP->setComponentID(String(chIdx));
    mBreakPoints.add(newBP);
    addAndMakeVisible(newBP);
  }
  
  refreshPath();
  
  mNeedsResync = false;
}

Point<float> BreakPointFunction::evaluatePathPos(const float normalizedPos)
{
  const float rangePos = normalizedPos * static_cast<float>(mBreakPoints.size()-1);
  const int intPart = static_cast<int>(truncf(rangePos));
  const float floatPart = rangePos - intPart;

  if(intPart >= mBreakPoints.size()-1)
    return Point<float>(0.f,0.f);
  
  const Point<float> pointA = mBreakPoints[intPart]->getNormalizedPoint();
  const Point<float> pointB = mBreakPoints[intPart+1]->getNormalizedPoint();
  
  const float x = (pointB.getX() * floatPart) + (pointA.getX() * (1.f-floatPart));
  const float y = (pointB.getY() * floatPart) + (pointA.getY() * (1.f-floatPart));

  return Point<float>(x,y);
}

float BreakPointFunction::getYforX(const float normalizedXPosition)
{
  float returnValue = 0.f;
  
  const int numPoints = mState.getNumChildren();
  
  //jassert(numPoints >= mMinimumNPoints);
  
  int lowPointIndex = 0;
  int highPointIndex = 1;
  
  float interpolationPosition = normalizedXPosition;
  
  if (normalizedXPosition == 0.f)
  {
    returnValue = mState.getChild(0).getProperty("y");
    return returnValue;
  }
  
  if(numPoints == 0)
    returnValue = 0.f; // TODO?
  else if(numPoints == 1)
  {
    returnValue = mState.getChild(0).getProperty("y");
    return returnValue;
  }
  else
  {
    if(numPoints == 2)
    {
      //interpolationPosition = normalizedXPosition;
    }
    else
    {
      // work out points to the left and right of the x position
      int pointIndex = numPoints;
      lowPointIndex = numPoints-1;
    
      while(pointIndex--)
      {
        ValueTree child = mState.getChild(pointIndex);
        float childNormalizedXPosition = mXRange.convertTo0to1(child.getProperty("x"));
    
        if(normalizedXPosition <= childNormalizedXPosition)
          lowPointIndex--;
      }
      
      highPointIndex = lowPointIndex + 1;
      
      // work out interpolation position
      
      const Point<float> pointA(mState.getChild(lowPointIndex).getProperty("x"), mState.getChild(lowPointIndex).getProperty("y"));
      const Point<float> pointB(mState.getChild(highPointIndex).getProperty("x"), mState.getChild(highPointIndex).getProperty("y"));
      
      const float distance = pointB.getX() - pointA.getX();

      if(distance > 0.f)
        interpolationPosition = (normalizedXPosition - pointA.getX()) / distance;
      else
        interpolationPosition = 0.f;
    }
    
    // convert point values to normalized
    const float normalizedLowY = mYRange.convertTo0to1(mState.getChild(lowPointIndex).getProperty("y"));
    const float normalizedhighY = mYRange.convertTo0to1(mState.getChild(highPointIndex).getProperty("y"));
    
    // linear interpolation
    const float interpolatedY = (normalizedhighY * interpolationPosition) + (normalizedLowY * (1.f-interpolationPosition));
    
    // get non normalized
    const float nonNormalizedY = mYRange.convertFrom0to1(interpolatedY);
    
    returnValue = nonNormalizedY;
  }
  
  return returnValue;
}

void BreakPointFunction::initialize()
{
  mState.removeAllChildren(nullptr);
  
  for(int bpIdx = 0; bpIdx < mInitialPoints.size(); bpIdx++)
  {
    PointInfo& bp = mInitialPoints.getReference(bpIdx);
    ValueTree child = mState.getOrCreateChildWithName(Identifier(String("bp_") + String(bpIdx)), nullptr);
    child.setProperty("x", bp.normX, nullptr);
    child.setProperty("y", bp.normY, nullptr); // inverted because top left is 0,0 in coordinate space
    child.setProperty("hdrag", bp.hDrag, nullptr);
    child.setProperty("vdrag", bp.vDrag, nullptr);
  }
  
  mNeedsResync = true;
  repaint();
}

void BreakPointFunction::addPointToInitState(float x, float y, bool enableHorizontalDrag, bool enableVerticalDrag)
{
  mInitialPoints.add(PointInfo(x, y, enableHorizontalDrag, enableVerticalDrag));
}

void BreakPointFunction::deleteBreakPoint(StringRef componentID)
{
  BreakPoint* obj = dynamic_cast<BreakPoint*>(findChildWithID(componentID));
  removeChildComponent(obj);
  int bpIdx = mBreakPoints.indexOf(obj); // ouch
  mBreakPoints.removeObject(obj);
  mState.removeChild(bpIdx, nullptr);
  
  updateState();
  
  mNeedsResync = true;
  repaint();
}

void BreakPointFunction::BreakPoint::mouseDown (const MouseEvent& e) 
{
  if (e.mods.isPopupMenu())
  {
    PopupMenu m;
    m.addItem(1, "Delete");
    int result = m.show();
    
    switch (result) {
      case 1:
      {
        BreakPointFunction* parent = dynamic_cast<BreakPointFunction*>(getParentComponent());
        parent->deleteBreakPoint(getComponentID());
        break;
      }
      default:
        break;
    }
  }
  else
  {
    startDraggingComponent (this, e);
  }
}

void BreakPointFunction::loadFromXMLString(const XmlElement& xml)
{
  mState = ValueTree::fromXml(xml);
  reloadFromState();
}
