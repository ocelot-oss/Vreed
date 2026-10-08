#include "storage/settings.hpp"
#include "storage/atomic_file.hpp"
#include "storage/file.hpp"
#include <sstream>
#include <cstdio>
#include <fstream>
namespace vr {
ReadingSettings loadSettings(const std::string& path) {
  std::string data; if(!readFile(path,data)&&!readFile(path+".bak",data))return {};
  ReadingSettings s; std::istringstream in(data); std::string version; int mode,layout,book,clean,oled,sound;
  if(!(in>>version)||(version!="VRS1"&&version!="VRS2"&&version!="VRS3")||!(in>>mode>>layout>>s.fontSize>>s.lineSpacing>>s.margins>>book>>clean>>oled>>sound>>s.soundVolume))return {};
  if(version=="VRS2"||version=="VRS3") { int capture; if(!(in>>s.fontFamily>>capture)||(capture!=0&&capture!=1))return {}; s.bookmarkScreenshot=capture!=0; }
  if(version=="VRS3") { int orientation; if(!(in>>orientation))return {}; s.orientation=static_cast<Orientation>(orientation); }
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
  { std::ostringstream out;
    out<<"VRS3\n"<<static_cast<int>(s.mode)<<' '<<static_cast<int>(s.bookLayout)<<' '<<s.fontSize<<' '<<s.lineSpacing<<' '<<s.margins<<' '
       <<static_cast<int>(s.animation[0])<<' '<<static_cast<int>(s.animation[1])<<' '<<static_cast<int>(s.animation[2])<<' '<<s.pageSound<<' '<<s.soundVolume<<' '<<s.fontFamily<<' '<<s.bookmarkScreenshot<<' '<<static_cast<int>(s.orientation)<<'\n';
    if(!writeFile(temp,out.str()))return false;
  }
  return replaceFile(temp,path);
}
}
