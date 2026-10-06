#include "rendering/curl.hpp"
#include <algorithm>
#include <cmath>
namespace vr {
CurlGeometry curlGeometry(const Presentation& p,int direction,float progress,float corner) {
  CurlGeometry result;
  progress=std::clamp(progress,0.0f,1.0f); corner=std::clamp(corner,0.0f,1.0f);
  const int sign=direction>0?1:-1;
  const int index=p.pageCount==2?(direction>0?1:0):0;
  const auto page=p.paper[index];
  const float hinge=direction>0?float(page.x):float(page.x+page.width);
  const float angle=progress*3.14159265f,c=std::cos(angle),lift=std::max(0.0f,std::sin(angle));
  auto vertex=[&](float s,bool bottom) {
    const float bow=std::sin(s*3.14159265f)*lift;
    const float x=hinge+sign*(s*page.width*c+bow*page.width*0.10f);
    const float bend=bow*14;
    const float y=bottom?page.y+page.height+bend*corner:page.y-bend*(1-corner);
    float u=direction>0?page.x+s*page.width:page.x+(1-s)*page.width;
    if(c<0&&p.pageCount==2) {
      const auto reverse=p.paper[1-index];
      u=direction>0?reverse.x+(1-s)*reverse.width:reverse.x+s*reverse.width;
    }
    return CurlVertex{x,y,u/960.0f,(bottom?page.y+page.height:page.y)/544.0f};
  };
  for(int i=0;i<interaction::curlSegments;++i) {
    auto& band=result.bands[i]; const float a=float(i)/interaction::curlSegments,b=float(i+1)/interaction::curlSegments;
    band.vertices={vertex(a,false),vertex(b,false),vertex(a,true),vertex(b,true)};
    band.backside=c<0;
    band.shade=std::clamp(1-lift*(0.07f+0.15f*std::sin((a+b)*1.5707963f)),0.7f,1.0f);
  }
  const float edge=hinge+sign*page.width*c;
  result.shadowStrength=lift*0.22f;
  for(int i=0;i<interaction::shadowBands;++i)result.shadows[i]={int(edge)+sign*(i-4)*3,page.y+3,3,page.height-3};
  return result;
}
}
