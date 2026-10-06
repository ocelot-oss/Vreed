#include "rendering/renderer.hpp"
#include "app/identity.hpp"
#include "rendering/typography.hpp"
#include <stdexcept>
#include <algorithm>
#include <cmath>
#include <png.h>
#include <set>
#include <cstring>
#include <psp2/sysmodule.h>
namespace vr {
Renderer::Renderer() {
  if(sceSysmoduleLoadModule(SCE_SYSMODULE_PGF)<0)throw std::runtime_error("PGF module unavailable");
  if(vita2d_init()<0) { sceSysmoduleUnloadModule(SCE_SYSMODULE_PGF); throw std::runtime_error("Graphics initialization failed"); }
  font_=vita2d_load_default_pgf();
  if(!font_) { vita2d_fini(); sceSysmoduleUnloadModule(SCE_SYSMODULE_PGF); throw std::runtime_error("System font unavailable"); }
  logo_=vita2d_load_PNG_file("app0:/assets/branding/logo.png");
  if(logo_) { logo_->gxm_rtgt=nullptr; logo_->depth_UID=0; vita2d_texture_set_filters(logo_,SCE_GXM_TEXTURE_FILTER_LINEAR,SCE_GXM_TEXTURE_FILTER_LINEAR); }
}
Renderer::~Renderer() {
  vita2d_wait_rendering_done();
  clearImages();
  if(logo_)vita2d_free_texture(logo_);
  if(source_)vita2d_free_texture(source_);
  if(destination_)vita2d_free_texture(destination_);
  vita2d_free_pgf(font_); vita2d_fini(); sceSysmoduleUnloadModule(SCE_SYSMODULE_PGF);
}
void Renderer::prepareLayout() { vita2d_wait_rendering_done(); } // Measurements may populate the shared glyph atlas.
void Renderer::clearImages() {
  for(auto& pair:images_)if(pair.second.texture)vita2d_free_texture(pair.second.texture);
  images_.clear();
}
void Renderer::prepareImages(std::initializer_list<const Page*> pages) {
  vita2d_wait_rendering_done(); std::set<std::string> wanted;
  for(auto* page:pages)if(page)for(const auto& item:page->images)wanted.insert(item.image->key);
  for(auto it=images_.begin();it!=images_.end();) {
    if(!wanted.count(it->first)) { if(it->second.texture)vita2d_free_texture(it->second.texture); it=images_.erase(it); } else ++it;
  }
  std::size_t used=0; for(const auto& pair:images_)used+=pair.second.bytes;
  for(auto* page:pages)if(page)for(const auto& item:page->images) {
    const auto& data=*item.image; if(images_.count(data.key))continue;
    ImageTexture loaded; const auto bytes=std::size_t(data.width)*data.height*4;
    if(used+bytes<=24*1024*1024) {
      if(data.png) {
        png_image image{}; image.version=PNG_IMAGE_VERSION;
        if(png_image_begin_read_from_memory(&image,data.bytes.data(),data.bytes.size())) {
          if(image.width==unsigned(data.width)&&image.height==unsigned(data.height)) {
            image.format=PNG_FORMAT_RGBA; std::vector<unsigned char> rgba(bytes);
            if(png_image_finish_read(&image,nullptr,rgba.data(),0,nullptr)) {
              loaded.texture=vita2d_create_empty_texture(data.width,data.height);
              if(loaded.texture) {
                auto* dest=static_cast<unsigned char*>(vita2d_texture_get_datap(loaded.texture));
                const auto stride=vita2d_texture_get_stride(loaded.texture);
                for(int y=0;y<data.height;++y)std::memcpy(dest+y*stride,rgba.data()+std::size_t(y)*data.width*4,data.width*4);
              }
            }
          }
          png_image_free(&image);
        }
      } else loaded.texture=vita2d_load_JPEG_buffer(data.bytes.data(),data.bytes.size());
      if(loaded.texture) {
        // Raster images own no render target/depth buffer. Some vita2d versions
        // leave these optional fields uninitialized on ordinary textures.
        loaded.texture->gxm_rtgt=nullptr; loaded.texture->depth_UID=0;
        loaded.bytes=bytes; used+=bytes;
        vita2d_texture_set_filters(loaded.texture,SCE_GXM_TEXTURE_FILTER_LINEAR,SCE_GXM_TEXTURE_FILTER_LINEAR);
      }
    }
    images_.emplace(data.key,loaded);
  }
}
Presentation Renderer::readingPresentation(const ReadingSettings& settings) const {
  return presentation(settings,[&](float scale){return vita2d_pgf_text_height(font_,scale,"Agéœ");});
}
void Renderer::configure(const Presentation& p) {
  clearImages();
  auto paper=pageDecoration(p);
  auto ribbon=bookmarkRibbon(p.paper[p.pageCount-1]);
  presentation_=p; paper_=std::move(paper); ribbon_=std::move(ribbon);
}
void Renderer::paint(const std::vector<PaintRect>& rectangles) {
  for(const auto& rect:rectangles)vita2d_draw_rectangle(rect.bounds.x,rect.bounds.y,rect.bounds.width,rect.bounds.height,rect.color);
}
int Renderer::measure(const std::string& s,float scale) const {
  return textAdvance(s,scale,[&](const std::string& piece,float size){return vita2d_pgf_text_width(font_,size,piece.c_str());},[](int,const std::string&){});
}
void Renderer::text(int x,int y,const std::string& s,float scale,unsigned color) {
  textAdvance(s,scale,[&](const std::string& piece,float size){return vita2d_pgf_text_width(font_,size,piece.c_str());},[&](int offset,const std::string& piece){vita2d_pgf_draw_text(font_,x+offset,y,color,scale,piece.c_str());});
}
std::string Renderer::fit(const std::string& s,int width,float scale) const {
  if(measure(s,scale)<=width)return s; std::string out=s;
  while(!out.empty()&&measure(out+"...",scale)>width) { auto i=out.size()-1; while(i>0&&(static_cast<unsigned char>(out[i])&0xc0)==0x80)--i; out.resize(i); } return out+"...";
}
void Renderer::begin(bool reading) { reading_=reading; vita2d_set_clear_color(reading?presentation_.theme.background:RGBA8(15,20,25,255)); vita2d_start_drawing(); vita2d_clear_screen(); }
void Renderer::finish() { vita2d_end_drawing(); vita2d_swap_buffers(); }
void Renderer::library(const std::vector<Book>& books,std::size_t selected) {
  vita2d_draw_rectangle(0,0,960,544,RGBA8(24,21,21,255));
  if(logo_)vita2d_draw_texture_scale(logo_,42,8,80.0f/vita2d_texture_get_width(logo_),80.0f/vita2d_texture_get_height(logo_));
  text(140,48,config::name,1.65f,RGBA8(239,223,190,255)); text(140,80,"Bibliothèque · "+std::to_string(books.size())+" livres",0.85f,RGBA8(164,151,132,255));
  for(int row=0;row<2;++row) {
    const int y=260+row*208;
    vita2d_draw_rectangle(28,y+14,904,14,RGBA8(0,0,0,90));
    vita2d_draw_rectangle(28,y,904,18,RGBA8(102,67,43,255));
    vita2d_draw_rectangle(28,y,904,2,RGBA8(157,111,71,255));
    for(int line=0;line<5;++line)vita2d_draw_rectangle(32,y+4+line*2,896,1,RGBA8(43,28,18,25));
  }
  if(books.empty()) { text(42,210,"Ajoutez vos fichiers EPUB dans :",1.2f); text(42,252,"ux0:/data/VitaReader/books",1.1f); }
  const unsigned colors[]={RGBA8(77,109,103,255u),RGBA8(139,72,59,255u),RGBA8(119,97,142,255u),RGBA8(154,121,61,255u),RGBA8(61,86,121,255u),RGBA8(114,91,76,255u)};
  auto first=(selected/20)*20;
  for(std::size_t i=first;i<books.size()&&i<first+20;++i) {
    const int slot=int(i-first),row=slot/10,x=48+(slot%10)*87,h=148+int((i*17)%35),bottom=260+row*208,y=bottom-h;
    const bool active=i==selected; unsigned seed=2166136261u; for(unsigned char ch:books[i].title)seed=(seed^ch)*16777619u;
    vita2d_draw_rectangle(x+5,y+4,77,h,RGBA8(0,0,0,100));
    if(active)vita2d_draw_rectangle(x-3,y-3,80,h+5,RGBA8(237,207,133,255));
    vita2d_draw_rectangle(x,y,74,h,colors[seed%6]);
    vita2d_draw_rectangle(x+2,y,3,h,RGBA8(255,244,219,36));
    vita2d_draw_rectangle(x+69,y,5,h,RGBA8(0,0,0,38));
    vita2d_draw_rectangle(x+8,y+13,56,1,RGBA8(239,218,173,160));
    std::string remaining=books[i].title;
    for(int line=0;line<5&&!remaining.empty();++line) {
      std::size_t end=remaining.size();
      while(end>0&&measure(remaining.substr(0,end),0.72f)>56) { --end; while(end>0&&(static_cast<unsigned char>(remaining[end])&0xc0)==0x80)--end; }
      if(!end)break; auto split=remaining.rfind(' ',end); if(split!=std::string::npos&&split>0)end=split;
      const auto label=remaining.substr(0,end); text(x+9,y+38+line*18,label,0.72f,RGBA8(249,239,216,255));
      remaining.erase(0,end); while(!remaining.empty()&&remaining.front()==' ')remaining.erase(0,1);
    }
    vita2d_draw_rectangle(x+8,bottom-27,56,1,RGBA8(239,218,173,150));
    text(x+10,bottom-9,books[i].position.resumePinned?"Signet":"EPUB",0.65f,RGBA8(242,224,186,255));
  }
  text(42,518,"X Ouvrir · Toucher pour sélectionner · Triangle Actualiser",0.8f,RGBA8(186,172,149,255));
}
void Renderer::drawPage(const Page& page,int index,Position position,std::size_t count,std::size_t chapters) {
  const auto area=presentation_.content[index];
  // Each page clips to its own rectangle; glyph bearings cannot enter the gutter.
  vita2d_set_clip_rectangle(area.x,area.y,area.x+area.width,area.y+area.height);
  vita2d_enable_clipping();
  for(const auto& line:page.lines)readingLine(line,area.width,
    [&](const std::string& s,float scale){return measure(s,scale);},
    [&](int x,const std::string& s){text(area.x+x,area.y+line.y,s,line.scale,presentation_.theme.text);});
  for(const auto& image:page.images) {
    const auto it=images_.find(image.image->key);
    if(it!=images_.end()&&it->second.texture)vita2d_draw_texture_scale(it->second.texture,area.x+(area.width-image.width)/2,area.y+image.y,float(image.width)/image.image->width,float(image.height)/image.image->height);
    else text(area.x,area.y+image.y+24,"Illustration indisponible",0.8f,presentation_.theme.secondary);
  }
  vita2d_disable_clipping();
  std::string label=std::to_string(position.page+1)+" / "+std::to_string(count);
  if(presentation_.pageCount==2)label=std::to_string(position.chapter+1)+" · "+label;
  else label="Chapitre "+std::to_string(position.chapter+1)+" / "+std::to_string(chapters)+"   ·   "+label;
  const auto paper=presentation_.paper[index];
  const int width=measure(label,0.8f);
  const int x=presentation_.pageCount==2?(index==0?area.x:area.x+area.width-width):(960-width)/2;
  text(x,presentation_.theme.physical?paper.y+paper.height-24:510,label,0.8f,presentation_.theme.secondary);
}
bool Renderer::captureOpening(const std::vector<Book>& books,std::size_t selected,Session& session,const Session::VisiblePage& second) {
  vita2d_wait_rendering_done(); if(!ensureSurfaces())return false;
  openingTitle_=books[selected].title;
  unsigned seed=2166136261u; for(unsigned char c:openingTitle_)seed=(seed^c)*16777619u;
  const unsigned palette[]={RGBA8(77,109,103,255u),RGBA8(139,72,59,255u),RGBA8(119,97,142,255u),RGBA8(154,121,61,255u),RGBA8(61,86,121,255u),RGBA8(114,91,76,255u)};
  openingCover_=palette[seed%6];
  vita2d_start_drawing_advanced(source_,0); vita2d_set_clear_color(RGBA8(24,21,21,255)); vita2d_clear_screen();
  library(books,selected); vita2d_end_drawing(); vita2d_wait_rendering_done();
  capture(destination_,books[selected],session,second); return true;
}
void Renderer::opening(float progress,std::size_t selected) {
  const float t=std::clamp(progress,0.0f,1.0f);
  vita2d_draw_texture(source_,0,0);
  const int slot=int(selected%20),row=slot/10;
  const float sx=48+(slot%10)*87,sh=148+int((selected*17)%35),sy=260+row*208-sh;
  const float lift=smoothStep(std::min(1.0f,t/0.4f)),unfold=smoothStep(std::max(0.0f,(t-0.25f)/0.75f));
  const auto left=presentation_.paper[0],right=presentation_.paper[presentation_.pageCount-1];
  const float width=right.x+right.width-left.x,height=left.height;
  const float x=sx+(left.x-sx)*lift,y=sy+(left.y-sy)*lift,w=74+(width-74)*lift,h=sh+(height-sh)*lift;
  vita2d_draw_rectangle(0,0,960,544,RGBA8(0,0,0,int(170*unfold)));
  vita2d_draw_rectangle(x+7,y+10,w,h,RGBA8(0,0,0,int(90*(1-t))));
  vita2d_set_clip_rectangle(int(x),int(y),int(x+w),int(y+h)); vita2d_enable_clipping();
  vita2d_draw_texture(destination_,0,0); vita2d_disable_clipping();
  const float cover=w*(1-unfold);
  vita2d_draw_rectangle(x,y,cover,h,openingCover_);
  vita2d_draw_rectangle(x+3,y,2,h,RGBA8(224,208,164,int(120*(1-unfold))));
  if(cover>55)text(int(x+12),int(y+45),fit(openingTitle_,int(cover)-24,0.85f),0.85f,RGBA8(245,229,197,255));
  if(cover>1)vita2d_draw_rectangle(x+cover,y,6,h,RGBA8(0,0,0,int(80*(1-unfold))));
  if(t>0.75f)vita2d_draw_texture_tint(destination_,0,0,RGBA8(255,255,255,int(smoothStep((t-0.75f)/0.25f)*255)));
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
  if(!presentation_.theme.physical||amount<0)return;
  const auto sheet=presentation_.paper[presentation_.pageCount-1];
  const int top=sheet.y-3,offset=-int((1-std::clamp(amount,0.0f,1.0f))*45);
  for(int page=0;page<presentation_.pageCount;++page)for(const auto& paint:ribbon_) {
    auto r=paint.bounds; r.x+=presentation_.paper[page].x-sheet.x;
    r.y+=page==0?offset:-45;
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
  reader(book,session.page(),session.position,session.pageCount(),session.chapterCount(),second,-1);
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
