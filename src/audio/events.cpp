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
      const float seconds=float(i)/audioRate;
      const float confirmation=(std::sin(seconds*6.2831853f*659.25f)+0.5f*std::sin(seconds*6.2831853f*987.77f))*envelope*2000;
      samples_[event][i]=static_cast<std::int16_t>(event==2?confirmation:rustle*envelope*(event<2?1800:950));
    }
  }
}
void PaperSounds::play(AudioEvent e) {
  event_=static_cast<int>(e); cursor_=0;
  length_=e==AudioEvent::StartupMusic?audioRate*6:e==AudioEvent::StopMusic?0:lengths_[event_];
}
void PaperSounds::render(std::int16_t* output,std::size_t frames,int volume) {
  volume=std::clamp(volume,0,3);
  for(std::size_t i=0;i<frames;++i) {
    if(cursor_>=length_) { output[i]=0; continue; }
    if(event_==static_cast<int>(AudioEvent::StartupMusic)) {
      const float t=float(cursor_++)/audioRate;
      const float notes[]={261.63f,329.63f,392.0f,493.88f,293.66f,349.23f,440.0f,523.25f};
      const int beat=int(t/0.75f)%8; const float phase=std::fmod(t,0.75f);
      const float attack=std::min(1.0f,phase/0.025f),fade=std::min(1.0f,(6-t)/0.8f);
      const float bell=std::sin(6.2831853f*notes[beat]*t)+0.22f*std::sin(6.2831853f*notes[beat]*2*t);
      const float pad=std::sin(6.2831853f*130.815f*t)+0.4f*std::sin(6.2831853f*196*t);
      output[i]=static_cast<std::int16_t>((bell*std::exp(-phase*4)*attack+pad*0.17f)*fade*1300*volume/4);
    } else output[i]=static_cast<std::int16_t>(samples_[event_][cursor_++]*volume/4);
  }
}
}
