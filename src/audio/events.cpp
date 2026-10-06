#include "audio/events.hpp"
#include <algorithm>
#include <cmath>
namespace vr {
bool AudioEvents::push(AudioEvent e) {
  auto w=write_.load(std::memory_order_relaxed); auto next=(w+1)%events_.size();
  if(next==read_.load(std::memory_order_acquire))return false;
  events_[w]=e; write_.store(static_cast<unsigned>(next),std::memory_order_release); return true;
}
bool AudioEvents::pop(AudioEvent& e) {
  auto r=read_.load(std::memory_order_relaxed);
  if(r==write_.load(std::memory_order_acquire))return false;
  e=events_[r]; read_.store(static_cast<unsigned>((r+1)%events_.size()),std::memory_order_release); return true;
}
PaperSounds::PaperSounds() {
  // Original filtered-noise development placeholders. No recordings or downloaded assets.
  for(int event=0;event<4;++event) {
    std::uint32_t seed=0x71a54u+event*103u; float low=0;
    for(int i=0;i<lengths_[event];++i) {
      seed=1664525u*seed+1013904223u;
      const float noise=float(int((seed>>16)&0xffff)-32768)/32768;
      low=low*0.85f+noise*0.15f;
      const float t=float(i)/lengths_[event];
      const float envelope=std::sin(3.14159265f*t)*std::sin(3.14159265f*t);
      const float rustle=event<2?(noise-low)*0.35f+low:low;
      samples_[event][i]=static_cast<std::int16_t>(rustle*envelope*(event<2?1800:950));
    }
  }
}
void PaperSounds::play(AudioEvent e) { event_=static_cast<int>(e); cursor_=0; length_=lengths_[event_]; }
void PaperSounds::render(std::int16_t* output,std::size_t frames,int volume) {
  volume=std::clamp(volume,0,3);
  for(std::size_t i=0;i<frames;++i)output[i]=cursor_<length_?static_cast<std::int16_t>(samples_[event_][cursor_++]*volume/4):0;
}
}
