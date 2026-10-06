#pragma once
#include "input/input.hpp"
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
  bool touchReady_=false;
  float minX_=0,minY_=0,rangeX_=1,rangeY_=1;
};
}
