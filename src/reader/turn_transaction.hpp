#pragma once
#include "reader/session.hpp"
#include "reader/transition.hpp"
#include <memory>
namespace vr {
// Candidate navigation never changes the live session until a completed turn commits.
class ReaderTurn {
public:
  bool prepare(Session& current,int direction,const Measure& measure,int pages);
  bool start(TransitionKind kind,double nowMs);
  bool beginDrag(TransitionKind kind,double nowMs,float x,float y,float width,float top,float height);
  TurnEvents tick(double nowMs);
  TurnEvents finish();
  void abort();
  PageTransition& motion() { return motion_; }
  const PageTransition& motion() const { return motion_; }
  Session* candidate() { return candidate_.get(); }
private:
  Session* current_=nullptr;
  std::unique_ptr<Session> candidate_;
  PageTransition motion_;
  int direction_=0;
  TurnEvents apply(TurnEvents event);
};
}
