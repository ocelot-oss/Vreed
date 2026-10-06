#pragma once
#include <cstdint>
namespace vr {
// Collected in memory; never rendered as release UI. Estimates long frame gaps, not GPU-measured dropped frames.
struct FrameStats {
  std::uint64_t frames=0,longGaps=0;
  double totalMs=0,maxMs=0,lastTurnMs=0;
  void frame(double ms) { if(ms<0)return; ++frames; totalMs+=ms; if(ms>maxMs)maxMs=ms; if(ms>25)++longGaps; }
};
}
