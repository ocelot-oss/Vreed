#pragma once
#include <array>
#include <atomic>
#include <cstddef>
#include <cstdint>
namespace vr {
enum class AudioEvent { PageForward, PageBackward, BookmarkInsert, BookmarkRemove };
// UI producer, audio-thread consumer. Bounded, no allocation or locking on the UI path.
class AudioEvents {
public:
  bool push(AudioEvent event);
  bool pop(AudioEvent& event);
private:
  std::array<AudioEvent,16> events_{};
  std::atomic<unsigned> read_{0},write_{0};
};
constexpr int audioRate=48000,audioBlock=512;
class PaperSounds {
public:
  PaperSounds();
  void play(AudioEvent event);
  void stop() { cursor_=length_; }
  void render(std::int16_t* output,std::size_t frames,int volume);
private:
  static constexpr int maxSamples=6720;
  std::array<std::array<std::int16_t,maxSamples>,4> samples_{};
  std::array<int,4> lengths_{6720,6720,2400,2400};
  int event_=0,cursor_=0,length_=0;
};
}
