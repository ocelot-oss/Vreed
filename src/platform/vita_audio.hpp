#pragma once
#include "audio/events.hpp"
#include <memory>
namespace vr {
class VitaAudio {
public:
  VitaAudio(); ~VitaAudio();
  VitaAudio(const VitaAudio&)=delete; VitaAudio& operator=(const VitaAudio&)=delete;
  void settings(bool enabled,int volume) { enabled_.store(enabled); volume_.store(volume); }
  void emit(AudioEvent event) { if(available_.load()&&(enabled_.load()||event==AudioEvent::StartupMusic||event==AudioEvent::StopMusic))events_.push(event); }
  bool available() const { return available_.load(); }
private:
  int port_=-1,thread_=-1;
  std::atomic<bool> running_{false},available_{false},enabled_{true};
  std::atomic<int> volume_{1};
  AudioEvents events_; std::unique_ptr<PaperSounds> sounds_;
  static int entry(unsigned args,void* data);
  void run();
};
}
