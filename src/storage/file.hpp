#pragma once
#include <string>
#include <fstream>
#ifdef VITAREADER_VITA
#include <psp2/io/fcntl.h>
#endif
namespace vr {
inline bool readFile(const std::string& path,std::string& out) {
#ifdef VITAREADER_VITA
  const int fd=sceIoOpen(path.c_str(),SCE_O_RDONLY,0); if(fd<0)return false;
  out.clear(); char buffer[1024]; int n;
  while((n=sceIoRead(fd,buffer,sizeof buffer))>0) { out.append(buffer,n); if(out.size()>4096) { sceIoClose(fd); return false; } }
  sceIoClose(fd); return n==0;
#else
  std::ifstream in(path); if(!in)return false;
  out.assign(std::istreambuf_iterator<char>(in),{}); return out.size()<=4096;
#endif
}
inline bool writeFile(const std::string& path,const std::string& data) {
#ifdef VITAREADER_VITA
  const int fd=sceIoOpen(path.c_str(),SCE_O_WRONLY|SCE_O_CREAT|SCE_O_TRUNC,0666); if(fd<0)return false;
  std::size_t offset=0; bool good=true;
  while(offset<data.size()) { const int n=sceIoWrite(fd,data.data()+offset,data.size()-offset); if(n<=0) { good=false; break; } offset+=n; }
  return sceIoClose(fd)>=0&&good;
#else
  std::ofstream out(path,std::ios::trunc); out<<data; out.flush(); return bool(out);
#endif
}
}
