#pragma once
#include "reader/settings.hpp"
#include <array>
namespace vr {
enum class TouchPhase { None, Down, Move, Up, Cancel };
struct TouchPoint { TouchPhase phase=TouchPhase::None; int id=-1; float x=0,y=0; };
struct Contact { int id; float x,y; };
class TouchTracker {
public:
  TouchPoint sample(const Contact* contacts,int count,bool valid=true);
private: int id_=-1; float x_=0,y_=0; bool blocked_=false;
};
int pageEdge(const Presentation& p,const TouchPoint& touch);
}
