#pragma once
#include "input/touch.hpp"
namespace vr {
struct Input {
  bool up=false,down=false,previous=false,next=false,open=false,back=false,rescan=false,quit=false,bookmark=false;
  TouchPoint touch;
  double nowMs=0;
};
}
