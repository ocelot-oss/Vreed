#pragma once
#include "input/touch.hpp"
namespace vr {
struct Input {
  bool up=false,down=false,previous=false,next=false,open=false,back=false,rescan=false,quit=false,bookmark=false;
  TouchPoint touch;
  double nowMs=0;
  bool jump=false;
  bool left=false,right=false,triggerPrevious=false,triggerNext=false;
};
}
