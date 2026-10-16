#pragma once
#include "i18n/language.hpp"
#include "storage/atomic_file.hpp"
namespace vr {
inline Language loadLanguage(const std::string& file) {
  std::string data;
  if(!readFile(file,data)&&!readFile(file+".bak",data))return Language::English;
  return data=="VRLANG1\nfr\n"?Language::French:Language::English;
}
inline bool saveLanguage(const std::string& file,Language language) {
  if(language!=Language::English&&language!=Language::French)return false;
  return writeFile(file+".tmp",language==Language::French?"VRLANG1\nfr\n":"VRLANG1\nen\n")&&replaceFile(file+".tmp",file);
}
}
