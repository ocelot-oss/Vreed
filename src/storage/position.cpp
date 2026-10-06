#include "storage/library.hpp"
#include "storage/atomic_file.hpp"
#include "app/config.hpp"
#include <cstdint>
#include <cstdio>
#include <fstream>
#include <iomanip>
#include <sstream>
namespace vr {
namespace {
std::string file(const std::string& path) {
  std::uint64_t h=14695981039346656037ULL; for(unsigned char c:path) { h^=c; h*=1099511628211ULL; }
  std::ostringstream s; s<<config::positions<<'/'<<std::hex<<h<<".pos"; return s.str();
}
}
Position loadPosition(const std::string& path) {
  std::ifstream in(file(path)); std::string version,saved; Position p;
  if(!std::getline(in,version)||(version!="VR1"&&version!="VR2")||!std::getline(in,saved)||saved!=path||!(in>>p.chapter>>p.page))return {};
  if(version=="VR2") { if(!(in>>p.offset>>p.hasOffset))return {}; }
  return p;
}
bool savePosition(const std::string& path,Position p) {
  auto target=file(path),temp=target+".tmp";
  { std::ofstream out(temp,std::ios::trunc); out<<"VR2\n"<<path<<'\n'<<p.chapter<<' '<<p.page<<' '<<p.offset<<' '<<p.hasOffset<<'\n'; out.flush(); if(!out)return false; }
  // POSIX rename replaces atomically; retain the old checkpoint on failure.
  return replaceFile(temp,target);
}
}
