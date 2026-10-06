#include "reader/transition.hpp"
#include <algorithm>
#include <cmath>
namespace vr {
float smoothStep(float t) { t=std::clamp(t,0.0f,1.0f); return t*t*(3-2*t); }
bool PageTransition::start(int direction,TransitionKind kind,double nowMs) {
  if(active()||(direction!=1&&direction!=-1))return false;
  state_=TurnState::Completing; kind_=kind; direction_=direction; progress_=from_=0; target_=1;
  started_=nowMs; sounded_=false; corner_=0.5f;
  duration_=kind==TransitionKind::None?0:kind==TransitionKind::Curl?interaction::curlMs:interaction::slideMs;
  return true;
}
bool PageTransition::beginDrag(int direction,TransitionKind kind,double nowMs,float x,float y,float width,float top,float height) {
  if(width<=0||height<=0||!start(direction,kind,nowMs))return false;
  state_=TurnState::Dragging; startX_=lastX_=x; width_=width; lastSample_=nowMs; velocity_=0;
  corner_=std::clamp((y-top)/height,0.0f,1.0f); return true;
}
void PageTransition::drag(float x,double nowMs) {
  if(state_!=TurnState::Dragging)return;
  const auto dt=nowMs-lastSample_;
  if(dt>0) {
    const float speed=float((lastX_-x)*direction_/width_*1000/dt);
    velocity_=0.45f*velocity_+0.55f*speed;
  }
  progress_=std::clamp((startX_-x)*direction_/width_,0.0f,1.0f);
  lastX_=x; lastSample_=nowMs;
}
void PageTransition::settle(bool accept,double nowMs) {
  from_=progress_; target_=accept?1:0; state_=accept?TurnState::Completing:TurnState::Cancelling; started_=nowMs;
  const float total=kind_==TransitionKind::Curl?interaction::curlMs:interaction::slideMs;
  duration_=kind_==TransitionKind::None?0:std::max(interaction::settleMinMs,total*std::abs(target_-from_));
}
void PageTransition::release(double nowMs) {
  if(state_!=TurnState::Dragging)return;
  const bool flick=nowMs-lastSample_<=interaction::velocityFreshMs&&velocity_>=interaction::flickPagesPerSecond&&progress_>=interaction::minFlickProgress;
  settle(progress_>=interaction::releaseThreshold||flick,nowMs);
}
void PageTransition::cancel(double nowMs) { if(active())settle(false,nowMs); }
TurnEvents PageTransition::tick(double nowMs) {
  TurnEvents event; event.direction=direction_;
  if(state_==TurnState::Idle||state_==TurnState::Dragging)return event;
  const float t=duration_<=0?1:std::clamp(float((nowMs-started_)/duration_),0.0f,1.0f);
  progress_=from_+(target_-from_)*smoothStep(t);
  if(state_==TurnState::Completing&&!sounded_&&progress_>=interaction::soundProgress) { event.sound=true; sounded_=true; }
  if(t>=1) { event.commit=state_==TurnState::Completing; event.cancel=!event.commit; state_=TurnState::Idle; }
  return event;
}
TurnEvents PageTransition::finish() {
  TurnEvents event; event.direction=direction_;
  if(!active())return event;
  event.commit=state_==TurnState::Completing; event.cancel=!event.commit;
  event.sound=event.commit&&!sounded_; progress_=event.commit?1:0; state_=TurnState::Idle; sounded_=sounded_||event.sound;
  return event;
}
void RibbonMotion::setVisible(bool visible,double nowMs,bool immediate) {
  tick(nowMs); const float target=visible?1:0;
  if(immediate) { value_=from_=target_=target; started_=nowMs; return; }
  if(target==target_)return;
  from_=value_; target_=target; started_=nowMs;
}
void RibbonMotion::tick(double nowMs) {
  const float t=smoothStep(float((nowMs-started_)/interaction::ribbonMs));
  value_=from_+(target_-from_)*t;
}
float EdgeFeedback::amount(double nowMs) const {
  if(!direction_||nowMs<started_)return 0;
  return 1-smoothStep(float((nowMs-started_)/interaction::boundaryMs));
}
}
