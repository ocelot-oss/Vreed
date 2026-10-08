#include "reader/orientation.hpp"
#include "rendering/renderer.hpp"
#include "app/identity.hpp"
#include "rendering/typography.hpp"
#include "storage/file.hpp"
#include "storage/atomic_file.hpp"
#include <ctime>
#include <sstream>
#include <iomanip>
#include <stdexcept>
#include <algorithm>
#include <cmath>
#include <png.h>
#include <set>
#include <cstring>
#include <psp2/sysmodule.h>
extern "C" { extern float _vita2d_ortho_matrix[16]; }
namespace vr {
Renderer::Renderer() {
  if(sceSysmoduleLoadModule(SCE_SYSMODULE_PGF)<0)throw std::runtime_error("PGF module unavailable");
  if(vita2d_init()<0) { sceSysmoduleUnloadModule(SCE_SYSMODULE_PGF); throw std::runtime_error("Graphics initialization failed"); }
  std::memcpy(landscapeProjection_,_vita2d_ortho_matrix,sizeof(landscapeProjection_));
  font_=vita2d_load_default_pgf();
  if(!font_) { vita2d_fini(); sceSysmoduleUnloadModule(SCE_SYSMODULE_PGF); throw std::runtime_error("System font unavailable"); }
  logo_=vita2d_load_PNG_file("app0:/assets/branding/library-logo.png");
  if(logo_) { logo_->gxm_rtgt=nullptr; logo_->depth_UID=0; vita2d_texture_set_filters(logo_,SCE_GXM_TEXTURE_FILTER_LINEAR,SCE_GXM_TEXTURE_FILTER_LINEAR); }
  mascots_=vita2d_load_PNG_file("app0:/assets/branding/mascot-poses.png");
  if(mascots_) { mascots_->gxm_rtgt=nullptr; mascots_->depth_UID=0; vita2d_texture_set_filters(mascots_,SCE_GXM_TEXTURE_FILTER_LINEAR,SCE_GXM_TEXTURE_FILTER_LINEAR); }
}
Renderer::~Renderer() {
  vita2d_wait_rendering_done();
  clearImages();
  if(logo_)vita2d_free_texture(logo_);
  if(mascots_)vita2d_free_texture(mascots_);
  if(readingFont_)vita2d_free_font(readingFont_);
  if(source_)vita2d_free_texture(source_);
  if(destination_)vita2d_free_texture(destination_);
  vita2d_free_pgf(font_); vita2d_fini(); sceSysmoduleUnloadModule(SCE_SYSMODULE_PGF);
}
void Renderer::prepareLayout() { vita2d_wait_rendering_done(); } // Measurements may populate the shared glyph atlas.
void Renderer::selectFont(int family) {
  if(family==fontFamily_)return;
  if(family<0||family>3)throw std::runtime_error("Police inconnue");
  vita2d_wait_rendering_done();
  const char* paths[]={"","app0:/assets/fonts/DejaVuSerif.ttf","app0:/assets/fonts/DejaVuSans.ttf","app0:/assets/fonts/DejaVuSansMono.ttf"};
  auto* next=family?vita2d_load_font_file(paths[family]):nullptr;
  if(family&&!next)throw std::runtime_error("Impossible de charger la police");
  if(readingFont_)vita2d_free_font(readingFont_);
  readingFont_=next; fontFamily_=family;
}
int Renderer::rawWidth(const std::string& s,float scale) const {
  return readingFont_?vita2d_font_text_width(readingFont_,unsigned(std::max(1,int(24*scale))),s.c_str()):vita2d_pgf_text_width(font_,scale,s.c_str());
}
int Renderer::fontHeight(float scale) const {
  return readingFont_?vita2d_font_text_height(readingFont_,unsigned(std::max(1,int(24*scale))),"Agéœ"):vita2d_pgf_text_height(font_,scale,"Agéœ");
}
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
      if(data.rgba&&data.bytes.size()==bytes) {
        loaded.texture=vita2d_create_empty_texture(data.width,data.height);
        if(loaded.texture) {
          auto* dest=static_cast<unsigned char*>(vita2d_texture_get_datap(loaded.texture));
          const auto stride=vita2d_texture_get_stride(loaded.texture);
          for(int y=0;y<data.height;++y)std::memcpy(dest+y*stride,data.bytes.data()+std::size_t(y)*data.width*4,data.width*4);
        }
      } else if(data.png) {
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
  return presentation(settings,[&](float scale){return fontHeight(scale);});
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
  return textAdvance(s,scale,[&](const std::string& piece,float size){return rawWidth(piece,size);},[](int,const std::string&){});
}
void Renderer::text(int x,int y,const std::string& s,float scale,unsigned color) {
  textAdvance(s,scale,[&](const std::string& piece,float size){return rawWidth(piece,size);},[&](int offset,const std::string& piece){
    if(readingFont_)vita2d_font_draw_text(readingFont_,x+offset,y,color,unsigned(std::max(1,int(24*scale))),piece.c_str());
    else vita2d_pgf_draw_text(font_,x+offset,y,color,scale,piece.c_str());
  });
}
std::string Renderer::fit(const std::string& s,int width,float scale) const {
  if(measure(s,scale)<=width)return s; std::string out=s;
  while(!out.empty()&&measure(out+"...",scale)>width) { auto i=out.size()-1; while(i>0&&(static_cast<unsigned char>(out[i])&0xc0)==0x80)--i; out.resize(i); } return out+"...";
}
void Renderer::projection(Orientation orientation) {
  std::memcpy(_vita2d_ortho_matrix,landscapeProjection_,sizeof(landscapeProjection_));
  if(orientation==Orientation::Horizontal)return;
  const float sign=orientation==Orientation::VerticalRight?1.0f:-1.0f;
  _vita2d_ortho_matrix[0]=0; _vita2d_ortho_matrix[5]=0;
  _vita2d_ortho_matrix[4]=-sign*2/960; _vita2d_ortho_matrix[1]=-sign*2/544;
  _vita2d_ortho_matrix[12]=sign; _vita2d_ortho_matrix[13]=sign;
}
void Renderer::drawScene(vita2d_texture* texture,float offset,unsigned color) {
  if(presentation_.orientation==Orientation::VerticalRight)
    vita2d_draw_texture_tint_scale_rotate_hotspot(texture,offset,960,1,1,-1.5707963268f,0,0,color);
  else if(presentation_.orientation==Orientation::VerticalLeft)
    vita2d_draw_texture_tint_scale_rotate_hotspot(texture,544+offset,0,1,1,1.5707963268f,0,0,color);
  else vita2d_draw_texture_tint(texture,offset,0,color);
}
void Renderer::clip(const Rect& area) {
  // vita2d's stencil reset is fixed at 960x544. Clear the entire logical
  // viewport first so portrait clipping cannot retain stale lower-page pixels.
  vita2d_disable_clipping();
  sceGxmSetFrontStencilFunc(vita2d_get_context(),SCE_GXM_STENCIL_FUNC_NEVER,
    SCE_GXM_STENCIL_OP_ZERO,SCE_GXM_STENCIL_OP_ZERO,SCE_GXM_STENCIL_OP_ZERO,0xFF,0xFF);
  vita2d_draw_rectangle(0,0,presentation_.width,presentation_.height,0);
  vita2d_set_clip_rectangle(area.x,area.y,area.x+area.width,area.y+area.height);
  vita2d_enable_clipping();
}
void Renderer::begin(bool reading) {
  reading_=reading; projection(reading?presentation_.orientation:Orientation::Horizontal);
  vita2d_set_clear_color(reading?presentation_.theme.background:RGBA8(15,20,25,255));
  vita2d_start_drawing(); vita2d_clear_screen();
}
void Renderer::finish() { vita2d_end_drawing(); vita2d_swap_buffers(); }
void Renderer::library(const std::vector<Book>& books,std::size_t selected,double nowMs) {
  libraryTime_=nowMs;
  vita2d_draw_rectangle(0,0,960,544,RGBA8(248,243,230,255));
  // Sample only the mascot area of the existing logo, leaving the wordmark out.
  if(mascots_) {
    const float cell=vita2d_texture_get_width(mascots_)/3.0f,height=vita2d_texture_get_height(mascots_);
    const float t=float(nowMs/1000);
    for(int pose=0;pose<3;++pose) {
      const float x=90+pose*255+std::sin(t*0.16f+pose*2)*44;
      const float y=95+std::sin(t*0.24f+pose)*19+pose*16;
      vita2d_draw_texture_tint_part_scale(mascots_,x,y,pose*cell,0,cell,height,290/cell,290/height,RGBA8(255,255,255,40));
    }
  } else if(logo_) {
    const float w=vita2d_texture_get_width(logo_),h=vita2d_texture_get_height(logo_);
    vita2d_draw_texture_tint_part_scale(logo_,350,92,w*0.30f,h*0.19f,w*0.47f,h*0.43f,
      480.0f/(w*0.47f),410.0f/(h*0.43f),RGBA8(255,255,255,55));
  }
  if(logo_)vita2d_draw_texture_scale(logo_,30,-18,120.0f/vita2d_texture_get_width(logo_),120.0f/vita2d_texture_get_height(logo_));
  text(164,68,"Bibliothèque · "+std::to_string(books.size())+" livres",0.85f,RGBA8(105,91,74,255));
  for(int row=0;row<2;++row) {
    const int y=260+row*208;
    vita2d_draw_rectangle(28,y+14,904,14,RGBA8(0,0,0,90));
    vita2d_draw_rectangle(28,y,904,18,RGBA8(102,67,43,255));
    vita2d_draw_rectangle(28,y,904,2,RGBA8(157,111,71,255));
    for(int line=0;line<5;++line)vita2d_draw_rectangle(32,y+4+line*2,896,1,RGBA8(43,28,18,25));
  }
  if(books.empty()) { text(42,210,"Ajoutez vos fichiers EPUB ou PDF dans :",1.2f,RGBA8(54,44,36,255)); text(42,252,"ux0:/data/VitaReader/books",1.1f,RGBA8(54,44,36,255)); }
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
    text(x+10,bottom-9,books[i].position.resumePinned?"Signet":books[i].format==Book::Format::Pdf?"PDF":"EPUB",0.65f,RGBA8(242,224,186,255));
  }
  text(42,518,"X Ouvrir · Toucher pour sélectionner · Triangle Actualiser",0.8f,RGBA8(105,91,74,255));
}
void Renderer::drawPage(const Page& page,int index,Position position,std::size_t count,std::size_t chapters) {
  const auto area=presentation_.content[index];
  // Each page clips to its own rectangle; glyph bearings cannot enter the gutter.
  clip(area);
  for(const auto& line:page.lines)readingLine(line,area.width,
    [&](const std::string& s,float scale){return measure(s,scale);},
    [&](int x,const std::string& s){text(area.x+x,area.y+line.y,s,line.scale,presentation_.theme.text);});
  for(const auto& image:page.images) {
    const auto it=images_.find(image.image->key);
    if(it!=images_.end()&&it->second.texture) {
      if(image.image->rgba) {
        const float scale=pdfFitWidth_?float(area.width)/image.image->width:
          std::min(float(area.width)/image.image->width,float(area.height)/image.image->height);
        const float h=image.image->height*scale,w=image.image->width*scale;
        const float maxPan=std::max(0.0f,h-area.height);
        pdfMaxPan_=std::max(pdfMaxPan_,maxPan);
        const float y=pdfFitWidth_?-std::min(pdfPan_,maxPan):(area.height-h)/2;
        vita2d_draw_texture_scale(it->second.texture,area.x+(area.width-w)/2,area.y+y,scale,scale);
      } else vita2d_draw_texture_scale(it->second.texture,area.x+(area.width-image.width)/2,area.y+image.y,float(image.width)/image.image->width,float(image.height)/image.image->height);
    }
    else text(area.x,area.y+image.y+24,"Illustration indisponible",0.8f,presentation_.theme.secondary);
  }
  vita2d_disable_clipping();
  std::string label=std::to_string(position.page+1)+" / "+std::to_string(count);
  if(!page.images.empty()&&page.images.front().image->rgba)label=std::to_string(position.chapter+1)+" / "+std::to_string(chapters);
  else if(presentation_.pageCount==2)label=std::to_string(position.chapter+1)+" · "+label;
  else label="Chapitre "+std::to_string(position.chapter+1)+" / "+std::to_string(chapters)+"   ·   "+label;
  const auto paper=presentation_.paper[index];
  const int width=measure(label,0.8f);
  const int x=presentation_.pageCount==2?(index==0?area.x:area.x+area.width-width):(presentation_.width-width)/2;
  text(x,presentation_.theme.physical?paper.y+paper.height-24:presentation_.height-34,label,0.8f,presentation_.theme.secondary);
}
bool Renderer::captureOpening(const std::vector<Book>& books,std::size_t selected,Session& session,const Session::VisiblePage& second) {
  vita2d_wait_rendering_done(); if(!ensureSurfaces())return false;
  openingTitle_=books[selected].title;
  unsigned seed=2166136261u; for(unsigned char c:openingTitle_)seed=(seed^c)*16777619u;
  const unsigned palette[]={RGBA8(77,109,103,255u),RGBA8(139,72,59,255u),RGBA8(119,97,142,255u),RGBA8(154,121,61,255u),RGBA8(61,86,121,255u),RGBA8(114,91,76,255u)};
  openingCover_=palette[seed%6];
  projection(Orientation::Horizontal);
  vita2d_start_drawing_advanced(source_,0); vita2d_set_clear_color(RGBA8(24,21,21,255)); vita2d_clear_screen();
  library(books,selected,libraryTime_); vita2d_end_drawing(); vita2d_wait_rendering_done();
  capture(destination_,books[selected],session,second); return true;
}
void Renderer::opening(float progress,std::size_t selected) {
  const float t=std::clamp(progress,0.0f,1.0f);
  drawScene(source_);
  const int slot=int(selected%20),row=slot/10;
  float sx=48+(slot%10)*87,sh=148+int((selected*17)%35),sy=260+row*208-sh,sw=74;
  if(presentation_.orientation==Orientation::VerticalRight) { const float oldX=sx; sx=sy; sy=960-oldX-sw; std::swap(sw,sh); }
  else if(presentation_.orientation==Orientation::VerticalLeft) { const float oldX=sx; sx=544-sy-sh; sy=oldX; std::swap(sw,sh); }
  const float lift=smoothStep(std::min(1.0f,t/0.4f)),unfold=smoothStep(std::max(0.0f,(t-0.25f)/0.75f));
  const auto left=presentation_.paper[0],right=presentation_.paper[presentation_.pageCount-1];
  const float width=right.x+right.width-left.x,height=left.height;
  const float x=sx+(left.x-sx)*lift,y=sy+(left.y-sy)*lift,w=sw+(width-sw)*lift,h=sh+(height-sh)*lift;
  vita2d_draw_rectangle(0,0,presentation_.width,presentation_.height,RGBA8(0,0,0,int(170*unfold)));
  vita2d_draw_rectangle(x+7,y+10,w,h,RGBA8(0,0,0,int(90*(1-t))));
  clip({int(x),int(y),int(w),int(h)});
  drawScene(destination_); vita2d_disable_clipping();
  const float cover=w*(1-unfold);
  vita2d_draw_rectangle(x,y,cover,h,openingCover_);
  vita2d_draw_rectangle(x+3,y,2,h,RGBA8(224,208,164,int(120*(1-unfold))));
  if(cover>55)text(int(x+12),int(y+45),fit(openingTitle_,int(cover)-24,0.85f),0.85f,RGBA8(245,229,197,255));
  if(cover>1)vita2d_draw_rectangle(x+cover,y,6,h,RGBA8(0,0,0,int(80*(1-unfold))));
  if(t>0.75f)drawScene(destination_,0,RGBA8(255,255,255,int(255*smoothStep((t-0.75f)/0.25f))));
}
void Renderer::reader(const Book& book,const Page& page,Position p,std::size_t count,std::size_t chapters,const Session::VisiblePage& second,float ribbon) {
  pdfMaxPan_=0;
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
  if(book.format==Book::Format::Pdf)text(24,presentation_.height-6,"X : page / largeur · Haut / bas : déplacer",0.65f,presentation_.theme.secondary);
}
void Renderer::panPdf(int direction) { pdfPan_=std::clamp(pdfPan_+direction*64.0f,0.0f,pdfMaxPan_); }
void Renderer::drawRibbon(float amount) {
  if(!presentation_.theme.physical||amount<0)return;
  const auto sheet=presentation_.paper[presentation_.pageCount-1];
  const int top=sheet.y-3,offset=-int((1-std::clamp(amount,0.0f,1.0f))*45);
  for(const auto& paint:ribbon_) {
    auto r=paint.bounds;
    r.y+=offset;
    if(r.y+r.height<=top)continue;
    if(r.y<top) { r.height-=top-r.y; r.y=top; }
    vita2d_draw_rectangle(r.x,r.y,r.width,r.height,paint.color);
  }
}
bool Renderer::saveScreenshot(const Book& book,Session& session,const Session::VisiblePage& second,const std::string& directory) {
  vita2d_wait_rendering_done();
  if(!ensureSurfaces()) { storageFailure="capture : mémoire graphique indisponible"; return false; }
  vita2d_set_clear_color(presentation_.theme.background);
  projection(presentation_.orientation);
  vita2d_start_drawing_advanced(source_,0); vita2d_clear_screen();
  reader(book,session.page(),session.position,session.pageCount(),session.chapterCount(),second,1);
  vita2d_end_drawing(); vita2d_wait_rendering_done();
  png_image image{}; image.version=PNG_IMAGE_VERSION; image.width=presentation_.width; image.height=presentation_.height; image.format=PNG_FORMAT_RGBA;
  const void* pixels=vita2d_texture_get_datap(source_);
  int stride=vita2d_texture_get_stride(source_);
  std::vector<unsigned char> upright;
  if(presentation_.orientation!=Orientation::Horizontal) {
    upright.resize(544*960*4);
    const auto* source=static_cast<const unsigned char*>(pixels);
    for(int y=0;y<960;++y)for(int x=0;x<544;++x) {
      const int sx=presentation_.orientation==Orientation::VerticalRight?959-y:y;
      const int sy=presentation_.orientation==Orientation::VerticalRight?x:543-x;
      std::memcpy(upright.data()+(y*544+x)*4,source+sy*stride+sx*4,4);
    }
    pixels=upright.data(); stride=544*4;
  }
  png_alloc_size_t size=0;
  if(!png_image_write_to_memory(&image,nullptr,&size,0,pixels,stride,nullptr)) {
    storageFailure="PNG : "+std::string(image.message); png_image_free(&image); return false;
  }
  std::string encoded(size,'\0');
  if(!png_image_write_to_memory(&image,encoded.data(),&size,0,pixels,stride,nullptr)) {
    storageFailure="PNG : "+std::string(image.message); png_image_free(&image); return false;
  }
  png_image_free(&image); encoded.resize(size);
  std::uint64_t hash=14695981039346656037ULL;
  for(unsigned char c:book.path) { hash^=c; hash*=1099511628211ULL; }
  std::ostringstream name; name<<directory<<'/'<<std::hex<<hash<<std::dec<<"-ch"<<session.position.chapter+1
    <<"-p"<<session.position.page+1<<'-'<<std::time(nullptr);
  std::string path;
  for(unsigned counter=0;;++counter) {
    path=name.str()+"-"+std::to_string(counter)+".png";
    SceIoStat stat{}; if(sceIoGetstat(path.c_str(),&stat)<0)break;
    if(counter>=9999) { storageFailure="capture : trop de fichiers pour cette page"; return false; }
  }
  const auto temporary=path+".tmp";
  if(!writeFile(temporary,encoded)) { sceIoRemove(temporary.c_str()); return false; }
  if(!replaceFile(temporary,path)) { sceIoRemove(temporary.c_str()); return false; }
  return true;
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
  projection(presentation_.orientation);
  vita2d_start_drawing_advanced(target,0); vita2d_clear_screen();
  reader(book,session.page(),session.position,session.pageCount(),session.chapterCount(),second,-1);
  vita2d_end_drawing(); vita2d_wait_rendering_done();
}
bool Renderer::captureTurn(const Book& book,Session& current,const Session::VisiblePage& currentSecond,Session& next,const Session::VisiblePage& nextSecond) {
  vita2d_wait_rendering_done();
  if(!ensureSurfaces())return false;
  capture(source_,book,current,currentSecond);
  const float pan=pdfPan_,maxPan=pdfMaxPan_; pdfPan_=0;
  capture(destination_,book,next,nextSecond);
  pdfPan_=pan; pdfMaxPan_=maxPan; return true;
}
void Renderer::transition(const PageTransition& motion) {
  if(!source_||!destination_)return;
  const float progress=motion.progress(); const int direction=motion.direction();
  if(motion.kind()==TransitionKind::Slide) {
    drawScene(source_,-direction*progress*presentation_.width);
    drawScene(destination_,direction*(1-progress)*presentation_.width); return;
  }
  if(motion.kind()!=TransitionKind::Curl) { drawScene(source_); return; }
  const float p=motion.state()==TurnState::Dragging?std::max(0.012f,progress):progress;
  drawScene(destination_);
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
  const unsigned bg=settings.mode==ReadingMode::Oled?RGBA8(0,0,0,255):RGBA8(20,26,31,250),fg=RGBA8(228,230,228,255),dim=RGBA8(137,151,159,255);
  const int w=std::min(510,presentation_.width-32),x=(presentation_.width-w)/2,y=(presentation_.height-512)/2;
  vita2d_draw_rectangle(x,y,w,512,bg); text(x+24,y+36,"Lecture",1.2f,fg);
  const char* labels[]={"Mode","Disposition","Taille de police","Interligne","Marges","Animation","Son de page","Volume","Police","Capture sur signet","Orientation"};
  for(int row=0;row<settingsRowCount;++row) {
    const int line=y+53+row*34;
    if(row==selected)vita2d_draw_rectangle(x+12,line-3,w-24,30,RGBA8(36,48,56,255));
    text(x+24,line+20,labels[row],0.85f,fg);
    const auto value=settingValue(settings,row);
    text(x+w-25-measure(value,0.85f),line+20,value,0.85f,dim);
  }
  text(x+24,y+454,presentation_.spreadLimited?"Une page pour rester lisible":"Gauche / droite : modifier",0.75f,dim);
  text(x+24,y+480,"O ou Triangle : fermer",0.75f,dim);
}
void Renderer::pageChooser(std::size_t page,std::size_t count) {
  const auto fg=presentation_.theme.text; const auto track=pageChooserTrack(presentation_);
  const int x=track.x-60,y=track.y-125,w=track.width+120;
  vita2d_draw_rectangle(x,y,w,250,presentation_.theme.paper);
  text(x+30,y+38,"Aller à une page du livre",1.05f,fg);
  text(x+30,y+82,std::to_string(page+1)+" / "+std::to_string(count),1.4f,fg);
  vita2d_draw_rectangle(track.x,track.y,track.width,4,presentation_.theme.secondary);
  const float thumb=track.x+(count>1?float(track.width)*page/(count-1):0);
  vita2d_draw_fill_circle(thumb,track.y+2,9,presentation_.theme.accent);
  text(x+30,y+178,"Gauche / droite : 1 · Haut / bas : 10",0.7f,fg);
  text(x+30,y+208,"Toucher la barre · X Valider · O Annuler",0.7f,fg);
}
void Renderer::message(const std::string& s) {
  const bool oled=reading_&&presentation_.theme.background==RGBA8(0,0,0,255);
  vita2d_draw_rectangle(24,(reading_?presentation_.height:544)-86,(reading_?presentation_.width:960)-48,38,oled?RGBA8(0,0,0,255):RGBA8(108,39,39,255));
  text(40,(reading_?presentation_.height:544)-61,fit(s,(reading_?presentation_.width:960)-90,0.85f),0.85f,oled?RGBA8(154,110,106,255):RGBA8(232,232,232,255));
}
}
