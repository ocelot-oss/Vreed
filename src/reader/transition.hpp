#pragma once
#include <cstdint>
namespace vr {
enum class TransitionKind { None, Slide, Curl };
enum class TurnState { Idle, Dragging, Completing, Cancelling };
namespace interaction {
constexpr float curlMs=360,slideMs=160,settleMinMs=90;
constexpr float releaseThreshold=0.22f,flickPagesPerSecond=0.45f,minFlickProgress=0.06f;
constexpr float velocityFreshMs=80,soundProgress=0.52f,edgeZone=48;
constexpr float ribbonMs=220,boundaryMs=160,pauseGapMs=250;
constexpr int curlSegments=16,shadowBands=8;
}
float smoothStep(float value);
struct TurnEvents { bool commit=false,cancel=false,sound=false; int direction=0; };
class PageTransition {
public:
  bool start(int direction,TransitionKind kind,double nowMs);
  bool beginDrag(int direction,TransitionKind kind,double nowMs,float x,float y,float width,float top,float height);
  void drag(float x,double nowMs);
  void release(double nowMs);
  void cancel(double nowMs);
  TurnEvents tick(double nowMs);
  // Interrupt: complete an automatic/accepted turn, cancel an unaccepted drag.
  TurnEvents finish();
  TurnState state() const { return state_; }
  bool active() const { return state_!=TurnState::Idle; }
  float progress() const { return progress_; }
  int direction() const { return direction_; }
  TransitionKind kind() const { return kind_; }
  float corner() const { return corner_; }
private:
  TurnState state_=TurnState::Idle;
  TransitionKind kind_=TransitionKind::None;
  int direction_=0;
  float progress_=0,from_=0,target_=1,width_=1,startX_=0,lastX_=0,velocity_=0,corner_=0.5f;
  double started_=0,duration_=0,lastSample_=0;
  bool sounded_=false;
  void settle(bool accept,double nowMs);
};
class RibbonMotion {
public:
  void setVisible(bool visible,double nowMs,bool immediate=false);
  void tick(double nowMs);
  float amount() const { return value_; }
  bool targetVisible() const { return target_>0; }
private:
  float value_=0,from_=0,target_=0;
  double started_=0;
};
class EdgeFeedback {
public:
  void trigger(int direction,double nowMs) { direction_=direction; started_=nowMs; }
  float amount(double nowMs) const;
  int direction() const { return direction_; }
private: int direction_=0; double started_=-1000;
};
}
