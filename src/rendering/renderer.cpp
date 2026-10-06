#include "rendering/renderer.hpp"
#include <stdexcept>
#include <algorithm>
#include <cmath>
#include <psp2/sysmodule.h>
namespace vr {
Renderer::Renderer() {
  if(sceSysmoduleLoadModule(SCE_SYSMODULE_PGF)<0)throw std::runtime_error("PGF module unavailable");
  if(vita2d_init()<0) { sceSysmoduleUnloadModule(SCE_SYSMODULE_PGF); throw std::runtime_error("Graphics initialization failed"); }
  font_=vita2d_load_default_pgf();
  if(!font_) { vita2d_fini(); sceSysmoduleUnloadModule(SCE_SYSMODULE_PGF); throw std::runtime_error("System font unavailable"); }
}
Renderer::~Renderer() {
  vita2d_wait_rendering_done();
  if(source_)vita2d_free_texture(source_);
  if(destination_)vita2d_free_texture(destination_);
  vita2d_free_pgf(font_); vita2d_fini(); sceSysmoduleUnloadModule(SCE_SYSMODULE_PGF);
}
void Renderer::prepareLayout() { vita2d_wait_rendering_done(); } // Measurements may populate the shared glyph atlas.
Presentation Renderer::readingPresentation(const ReadingSettings& settings) const {
  return presentation(settings,[&](float scale){return vita2d_pgf_text_height(font_,scale,"Agéœ");});
}
void Renderer::configure(const Presentation& p) {
  auto paper=pageDecoration(p);
  auto ribbon=bookmarkRibbon(p.paper[p.pageCount-1]);
  presentation_=p; paper_=std::move(paper); ribbon_=std::move(ribbon);
}
void Renderer::paint(const std::vector<PaintRect>& rectangles) {
  for(const auto& rect:rectangles)vita2d_draw_rectangle(rect.bounds.x,rect.bounds.y,rect.bounds.width,rect.bounds.height,rect.color);
}
int Renderer::measure(const std::string& s,float scale) const { return vita2d_pgf_text_width(font_,scale,s.c_str()); }
void Renderer::text(int x,int y,const std::string& s,float scale,unsigned color) { vita2d_pgf_draw_text(font_,x,y,color,scale,s.c_str()); }
std::string Renderer::fit(const std::string& s,int width,float scale) const {
  if(measure(s,scale)<=width)return s; std::string out=s;
  while(!out.empty()&&measure(out+"...",scale)>width) { auto i=out.size()-1; while(i>0&&(static_cast<unsigned char>(out[i])&0xc0)==0x80)--i; out.resize(i); } return out+"...";
}
void Renderer::begin(bool reading) { reading_=reading; vita2d_set_clear_color(reading?presentation_.theme.background:RGBA8(15,20,25,255)); vita2d_start_drawing(); vita2d_clear_screen(); }
void Renderer::finish() { vita2d_end_drawing(); vita2d_swap_buffers(); }
void Renderer::library(const std::vector<Book>& books,std::size_t selected) {
  text(42,48,"Bibliothèque",1.65f); text(42,82,std::to_string(books.size())+" livres",0.9f,RGBA8(148,161,174,255));
  if(books.empty()) { text(42,210,"Ajoutez vos fichiers EPUB dans :",1.2f); text(42,252,"ux0:/data/VitaReader/books",1.1f); }
  auto first=(selected/6)*6;
  for(std::size_t i=first;i<books.size()&&i<first+6;++i) {
    int y=108+static_cast<int>(i-first)*61;
    if(i==selected)vita2d_draw_rectangle(32,y,896,55,RGBA8(36,55,68,255));
    vita2d_draw_rectangle(44,y+10,4,34,RGBA8(93,177,203,255));
    text(64,y+26,fit(books[i].title,810,1.15f),1.15f);
    text(64,y+47,books[i].position.chapter||books[i].position.page?"Lecture en cours":"EPUB",0.8f,RGBA8(148,161,174,255));
  }
  text(42,525,"X Ouvrir    O Retour    Triangle Actualiser    START Quitter",0.9f,RGBA8(148,161,174,255));
}
void Renderer::drawPage(const Page& page,int index,Position position,std::size_t count,std::size_t chapters) {
  const auto area=presentation_.content[index];
  // Each page clips to its own rectangle; glyph bearings cannot enter the gutter.
  vita2d_set_clip_rectangle(area.x,area.y,area.x+area.width,area.y+area.height);
  vita2d_enable_clipping();
  for(const auto& line:page.lines)text(area.x,area.y+line.y,line.text,line.scale,presentation_.theme.text);
  vita2d_disable_clipping();
  std::string label=std::to_string(position.page+1)+" / "+std::to_string(count);
  if(presentation_.pageCount==2)label=std::to_string(position.chapter+1)+" · "+label;
  else label="Chapitre "+std::to_string(position.chapter+1)+" / "+std::to_string(chapters)+"   ·   "+label;
  const auto paper=presentation_.paper[index];
  const int width=measure(label,0.8f);
  const int x=presentation_.pageCount==2?(index==0?area.x:area.x+area.width-width):(960-width)/2;
  text(x,presentation_.theme.physical?paper.y+paper.height-24:510,label,0.8f,presentation_.theme.secondary);
}
void Renderer::reader(const Book& book,const Page& page,Position p,std::size_t count,std::size_t chapters,const Session::VisiblePage& second,float ribbon) {
  paint(paper_);
  for(int i=0;i<presentation_.pageCount;++i) {
    const auto paper=presentation_.paper[i];
    const auto area=presentation_.content[i];
    const auto title=fit(book.title,area.width-32,0.78f);
    text(paper.x+(paper.width-measure(title,0.78f))/2,presentation_.theme.physical?paper.y+33:35,title,0.78f,presentation_.theme.secondary);
  }
  drawPage(page,0,p,count,chapters);
  if(presentation_.pageCount==2&&second.page)drawPage(*second.page,1,second.position,second.count,chapters);
  drawRibbon(ribbon);
}
void Renderer::drawRibbon(float amount) {
  if(!presentation_.theme.physical||amount<=0)return;
  const auto sheet=presentation_.paper[presentation_.pageCount-1];
  const int top=sheet.y-3,offset=-int((1-std::clamp(amount,0.0f,1.0f))*70);
  for(const auto& paint:ribbon_) {
    auto r=paint.bounds; r.y+=offset;
    if(r.y+r.height<=top)continue;
    if(r.y<top) { r.height-=top-r.y; r.y=top; }
    vita2d_draw_rectangle(r.x,r.y,r.width,r.height,paint.color);
  }
}
void Renderer::boundaryFeedback(int direction,float amount) {
  if(!presentation_.theme.physical||amount<=0)return;
  const auto r=presentation_.paper[direction>0?presentation_.pageCount-1:0];
  vita2d_draw_rectangle(direction>0?r.x+r.width-3:r.x,r.y+5,2,r.height-10,RGBA8(255,240,202,int(amount*110)));
}
bool Renderer::ensureSurfaces() {
  if(source_&&destination_)return true;
  if(surfacesFailed_)return false;
  source_=vita2d_create_empty_texture_rendertarget(960,544,SCE_GXM_TEXTURE_FORMAT_A8B8G8R8);
  destination_=vita2d_create_empty_texture_rendertarget(960,544,SCE_GXM_TEXTURE_FORMAT_A8B8G8R8);
  if(!source_||!destination_) {
    if(source_)vita2d_free_texture(source_);
    if(destination_)vita2d_free_texture(destination_);
    source_=destination_=nullptr;
    surfacesFailed_=true; return false;
  }
  return true;
}
void Renderer::capture(vita2d_texture* target,const Book& book,Session& session,const Session::VisiblePage& second) {
  vita2d_set_clear_color(presentation_.theme.background);
  vita2d_start_drawing_advanced(target,0); vita2d_clear_screen();
  reader(book,session.page(),session.position,session.pageCount(),session.chapterCount(),second,0);
  vita2d_end_drawing(); vita2d_wait_rendering_done();
}
bool Renderer::captureTurn(const Book& book,Session& current,const Session::VisiblePage& currentSecond,Session& next,const Session::VisiblePage& nextSecond) {
  vita2d_wait_rendering_done();
  if(!ensureSurfaces())return false;
  capture(source_,book,current,currentSecond); capture(destination_,book,next,nextSecond); return true;
}
void Renderer::transition(const PageTransition& motion) {
  if(!source_||!destination_)return;
  const float progress=motion.progress(); const int direction=motion.direction();
  if(motion.kind()==TransitionKind::Slide) {
    vita2d_draw_texture(source_,-direction*progress*960,0);
    vita2d_draw_texture(destination_,direction*(1-progress)*960,0); return;
  }
  if(motion.kind()!=TransitionKind::Curl) { vita2d_draw_texture(source_,0,0); return; }
  const float p=motion.state()==TurnState::Dragging?std::max(0.012f,progress):progress;
  vita2d_draw_texture(destination_,0,0);
  if(presentation_.pageCount==2&&p<0.5f) {
    const auto stationary=presentation_.paper[direction>0?0:1];
    vita2d_draw_texture_part(source_,stationary.x,stationary.y,stationary.x,stationary.y,stationary.width,stationary.height);
  }
  const auto geometry=curlGeometry(presentation_,direction,p,motion.corner());
  for(int i=0;i<interaction::shadowBands;++i) {
    const auto r=geometry.shadows[i]; const float weight=1-std::abs(i-3.5f)/4;
    vita2d_draw_rectangle(r.x,r.y,r.width,r.height,RGBA8(39,28,16,int(255*geometry.shadowStrength*weight)));
  }
  for(const auto& band:geometry.bands) {
    const unsigned shade=static_cast<unsigned>(band.shade*255);
    if(band.backside&&presentation_.pageCount==1) {
      auto* vertices=static_cast<vita2d_color_vertex*>(vita2d_pool_memalign(4*sizeof(vita2d_color_vertex),16));
      if(!vertices)continue;
      const auto paper=presentation_.theme.paper;
      const auto tint=RGBA8(int((paper&255)*band.shade),int(((paper>>8)&255)*band.shade),int(((paper>>16)&255)*band.shade),255);
      for(int i=0;i<4;++i)vertices[i]={band.vertices[i].x,band.vertices[i].y,0.5f,tint};
      vita2d_draw_array(SCE_GXM_PRIMITIVE_TRIANGLE_STRIP,vertices,4);
    } else {
      auto* vertices=static_cast<vita2d_texture_vertex*>(vita2d_pool_memalign(4*sizeof(vita2d_texture_vertex),16));
      if(!vertices)continue;
      for(int i=0;i<4;++i)vertices[i]={band.vertices[i].x,band.vertices[i].y,0.5f,band.vertices[i].u,band.vertices[i].v};
      vita2d_draw_array_textured(band.backside?destination_:source_,SCE_GXM_PRIMITIVE_TRIANGLE_STRIP,vertices,4,RGBA8(shade,shade,shade,255));
    }
  }
  // A restrained moving edge highlight, not a black fold band.
  const auto& edge=geometry.bands.back().vertices;
  vita2d_draw_line(edge[1].x,edge[1].y,edge[3].x,edge[3].y,RGBA8(255,246,219,int(120*geometry.shadowStrength)));
}
void Renderer::settings(const ReadingSettings& settings,int selected) {
  const bool oled=settings.mode==ReadingMode::Oled;
  const unsigned bg=oled?RGBA8(0,0,0,255):RGBA8(20,26,31,250);
  const unsigned fg=oled?RGBA8(184,187,183,255):RGBA8(228,230,228,255);
  const unsigned dim=oled?RGBA8(89,96,98,255):RGBA8(137,151,159,255);
  const unsigned accent=oled?presentation_.theme.accent:
    settings.mode==ReadingMode::Book?RGBA8(218,172,129,255):RGBA8(139,182,202,255);
  vita2d_draw_rectangle(245,72,470,406,bg);
  vita2d_draw_rectangle(245,72,470,1,dim);
  text(269,111,"Lecture",1.2f,fg);
  const char* labels[]={"Mode","Disposition","Taille de police","Interligne","Marges","Animation","Son de page","Volume"};
  for(int row=0;row<settingsRowCount;++row) {
    const int y=135+row*34;
    if(row==selected) {
      if(!oled)vita2d_draw_rectangle(257,y-3,446,30,RGBA8(36,48,56,255));
      vita2d_draw_rectangle(258,y+3,2,21,accent);
    }
    text(274,y+20,labels[row],0.95f,row==1&&settings.mode!=ReadingMode::Book?dim:fg);
    const auto value=settingValue(settings,row);
    text(690-measure(value,0.9f),y+20,value,0.9f,row==selected?accent:dim);
  }
  text(269,444,presentation_.spreadLimited?"Grande police : une page pour rester lisible":"Gauche / droite : modifier",0.75f,dim);
  text(269,468,"O ou Triangle : fermer",0.75f,dim);
}
void Renderer::message(const std::string& s) {
  const bool oled=reading_&&presentation_.theme.background==RGBA8(0,0,0,255);
  vita2d_draw_rectangle(24,458,912,38,oled?RGBA8(0,0,0,255):RGBA8(108,39,39,255));
  text(40,483,fit(s,870,0.85f),0.85f,oled?RGBA8(154,110,106,255):RGBA8(232,232,232,255));
}
}
