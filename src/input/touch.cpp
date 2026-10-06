#include "input/touch.hpp"
#include "reader/transition.hpp"
#include <algorithm>
#include <cmath>
namespace vr {
int SwipeGesture::update(const TouchPoint& point) {
  if(point.phase==TouchPhase::Down) { start_=point; pending_=true; return 0; }
  if(point.phase==TouchPhase::Cancel||point.phase==TouchPhase::Up) { pending_=false; return 0; }
  if(!pending_||point.phase!=TouchPhase::Move)return 0;
  const float dx=point.x-start_.x,dy=point.y-start_.y;
  if(std::abs(dy)>36&&std::abs(dy)>std::abs(dx)) { pending_=false; return 0; }
  if(std::abs(dx)<24||std::abs(dx)<std::abs(dy)*1.2f)return 0;
  pending_=false; return dx<0?1:-1;
}
TouchPoint TouchTracker::sample(const Contact* contacts,int count,bool valid) {
  if(!valid||count>1) {
    TouchPoint event{id_>=0?TouchPhase::Cancel:TouchPhase::None,id_,x_,y_};
    id_=-1; blocked_=true; return event;
  }
  if(count==0) {
    TouchPoint event{id_>=0?TouchPhase::Up:TouchPhase::None,id_,x_,y_};
    id_=-1; blocked_=false; return event;
  }
  if(blocked_)return {};
  const auto& c=contacts[0];
  if(id_>=0&&id_!=c.id) { auto event=TouchPoint{TouchPhase::Cancel,id_,x_,y_}; id_=-1; blocked_=true; return event; }
  const bool down=id_<0; id_=c.id; x_=c.x; y_=c.y;
  return {down?TouchPhase::Down:TouchPhase::Move,id_,x_,y_};
}
int pageEdge(const Presentation& p,const TouchPoint& touch) {
  if(touch.phase!=TouchPhase::Down)return 0;
  const auto left=p.paper[0],right=p.paper[p.pageCount-1];
  if(touch.y<left.y||touch.y>left.y+left.height)return 0;
  if(touch.x>=right.x+right.width-std::max(96.0f,interaction::edgeZone)&&touch.x<=959)return 1;
  if(touch.x<=left.x+std::max(96.0f,interaction::edgeZone)&&touch.x>=0)return -1;
  return 0;
}
}
