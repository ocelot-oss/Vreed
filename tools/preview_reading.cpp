// Off-device visual review using the same theme, geometry, decoration and pagination.
// Host font metrics are approximate; output is not a Vita screenshot.
#include "rendering/page_style.hpp"
#include <filesystem>
#include <fstream>
#include <iomanip>
#include <sstream>
using namespace vr;
namespace {
std::string color(std::uint32_t value) {
  std::ostringstream out; out<<'#'<<std::hex<<std::setfill('0')<<std::setw(2)<<(value&255)<<std::setw(2)<<((value>>8)&255)<<std::setw(2)<<((value>>16)&255); return out.str();
}
std::string escape(const std::string& value) {
  std::string result; for(char c:value) { if(c=='&')result+="&amp;"; else if(c=='<')result+="&lt;"; else if(c=='>')result+="&gt;"; else result+=c; } return result;
}
void rect(std::ostream& out,const PaintRect& paint) {
  const auto r=paint.bounds;
  out<<"<rect x='"<<r.x<<"' y='"<<r.y<<"' width='"<<r.width<<"' height='"<<r.height<<"' fill='"<<color(paint.color)<<"' fill-opacity='"<<double(paint.color>>24)/255<<"'/>\n";
}
void text(std::ostream& out,int x,int y,const std::string& value,int size,std::uint32_t ink,const char* align="start") {
  out<<"<text x='"<<x<<"' y='"<<y<<"' font-family='Arial, sans-serif' font-size='"<<size<<"' text-anchor='"<<align<<"' fill='"<<color(ink)<<"'>"<<escape(value)<<"</text>\n";
}
}
int main(int argc,char** argv) {
  if(argc!=2)return 1;
  std::filesystem::create_directories(argv[1]);
  Chapter chapter; Block heading; heading.kind=Block::Kind::Heading; heading.runs.push_back({"Le départ"}); chapter.blocks.push_back(heading);
  for(int i=0;i<14;++i) {
    Block block; block.runs.push_back({"À la fenêtre, une lumière douce éclaire le carnet. Éléonore écoute la rivière et prépare son voyage. Elle relit les mots laissés la veille, puis referme doucement la porte. Au loin, le jour commence."}); chapter.blocks.push_back(block);
  }
  for(int variant=0;variant<4;++variant) {
    ReadingSettings settings;
    if(variant==1)settings.bookLayout=BookLayout::Spread;
    if(variant==2)settings.mode=ReadingMode::Clean;
    if(variant==3)settings.mode=ReadingMode::Oled;
    auto p=presentation(settings,[](float scale){return int(20*scale);});
    auto pages=paginate(chapter,p.layout,[](const std::string& value,float scale) { int width=0; for(unsigned char c:value)if((c&0xc0)!=0x80)width+=c==' '?5:10; return int(width*scale); });
    const char* names[]={"book-single","book-spread","clean","oled-night"};
    std::ofstream out(std::filesystem::path(argv[1])/(std::string(names[variant])+".svg"));
    out<<"<svg xmlns='http://www.w3.org/2000/svg' width='960' height='544' viewBox='0 0 960 544'>\n";
    rect(out,{{0,0,960,544},p.theme.background});
    for(const auto& paint:pageDecoration(p))rect(out,paint);
    for(int i=0;i<p.pageCount;++i) {
      const auto paper=p.paper[i],area=p.content[i];
      out<<"<defs><clipPath id='page"<<i<<"'><rect x='"<<area.x<<"' y='"<<area.y<<"' width='"<<area.width<<"' height='"<<area.height<<"'/></clipPath></defs>";
      text(out,paper.x+paper.width/2,p.theme.physical?paper.y+33:35,"Le carnet de lecture",15,p.theme.secondary,"middle");
      out<<"<g clip-path='url(#page"<<i<<")'>";
      for(const auto& line:pages[i].lines)text(out,area.x,area.y+line.y,line.text,int(20*line.scale),p.theme.text);
      out<<"</g>";
      text(out,p.pageCount==2?(i==0?area.x:area.x+area.width):480,p.theme.physical?paper.y+paper.height-24:510,
           "1 · "+std::to_string(i+1)+" / "+std::to_string(pages.size()),15,p.theme.secondary,p.pageCount==2?(i==0?"start":"end"):"middle");
    }
    if(p.theme.physical)for(const auto& paint:bookmarkRibbon(p.paper[p.pageCount-1]))rect(out,paint);
    out<<"</svg>";
  }
}
