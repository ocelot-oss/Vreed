#include "input/touch.hpp"
#include "reader/transition.hpp"
namespace vr {
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
  if(!p.theme.physical||touch.phase!=TouchPhase::Down)return 0;
  const auto left=p.paper[0],right=p.paper[p.pageCount-1];
  if(touch.y<left.y||touch.y>left.y+left.height)return 0;
  if(touch.x>=right.x+right.width-interaction::edgeZone&&touch.x<=right.x+right.width)return 1;
  if(touch.x>=left.x&&touch.x<=left.x+interaction::edgeZone)return -1;
  return 0;
}
}
