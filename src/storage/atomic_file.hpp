#pragma once
#include <cstdio>
#include <string>
#include "storage/file.hpp"
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
  const int moved=sceIoRename(destination.c_str(),backup.c_str());
  if(moved<0)return fileFailure("backup",destination,moved);
  const int installed=sceIoRename(temporary.c_str(),destination.c_str());
  if(installed<0) {
    sceIoRename(backup.c_str(),destination.c_str()); return fileFailure("rename",destination,installed);
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
