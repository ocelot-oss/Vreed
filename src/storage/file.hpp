#pragma once
#include <string>
#include <fstream>
#include <cstdio>
#ifdef VITAREADER_VITA
#include <psp2/io/fcntl.h>
#include <psp2/io/stat.h>
#endif
namespace vr {
inline std::string storageFailure;
inline bool fileFailure(const char* operation,const std::string& path,int code) {
  char value[32]; std::snprintf(value,sizeof value,"%s 0x%08X ",operation,unsigned(code));
  storageFailure=value+path.substr(path.find_last_of('/')+1); return false;
}
#ifdef VITAREADER_VITA
inline bool ensureParent(const std::string& path) {
  const auto colon=path.find(':'); if(colon==std::string::npos)return fileFailure("path",path,-1);
  for(auto slash=path.find('/',colon+2);slash!=std::string::npos;slash=path.find('/',slash+1)) {
    const auto dir=path.substr(0,slash); const int result=sceIoMkdir(dir.c_str(),0777);
    if(result<0) { SceIoStat stat{}; if(sceIoGetstat(dir.c_str(),&stat)<0||!SCE_S_ISDIR(stat.st_mode))return fileFailure("mkdir",dir,result); }
  }
  return true;
}
#endif
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
  storageFailure.clear(); if(!ensureParent(path))return false;
  const int fd=sceIoOpen(path.c_str(),SCE_O_WRONLY|SCE_O_CREAT|SCE_O_TRUNC,0777); if(fd<0)return fileFailure("open",path,fd);
  std::size_t offset=0;
  while(offset<data.size()) { const int n=sceIoWrite(fd,data.data()+offset,data.size()-offset); if(n<=0) { sceIoClose(fd); return fileFailure("write",path,n); } offset+=n; }
  const int closed=sceIoClose(fd); if(closed<0)return fileFailure("close",path,closed); return true;
#else
  std::ofstream out(path,std::ios::trunc); out<<data; out.flush(); return bool(out);
#endif
}
}
