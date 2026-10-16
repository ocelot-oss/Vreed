#pragma once
#include "input/touch.hpp"
#include <cmath>
namespace vr {
struct PdfTouchAction { int turn=0; float pan=0; };
class PdfGesture {
public:
  PdfTouchAction update(const TouchPoint& p,double now,int width) {
    if(p.phase==TouchPhase::Down) { start_=p; last_=p; started_=now; active_=true; panning_=false; return {}; }
    if(p.phase==TouchPhase::Cancel) { active_=false; return {}; }
    if(!active_||p.phase==TouchPhase::None)return {};
    if(p.id!=start_.id) { active_=false; return {}; }
    const float dx=p.x-start_.x,dy=p.y-start_.y;
    if(p.phase==TouchPhase::Move) {
      if(!panning_&&std::abs(dy)>24&&std::abs(dy)>std::abs(dx)*1.2f)panning_=true;
      const float pan=panning_?last_.y-p.y:0; last_=p; return {0,pan};
    }
    if(p.phase!=TouchPhase::Up)return {};
    active_=false;
    if(panning_)return {};
    if(std::abs(dx)>=40&&std::abs(dx)>std::abs(dy)*1.2f)return {dx<0?1:-1,0};
    if(std::abs(dx)<=12&&std::abs(dy)<=12&&now-started_<=450) {
      if(p.x<width*0.28f)return {-1,0};
      if(p.x>width*0.72f)return {1,0};
    }
    return {};
  }
private:
  TouchPoint start_,last_; double started_=0; bool active_=false,panning_=false;
};
}
