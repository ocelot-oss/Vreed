#pragma once
#include "reader/settings.hpp"
#include <array>
namespace vr {
struct CurlVertex { float x,y,u,v; };
struct CurlBand {
  std::array<CurlVertex,4> vertices{};
  bool backside=false;
  float shade=1;
};
struct CurlGeometry {
  std::array<CurlBand,interaction::curlSegments> bands{};
  std::array<Rect,interaction::shadowBands> shadows{};
  float shadowStrength=0;
};
// Coordinates refer to full-scene page surfaces; normalized UVs are independent of content type.
CurlGeometry curlGeometry(const Presentation& presentation,int direction,float progress,float corner=0.5f);
}
