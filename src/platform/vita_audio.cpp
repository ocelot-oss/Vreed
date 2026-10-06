#include "platform/vita_audio.hpp"
#include <psp2/audioout.h>
#include <psp2/kernel/threadmgr.h>
#include <cstring>
#include <new>
#include <cstdio>
#include <algorithm>
namespace vr {
VitaAudio::VitaAudio() {
  port_=sceAudioOutOpenPort(SCE_AUDIO_OUT_PORT_TYPE_MAIN,audioBlock,audioRate,SCE_AUDIO_OUT_MODE_MONO);
  if(port_<0)return;
  int volume[2]={SCE_AUDIO_VOLUME_0DB,SCE_AUDIO_VOLUME_0DB};
  if(sceAudioOutSetVolume(port_,static_cast<SceAudioOutChannelFlag>(SCE_AUDIO_VOLUME_FLAG_L_CH|SCE_AUDIO_VOLUME_FLAG_R_CH),volume)<0) {
    sceAudioOutReleasePort(port_); port_=-1; return;
  }
  // Keep the fixed sample bank off the application's main stack.
  sounds_.reset(new(std::nothrow) PaperSounds);
  if(!sounds_) { sceAudioOutReleasePort(port_); port_=-1; return; }
  thread_=sceKernelCreateThread("VitaReader audio",entry,0x10000100,0x10000,0,0,nullptr);
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
  alignas(64) std::array<std::int16_t,audioBlock> block{};
  std::array<std::int16_t,audioBlock> musicBlock{};
  // Stream the converted recording on this worker; never load the whole song into RAM.
  auto* music=std::fopen("app0:/assets/audio/library.pcm","rb");
  bool playing=false;
  float gain=0;
  while(running_.load()) {
    AudioEvent event;
    while(events_.pop(event)) {
      if(event==AudioEvent::StartupMusic) {
        playing=true; gain=0; if(music)std::rewind(music);
      } else if(event==AudioEvent::StopMusic)playing=false;
      else if(enabled_.load())sounds_->play(event);
    }
    if(!enabled_.load())sounds_->stop();
    sounds_->render(block.data(),block.size(),volume_.load());
    if(music&&(playing||gain>0)) {
      auto count=std::fread(musicBlock.data(),sizeof(std::int16_t),musicBlock.size(),music);
      if(count<musicBlock.size()) {
        if(std::ferror(music)) { playing=false; gain=0; }
        else { std::rewind(music); count+=std::fread(musicBlock.data()+count,sizeof(std::int16_t),musicBlock.size()-count,music); }
      }
      for(std::size_t i=0;i<block.size();++i) {
        const float target=playing&&enabled_.load()?1.0f:0.0f;
        gain=std::clamp(gain+(target>gain?1.0f:-1.0f)/(audioRate*0.25f),0.0f,1.0f);
        const int sample=i<count?int(musicBlock[i]*gain*std::clamp(volume_.load(),0,3)/4):0;
        block[i]=static_cast<std::int16_t>(std::clamp(int(block[i])+sample,-32768,32767));
      }
    }
    if(sceAudioOutOutput(port_,block.data())<0) { available_.store(false); running_.store(false); }
  }
  if(music)std::fclose(music);
  sceAudioOutOutput(port_,nullptr);
}
}
