#include "reader/layout.hpp"
#include "reader/settings.hpp"
#include "rendering/page_style.hpp"
#include <cassert>
#include <iostream>
using namespace vr;
int main() {
  auto measure=[](const std::string& s,float) { int n=0; for(unsigned char c:s)if((c&0xc0)!=0x80)++n; return n*10; };
  Chapter c; Block b; b.runs.push_back({"é è ê à ù ç œ\nabcdefghijklmnopqrst",false,false}); c.blocks.push_back(b);
  Layout l; l.width=50; l.height=60; l.lineHeight=20; l.scale=1;
  auto pages=paginate(c,l,measure); assert(pages.size()>1);
  std::string letters;
  for(auto& p:pages)for(auto& line:p.lines) {
    assert(line.y<=60); assert(measure(line.text,1)<=50);
    if(!line.text.empty())assert((static_cast<unsigned char>(line.text.front())&0xc0)!=0x80);
    for(unsigned char ch:line.text)if(ch>='a'&&ch<='z')letters+=char(ch);
  }
  assert(letters=="abcdefghijklmnopqrst");
  for(std::size_t i=1;i<pages.size();++i)assert(pages[i].offset>pages[i-1].offset);
  auto anchor=pages[1].offset;
  auto wider=l; wider.width=100;
  auto reflowed=paginate(c,wider,measure);
  auto resumed=pageForOffset(reflowed,anchor);
  assert(reflowed[resumed].offset<=anchor);
  assert(resumed+1==reflowed.size()||reflowed[resumed+1].offset>anchor);
  auto height=[](float scale){return int(20*scale);};
  for(auto mode:{ReadingMode::Book,ReadingMode::Clean,ReadingMode::Oled})
    for(int size=0;size<3;++size)for(int margin=0;margin<3;++margin)for(int spacing=0;spacing<3;++spacing)
      for(auto bookLayout:{BookLayout::Single,BookLayout::Spread}) {
        ReadingSettings settings; settings.mode=mode; settings.fontSize=size; settings.margins=margin;
        settings.lineSpacing=spacing; settings.bookLayout=bookLayout;
        auto view=presentation(settings,height);
        assert(view.layout.width>0&&view.layout.height>=view.layout.lineHeight);
        if(mode==ReadingMode::Oled) { assert(view.theme.background==0xff000000u); assert(view.theme.paper==0xff000000u); assert(view.theme.text!=0xffffffffu); }
        if(mode!=ReadingMode::Book)assert(pageDecoration(view).empty());
        for(int i=0;i<view.pageCount;++i) {
          const auto area=view.content[i],paper=view.paper[i];
          assert(area.x>=paper.x&&area.y>=paper.y);
          assert(area.x+area.width<=paper.x+paper.width);
          assert(area.y+area.height<=paper.y+paper.height);
        }
        if(view.pageCount==2)assert(view.content[0].x+view.content[0].width<view.content[1].x);
        if(mode==ReadingMode::Book&&size==2&&bookLayout==BookLayout::Spread)assert(view.pageCount==1&&view.spreadLimited);
      }
  ReadingSettings settings; assert(adjustSetting(settings,0,-1)); assert(settings.mode==ReadingMode::Oled);
  assert(!adjustSetting(settings,1,1));
  settings.mode=ReadingMode::Book; assert(adjustSetting(settings,1,1)); assert(settings.bookLayout==BookLayout::Spread);
  assert(paginate(Chapter{},l,measure).size()==1);
  bool rejected=false; try { l.width=0; paginate(c,l,measure); } catch(...) { rejected=true; } assert(rejected);
  std::cout<<"layout: UTF-8, reflow anchors, 162 setting combinations, OLED black, gutters and readability fallback OK\n";
}
