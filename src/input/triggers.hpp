#pragma once
namespace vr {
struct TriggerAction { bool previous=false,next=false,jump=false; };
class TriggerChord {
public:
  TriggerAction update(bool left,bool right,double now) {
    TriggerAction action;
    if(left&&right) { action.jump=!latched_; latched_=true; pending_=0; }
    else if(latched_) { if(!left&&!right)latched_=false; }
    else {
      if((left&&!left_)||(right&&!right_)) { pending_=left?-1:1; deadline_=now+90; repeat_=now+350; }
      if(pending_&&(now>=deadline_||(!left&&!right))) {
        action.previous=pending_<0; action.next=pending_>0; pending_=0;
      } else if(!pending_&&(left||right)&&now>=repeat_) {
        action.previous=left; action.next=right; repeat_=now+110;
      }
    }
    left_=left; right_=right; return action;
  }
private:
  bool left_=false,right_=false,latched_=false;
  int pending_=0;
  double deadline_=0,repeat_=0;
};
}
