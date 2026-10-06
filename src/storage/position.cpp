#include "storage/library.hpp"
#include "storage/atomic_file.hpp"
#include "storage/file.hpp"
#include "app/config.hpp"
#include <cstdint>
#include <cstdio>
#include <fstream>
#include <iomanip>
#include <sstream>
namespace vr {
namespace {
std::string file(const std::string& path,const std::string& directory) {
  std::uint64_t h=14695981039346656037ULL; for(unsigned char c:path) { h^=c; h*=1099511628211ULL; }
  std::ostringstream s; s<<(directory.empty()?config::positions:directory)<<'/'<<std::hex<<h<<".pos"; return s.str();
}
}
Position loadPosition(const std::string& path,const std::string& directory) {
  std::string data; if(!readFile(file(path,directory),data)&&!readFile(file(path,directory)+".bak",data))return {};
  std::istringstream in(data); std::string version,saved; Position p;
  if(!std::getline(in,version)||(version!="VR1"&&version!="VR2"&&version!="VR3")||!std::getline(in,saved)||saved!=path||!(in>>p.chapter>>p.page))return {};
  if(version!="VR1") { if(!(in>>p.offset>>p.hasOffset))return {}; }
  if(version=="VR3"&&!(in>>p.resumePinned))return {};
  return p;
}
bool savePosition(const std::string& path,Position p,const std::string& directory) {
  auto target=file(path,directory),temp=target+".tmp";
  std::ostringstream out; out<<"VR3\n"<<path<<'\n'<<p.chapter<<' '<<p.page<<' '<<p.offset<<' '<<p.hasOffset<<' '<<p.resumePinned<<'\n';
  if(!writeFile(temp,out.str()))return false;
  // POSIX rename replaces atomically; retain the old checkpoint on failure.
  return replaceFile(temp,target);
}
}
