#pragma once
#include "reader/layout.hpp"
#include "reader/transition.hpp"
#include <array>
#include <cstdint>
namespace vr {
enum class ReadingMode { Book, Clean, Oled };
enum class BookLayout { Single, Spread };
struct ReadingSettings {
  ReadingMode mode=ReadingMode::Book;
  BookLayout bookLayout=BookLayout::Single;
  int fontSize=1, lineSpacing=1, margins=1;
  std::array<TransitionKind,3> animation{TransitionKind::Curl,TransitionKind::Slide,TransitionKind::Slide};
  bool pageSound=true;
  int soundVolume=1; // 0..3: quiet levels 0%, 25%, 50%, 75% of the already subdued placeholder.
};
struct Rect { int x=0,y=0,width=0,height=0; };
struct ReadingTheme {
  std::uint32_t background,paper,text,secondary,accent;
  bool physical=false,texture=false;
};
struct Presentation {
  ReadingTheme theme;
  std::array<Rect,2> paper{},content{};
  int pageCount=1;
  bool spreadLimited=false;
  Layout layout;
};
using HeightMeasure=std::function<int(float)>;
Presentation presentation(const ReadingSettings& settings,const HeightMeasure& height);
const char* modeName(ReadingMode mode);
bool adjustSetting(ReadingSettings& settings,int row,int direction);
std::string settingValue(const ReadingSettings& settings,int row);
constexpr int settingsRowCount=8;
TransitionKind animationFor(const ReadingSettings& settings);
bool validSettings(const ReadingSettings& settings);
}
