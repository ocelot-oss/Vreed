#pragma once
namespace vr {
enum class Language { English, French };
inline Language uiLanguage=Language::English;
inline const char* tr(const char* english,const char* french) {
  return uiLanguage==Language::French?french:english;
}
}
