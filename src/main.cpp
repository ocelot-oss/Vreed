#include "app/app.hpp"
#include <psp2/kernel/processmgr.h>
#include <cstdio>
#include <exception>
int main() {
  int status=0;
  try { vr::App app; app.run(); } catch(const std::exception& e) { std::fprintf(stderr,"VitaReader: %s\n",e.what()); status=1; }
  sceKernelExitProcess(status); return status;
}
