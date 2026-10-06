#include "platform/vita.hpp"
#include <psp2/ctrl.h>
#include <psp2/kernel/processmgr.h>
#include <psp2/touch.h>
#include <algorithm>
namespace vr {
Vita::Vita() {
  sceCtrlSetSamplingMode(SCE_CTRL_MODE_ANALOG);
  SceTouchPanelInfo panel{};
  if(sceTouchGetPanelInfo(SCE_TOUCH_PORT_FRONT,&panel)>=0&&panel.maxDispX>panel.minDispX&&panel.maxDispY>panel.minDispY&&
     sceTouchSetSamplingState(SCE_TOUCH_PORT_FRONT,SCE_TOUCH_SAMPLING_STATE_START)>=0) {
    minX_=panel.minDispX; minY_=panel.minDispY;
    rangeX_=panel.maxDispX-panel.minDispX; rangeY_=panel.maxDispY-panel.minDispY; touchReady_=true;
  }
}
double Vita::nowMs() const { return double(sceKernelGetProcessTimeWide())/1000; }
Input Vita::poll() {
  SceCtrlData pad{}; const bool controller=sceCtrlPeekBufferPositive(0,&pad,1)>0;
  auto bits=pad.buttons;
  if(controller) {
    if(pad.ly<64)bits|=SCE_CTRL_UP; if(pad.ly>192)bits|=SCE_CTRL_DOWN;
    if(pad.lx<64)bits|=SCE_CTRL_LEFT; if(pad.lx>192)bits|=SCE_CTRL_RIGHT;
  }
  auto edge=bits&~previous_; previous_=bits;
  auto directions=bits&(SCE_CTRL_UP|SCE_CTRL_DOWN|SCE_CTRL_LEFT|SCE_CTRL_RIGHT|SCE_CTRL_LTRIGGER|SCE_CTRL_RTRIGGER);
  auto now=sceKernelGetProcessTimeWide();
  if(directions!=held_) { held_=directions; repeatAt_=now+350000; }
  else if(directions&&now>=repeatAt_) { edge|=directions; repeatAt_=now+110000; }
  Input in;
  in.nowMs=double(now)/1000;
  if(touchReady_) {
    SceTouchData data{}; const int received=sceTouchPeek(SCE_TOUCH_PORT_FRONT,&data,1); const bool valid=received>0;
    std::array<Contact,SCE_TOUCH_MAX_REPORT> contacts{};
    const int count=valid?std::min(int(data.reportNum),SCE_TOUCH_MAX_REPORT):0;
    for(int i=0;i<count;++i)contacts[i]={data.report[i].id,
      std::clamp((data.report[i].x-minX_)*959/rangeX_,0.0f,959.0f),
      std::clamp((data.report[i].y-minY_)*543/rangeY_,0.0f,543.0f)};
    if(received!=0)in.touch=touch_.sample(contacts.data(),count,valid);
  }
  in.up=edge&SCE_CTRL_UP; in.down=edge&SCE_CTRL_DOWN;
  in.previous=edge&(SCE_CTRL_LEFT|SCE_CTRL_LTRIGGER); in.next=edge&(SCE_CTRL_RIGHT|SCE_CTRL_RTRIGGER);
  in.open=edge&SCE_CTRL_CROSS; in.back=edge&SCE_CTRL_CIRCLE; in.rescan=edge&SCE_CTRL_TRIANGLE;
  in.quit=edge&SCE_CTRL_START; in.bookmark=edge&SCE_CTRL_SQUARE; return in;
}
}
