#pragma once
#include "input/input.hpp"
#include "input/triggers.hpp"
#include <cstdint>
namespace vr {
class Vita {
public:
  Vita(); Input poll();
  double nowMs() const;
private:
  std::uint32_t previous_=0,held_=0;
  std::uint64_t repeatAt_=0;
  TouchTracker touch_;
  TriggerChord triggers_;
  bool touchReady_=false;
  float minX_=0,minY_=0,rangeX_=1,rangeY_=1;
};
}
