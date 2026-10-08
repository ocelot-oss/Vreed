#pragma once
#include "reader/settings.hpp"
#include <algorithm>
namespace vr {
struct ReadingPoint { float x,y; };
inline ReadingPoint screenPoint(ReadingPoint p,Orientation orientation) {
  if(orientation==Orientation::VerticalRight)return {960-p.y,p.x};
  if(orientation==Orientation::VerticalLeft)return {p.y,544-p.x};
  return p;
}
inline ReadingPoint readingTouch(ReadingPoint p,Orientation orientation) {
  if(orientation==Orientation::VerticalRight)return {p.y,959-p.x};
  if(orientation==Orientation::VerticalLeft)return {543-p.y,p.x};
  return p;
}
inline Rect pageChooserTrack(const Presentation& p) {
  const int width=std::min(580,p.width-40),x=(p.width-width)/2,y=(p.height-250)/2;
  return {x+60,y+125,width-120,4};
}
}
