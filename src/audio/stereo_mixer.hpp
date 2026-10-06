#pragma once
#include <algorithm>
#include <cstddef>
#include <cstdint>
#include "audio/events.hpp"
namespace vr {
// 250 ms gain ramp, applied once per frame to both channels without changing pitch.
class StereoMixer {
public:
  void restart() { level_=0; }
  void mix(std::int16_t* output,const std::int16_t* music,std::size_t samples,
           const std::int16_t* effects,std::size_t frames,bool playing,bool enabled,int volume) {
    const int target=playing&&enabled?fadeFrames:0;
    volume=std::clamp(volume,0,3);
    for(std::size_t i=0;i<frames;++i) {
      if(level_<target)++level_; else if(level_>target)--level_;
      for(std::size_t channel=0;channel<2;++channel) {
        const auto index=i*2+channel;
        const int recording=index<samples?int(std::int64_t(music[index])*volume*level_/(4*fadeFrames)):0;
        output[index]=static_cast<std::int16_t>(std::clamp(recording+int(effects[i]),-32768,32767));
      }
    }
  }
  bool audible() const { return level_>0; }
private:
  static constexpr int fadeFrames=audioRate/4;
  int level_=0;
};
}
