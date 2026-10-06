#include "platform/vita_audio.hpp"
#include <psp2/audioout.h>
#include <psp2/kernel/threadmgr.h>
#include <cstring>
#include <new>
#include <cstdio>
#include <algorithm>
#include "audio/stereo_mixer.hpp"
namespace vr {
namespace { constexpr int outputFrames=2048; }
VitaAudio::VitaAudio() {
  port_=sceAudioOutOpenPort(SCE_AUDIO_OUT_PORT_TYPE_MAIN,outputFrames,audioRate,SCE_AUDIO_OUT_MODE_STEREO);
  if(port_<0)return;
  int volume[2]={SCE_AUDIO_VOLUME_0DB,SCE_AUDIO_VOLUME_0DB};
  if(sceAudioOutSetVolume(port_,static_cast<SceAudioOutChannelFlag>(SCE_AUDIO_VOLUME_FLAG_L_CH|SCE_AUDIO_VOLUME_FLAG_R_CH),volume)<0) {
    sceAudioOutReleasePort(port_); port_=-1; return;
  }
  // Keep the fixed sample bank off the application's main stack.
  sounds_.reset(new(std::nothrow) PaperSounds);
  if(!sounds_) { sceAudioOutReleasePort(port_); port_=-1; return; }
  thread_=sceKernelCreateThread("VitaReader audio",entry,0x100000FF,0x10000,0,0,nullptr);
  if(thread_<0) { sceAudioOutReleasePort(port_); port_=-1; return; }
  running_.store(true); available_.store(true); auto* self=this;
  if(sceKernelStartThread(thread_,sizeof(self),&self)<0) {
    available_.store(false); running_.store(false); sceKernelDeleteThread(thread_); thread_=-1;
    sceAudioOutReleasePort(port_); port_=-1; return;
  }
}
VitaAudio::~VitaAudio() {
  available_.store(false); running_.store(false);
  if(thread_>=0) { sceKernelWaitThreadEnd(thread_,nullptr,nullptr); sceKernelDeleteThread(thread_); }
  if(port_>=0)sceAudioOutReleasePort(port_);
}
int VitaAudio::entry(unsigned args,void* data) {
  if(args!=sizeof(VitaAudio*)||!data)return -1;
  VitaAudio* self=nullptr; std::memcpy(&self,data,sizeof self); self->run(); return 0;
}
void VitaAudio::run() {
  // Do not overwrite a submitted buffer while the device is consuming it.
  alignas(64) std::array<std::array<std::int16_t,outputFrames*2>,2> output{};
  std::array<std::int16_t,outputFrames> effects{};
  std::array<std::int16_t,outputFrames*2> musicBlock{};
  unsigned nextBuffer=0;
  StereoMixer mixer;
  // Stream the converted recording on this worker; never load the whole song into RAM.
  auto* music=std::fopen("app0:/assets/audio/library-stereo.pcm","rb");
  if(music)std::setvbuf(music,nullptr,_IOFBF,65536);
  bool playing=false;
  while(running_.load()) {
    AudioEvent event;
    while(events_.pop(event)) {
      if(event==AudioEvent::StartupMusic) {
        playing=true; mixer.restart(); if(music)std::rewind(music);
      } else if(event==AudioEvent::StopMusic)playing=false;
      else if(enabled_.load())sounds_->play(event);
    }
    if(!enabled_.load())sounds_->stop();
    sounds_->render(effects.data(),effects.size(),volume_.load());
    std::size_t count=0;
    if(music&&(playing||mixer.audible())) {
      count=std::fread(musicBlock.data(),sizeof(std::int16_t),musicBlock.size(),music);
      if(count<musicBlock.size()) {
        if(std::ferror(music)) { playing=false; mixer.restart(); }
        else { std::rewind(music); count+=std::fread(musicBlock.data()+count,sizeof(std::int16_t),musicBlock.size()-count,music); }
      }
    }
    auto& block=output[nextBuffer];
    mixer.mix(block.data(),musicBlock.data(),count,effects.data(),effects.size(),playing,enabled_.load(),volume_.load());
    if(sceAudioOutOutput(port_,block.data())<0) { available_.store(false); running_.store(false); }
    nextBuffer^=1;
  }
  if(music)std::fclose(music);
  sceAudioOutOutput(port_,nullptr);
}
}
