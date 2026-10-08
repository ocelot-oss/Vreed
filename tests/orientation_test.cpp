#include "input/orientation.hpp"
#include "rendering/curl.hpp"
#include "storage/settings.hpp"
#include <fstream>
#include <stdexcept>
#include <cmath>
static void require(bool v) {if(!v)throw std::runtime_error("Portrait regression");}
int main(int argc,char** argv) {
  using namespace vr;
  if(argc!=2)return 2;
  for(auto orientation:{Orientation::VerticalRight,Orientation::VerticalLeft}) {
    ReadingSettings s; s.orientation=orientation; s.bookLayout=BookLayout::Spread;
    auto p=presentation(s,[](float){return 30;});
    require(p.width==544&&p.height==960&&p.pageCount==1&&p.layout.height>700);
    require(p.content[0].x+p.content[0].width<=544&&p.content[0].y+p.content[0].height<=960);
    auto track=pageChooserTrack(p); require(track.x>=0&&track.x+track.width<=544&&track.y>0&&track.y<960);
    const auto geo=curlGeometry(p,1,0,0.5f);
    for(auto band:geo.bands)for(auto vertex:band.vertices) {
      const auto physical=screenPoint({vertex.x,vertex.y},orientation);
      require(std::abs(vertex.u*960-physical.x)<0.01f&&std::abs(vertex.v*544-physical.y)<0.01f);
    }
    Input in; in.up=true; in.triggerNext=true; in.touch={TouchPhase::Down,1,0,0}; orientInput(in,orientation);
    require(in.next); require(in.touch.x>=0&&in.touch.x<544&&in.touch.y>=0&&in.touch.y<960);
    require(orientation==Orientation::VerticalRight?in.previous:!in.previous);
    require(saveSettings(argv[1],s)); require(loadSettings(argv[1]).orientation==orientation);
    s.orientation=Orientation::Horizontal;
    require(presentation(s,[](float){return 30;}).pageCount==2);
  }
  {std::ofstream f(argv[1]); f<<"VRS2\n0 1 1 1 1 2 1 1 1 1 2 0\n";}
  auto migrated=loadSettings(argv[1]); require(migrated.orientation==Orientation::Horizontal&&migrated.fontFamily==2);
  {std::ofstream f(argv[1]); f<<"VRS3\n0 1 1 1 1 2 1 1 1 1 2 0 99\n";}
  require(loadSettings(argv[1]).orientation==Orientation::Horizontal);
}
