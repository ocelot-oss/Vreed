#pragma once
#include "reader/settings.hpp"
#include <vector>
namespace vr {
struct PaintRect { Rect bounds; std::uint32_t color; };
std::vector<PaintRect> pageDecoration(const Presentation& presentation);
// Separate component; not baked into paper. Coordinates can be animated later.
std::vector<PaintRect> bookmarkRibbon(const Rect& paper);
}
