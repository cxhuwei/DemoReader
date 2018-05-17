#include "StdAfx.h"
#include <math.h>
#include "ImageScalingmath.h"


SS_Point2::SS_Point2()
{
  x=y=0.0f;
}

SS_Point2::SS_Point2(float const x_, float const y_)
{
  x=x_;
  y=y_;
}

SS_Point2::SS_Point2(SS_Point2 const &p)
{
  x=p.x;
  y=p.y;
}

float SS_Point2::Distance(SS_Point2 const p2)
{
  return (float)sqrt((x-p2.x)*(x-p2.x)+(y-p2.y)*(y-p2.y));
}

float SS_Point2::Distance(float const x_, float const y_)
{
  return (float)sqrt((x-x_)*(x-x_)+(y-y_)*(y-y_));
}

SS_Rect2::SS_Rect2()
{
}

SS_Rect2::SS_Rect2(float const x1_, float const y1_, float const x2_, float const y2_)
{
  botLeft.x=x1_;
  botLeft.y=y1_;
  topRight.x=x2_;
  topRight.y=y2_;
}

SS_Rect2::SS_Rect2(SS_Rect2 const &p)
{
  botLeft=p.botLeft;
  topRight=p.topRight;
}

float SS_Rect2::Surface() const
//Returns the surface of rectangle. 
{
  return (topRight.x-botLeft.x)*(topRight.y-botLeft.y);
}

SS_Rect2 SS_Rect2::CrossSection(SS_Rect2 const &r2) const
// Returns crossection with another rectangle.
{
  SS_Rect2 cs;
  cs.botLeft.x=max(botLeft.x, r2.botLeft.x);
  cs.botLeft.y=max(botLeft.y, r2.botLeft.y);
  cs.topRight.x=min(topRight.x, r2.topRight.x);
  cs.topRight.y=min(topRight.y, r2.topRight.y);
  if (cs.botLeft.x<=cs.topRight.x && cs.botLeft.y<=cs.topRight.y)
  {
    return cs;
  }else 
  {
    return SS_Rect2(0,0,0,0);
  }
}

SS_Point2 SS_Rect2::Center() const
//Returns the center point of rectangle. 
{
  return SS_Point2((topRight.x+botLeft.x)/2.0f, (topRight.y+botLeft.y)/2.0f);
}

float SS_Rect2::Width() const
//returns rectangle width
{
  return topRight.x-botLeft.x;
}

float SS_Rect2::Height() const
//returns rectangle height
{
  return topRight.y-botLeft.y;
}
