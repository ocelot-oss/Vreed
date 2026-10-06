#include "storage/settings.hpp"
#include "storage/atomic_file.hpp"
#include <cstdio>
#include <fstream>
namespace vr {
ReadingSettings loadSettings(const std::string& path) {
  ReadingSettings s; std::ifstream in(path); std::string version; int mode,layout,book,clean,oled,sound;
  if(!(in>>version)||version!="VRS1"||!(in>>mode>>layout>>s.fontSize>>s.lineSpacing>>s.margins>>book>>clean>>oled>>sound>>s.soundVolume))return {};
  s.mode=static_cast<ReadingMode>(mode); s.bookLayout=static_cast<BookLayout>(layout);
  s.animation={static_cast<TransitionKind>(book),static_cast<TransitionKind>(clean),static_cast<TransitionKind>(oled)};
  s.pageSound=sound!=0;
  std::string trailing;
  if((sound!=0&&sound!=1)||!validSettings(s)||(in>>trailing))return {};
  return s;
}
bool saveSettings(const std::string& path,const ReadingSettings& s) {
  if(!validSettings(s))return false;
  const auto temp=path+".tmp";
  { std::ofstream out(temp,std::ios::trunc);
    out<<"VRS1\n"<<static_cast<int>(s.mode)<<' '<<static_cast<int>(s.bookLayout)<<' '<<s.fontSize<<' '<<s.lineSpacing<<' '<<s.margins<<' '
       <<static_cast<int>(s.animation[0])<<' '<<static_cast<int>(s.animation[1])<<' '<<static_cast<int>(s.animation[2])<<' '<<s.pageSound<<' '<<s.soundVolume<<'\n';
    out.flush(); if(!out)return false;
  }
  return replaceFile(temp,path);
}
}
