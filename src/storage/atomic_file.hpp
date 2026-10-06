#pragma once
#include <cstdio>
#include <string>
#ifdef VITAREADER_VITA
#include <psp2/io/fcntl.h>
#endif
#ifdef _WIN32
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>
#endif
namespace vr {
inline bool replaceFile(const std::string& temporary,const std::string& destination) {
#ifdef VITAREADER_VITA
  if(sceIoRename(temporary.c_str(),destination.c_str())>=0)return true;
  const auto backup=destination+".bak";
  sceIoRemove(backup.c_str());
  if(sceIoRename(destination.c_str(),backup.c_str())<0)return false;
  if(sceIoRename(temporary.c_str(),destination.c_str())<0) {
    sceIoRename(backup.c_str(),destination.c_str()); return false;
  }
  sceIoRemove(backup.c_str()); return true;
#elif defined(_WIN32)
  // Windows CRT rename cannot replace an existing checkpoint. Host tests use ASCII paths.
  return MoveFileExA(temporary.c_str(),destination.c_str(),MOVEFILE_REPLACE_EXISTING|MOVEFILE_WRITE_THROUGH)!=0;
#else
  return std::rename(temporary.c_str(),destination.c_str())==0;
#endif
}
}
