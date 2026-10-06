#include "rendering/page_style.hpp"
namespace vr {
namespace {
std::uint32_t rgba(int r,int g,int b,int a) { return (std::uint32_t(a)<<24)|(b<<16)|(g<<8)|r; }
}
std::vector<PaintRect> pageDecoration(const Presentation& p) {
  std::vector<PaintRect> out;
  if(!p.theme.physical)return out;
  out.reserve(300);
  for(int i=0;i<p.pageCount;++i) {
    const auto r=p.paper[i];
    for(int s=8;s>0;--s)out.push_back({{r.x-s,r.y+s,r.width+2*s,r.height+2},rgba(0,0,0,8)});
    out.push_back({{r.x,r.y+3,r.width,r.height},rgba(177,157,125,255)});
    out.push_back({r,p.theme.paper});
    out.push_back({{r.x+1,r.y+1,r.width-2,1},rgba(255,253,239,160)});
    out.push_back({{r.x+1,r.y+2,1,r.height-3},rgba(255,253,239,110)});
    // Deterministic, sparse, almost invisible grain: no texture assets or random work per frame.
    if(p.theme.texture) {
      std::uint32_t seed=0x65431u+std::uint32_t(i);
      for(int n=0;n<90;++n) {
        seed=1664525u*seed+1013904223u; const int x=2+int(seed%std::uint32_t(r.width-4));
        seed=1664525u*seed+1013904223u; const int y=2+int(seed%std::uint32_t(r.height-4));
        out.push_back({{r.x+x,r.y+y,2,1},rgba(113,88,51,12)});
      }
    }
    if(p.pageCount==2) {
      for(int s=0;s<20;++s) {
        const int x=i==0?r.x+r.width-1-s:r.x+s;
        out.push_back({{x,r.y,1,r.height},rgba(66,44,22,(20-s)*3)});
      }
      out.push_back({{i==0?r.x+r.width-1:r.x,r.y,1,r.height},rgba(59,44,28,110)});
    } else {
      for(int s=0;s<8;++s)out.push_back({{r.x+s,r.y,1,r.height},rgba(95,71,39,(8-s)*2)});
    }
  }
  return out;
}
std::vector<PaintRect> bookmarkRibbon(const Rect& paper) {
  const int x=paper.x+paper.width-54,y=paper.y-3,w=19,h=57;
  std::vector<PaintRect> out;
  out.push_back({{x+2,y+4,w,h},rgba(30,15,12,40)});
  out.push_back({{x,y,w,h},rgba(132,38,41,255)});
  out.push_back({{x,y,2,h},rgba(185,70,66,255)});
  out.push_back({{x+w-2,y,2,h},rgba(94,27,31,255)});
  // Split lower edge into two tapering tips, leaving a real notch.
  for(int n=0;n<9;++n) {
    out.push_back({{x,y+h+n,9-n,1},rgba(132,38,41,255)});
    out.push_back({{x+10+n,y+h+n,9-n,1},rgba(117,32,37,255)});
  }
  return out;
}
}
