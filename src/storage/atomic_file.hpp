#pragma once
#include <cstdio>
#include <string>
#ifdef _WIN32
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>
#endif
namespace vr {
inline bool replaceFile(const std::string& temporary,const std::string& destination) {
#ifdef _WIN32
  // Windows CRT rename cannot replace an existing checkpoint. Host tests use ASCII paths.
  return MoveFileExA(temporary.c_str(),destination.c_str(),MOVEFILE_REPLACE_EXISTING|MOVEFILE_WRITE_THROUGH)!=0;
#else
  return std::rename(temporary.c_str(),destination.c_str())==0;
#endif
}
}
