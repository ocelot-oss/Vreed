#include "reader/layout.hpp"
#include <stdexcept>
namespace vr {
std::size_t pageForOffset(const std::vector<Page>& pages,std::size_t offset) {
  if(pages.empty())throw std::runtime_error("No reader pages");
  std::size_t index=0;
  while(index+1<pages.size()&&pages[index+1].offset<=offset)++index;
  return index;
}
std::vector<Page> paginate(const Chapter& chapter,const Layout& l,const Measure& measure) {
  if(l.width<=0||l.height<l.lineHeight||l.lineHeight<=0)throw std::runtime_error("Invalid viewport");
  std::vector<Page> pages(1); int y=0; std::size_t base=0;
  auto emit=[&](const std::string& text,float scale,std::size_t offset) {
    if(y+l.lineHeight>l.height) { pages.emplace_back(); y=0; }
    if(pages.back().lines.empty())pages.back().offset=offset;
    pages.back().lines.push_back({text,y+l.lineHeight,scale,offset}); y+=l.lineHeight;
  };
  for(const auto& block:chapter.blocks) {
    const float scale=block.kind==Block::Kind::Heading?l.scale*1.15f:l.scale;
    std::string text; for(const auto& run:block.runs)text+=run.text;
    std::string line; std::size_t lineOffset=base;
    for(std::size_t i=0;i<text.size();) {
      if(text[i]=='\n') { emit(line,scale,line.empty()?base+i:lineOffset); line.clear(); ++i; continue; }
      if(text[i]==' ') { ++i; continue; }
      const auto begin=i;
      while(i<text.size()&&text[i]!=' '&&text[i]!='\n')++i;
      const auto word=text.substr(begin,i-begin);
      auto candidate=line+(line.empty()?"":" ")+word;
      if(measure(candidate,scale)<=l.width) { if(line.empty())lineOffset=base+begin; line=std::move(candidate); continue; }
      if(!line.empty()) { emit(line,scale,lineOffset); line.clear(); }
      for(std::size_t j=0;j<word.size();) {
        std::size_t end=j+1;
        while(end<word.size()&&(static_cast<unsigned char>(word[end])&0xc0)==0x80)++end;
        const auto glyph=word.substr(j,end-j);
        if(measure(glyph,scale)>l.width)throw std::runtime_error("Glyph exceeds page width");
        if(!line.empty()&&measure(line+glyph,scale)>l.width) { emit(line,scale,lineOffset); line.clear(); }
        if(line.empty())lineOffset=base+begin+j;
        line+=glyph; j=end;
      }
    }
    if(!line.empty())emit(line,scale,lineOffset);
    y+=10; base+=text.size()+1;
  }
  return pages;
}
}
