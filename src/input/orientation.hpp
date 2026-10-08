#pragma once
#include "input/input.hpp"
#include "reader/orientation.hpp"
namespace vr {
inline void orientInput(Input& in,Orientation orientation) {
  if(orientation==Orientation::Horizontal)return;
  const bool up=in.up,down=in.down;
  const auto touch=readingTouch({in.touch.x,in.touch.y},orientation);
  in.touch.x=touch.x; in.touch.y=touch.y;
  if(orientation==Orientation::VerticalRight) {
    in.up=in.right; in.down=in.left;
    in.previous=up||in.triggerPrevious; in.next=down||in.triggerNext;
  } else {
    in.up=in.left; in.down=in.right;
    in.previous=down||in.triggerPrevious; in.next=up||in.triggerNext;
  }
}
}
