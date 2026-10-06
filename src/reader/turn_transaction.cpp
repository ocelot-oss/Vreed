#include "reader/turn_transaction.hpp"
namespace vr {
bool ReaderTurn::prepare(Session& current,int direction,const Measure& measure,int pages) {
  if(motion_.active()||candidate_)return false;
  auto candidate=std::make_unique<Session>(current);
  if(!candidate->turn(direction,measure,pages))return false;
  candidate_=std::move(candidate); current_=&current; direction_=direction; return true;
}
bool ReaderTurn::start(TransitionKind kind,double nowMs) { return candidate_&&motion_.start(direction_,kind,nowMs); }
bool ReaderTurn::beginDrag(TransitionKind kind,double nowMs,float x,float y,float width,float top,float height) {
  return candidate_&&motion_.beginDrag(direction_,kind,nowMs,x,y,width,top,height);
}
TurnEvents ReaderTurn::apply(TurnEvents event) {
  if(event.commit&&candidate_) *current_=std::move(*candidate_);
  if(event.commit||event.cancel) { candidate_.reset(); current_=nullptr; }
  return event;
}
TurnEvents ReaderTurn::tick(double nowMs) { return apply(motion_.tick(nowMs)); }
TurnEvents ReaderTurn::finish() { return apply(motion_.finish()); }
void ReaderTurn::abort() { motion_.cancel(0); motion_.finish(); candidate_.reset(); current_=nullptr; }
}
