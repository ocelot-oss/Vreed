#include "pdf/pdf.hpp"
#include <mupdf/fitz.h>
#include <algorithm>
#include <cmath>
#include <stdexcept>
#include <deque>
namespace vr {
struct PdfDocument::State {
  fz_context* ctx=nullptr;
  fz_document* doc=nullptr;
  int pages=0;
  std::string path;
  std::deque<std::pair<std::size_t,std::shared_ptr<const ImageData>>> cache;
  std::size_t cachedBytes=0;
  ~State() { if(ctx) { fz_drop_document(ctx,doc); fz_drop_context(ctx); } }
};
PdfDocument::PdfDocument(const std::string& path):state_(new State) {
  auto& s=*state_; s.path=path; s.ctx=fz_new_context(nullptr,nullptr,64*1024*1024);
  if(!s.ctx)throw std::runtime_error("Memoire insuffisante pour le PDF");
  char titleBuffer[512]={},authorBuffer[512]={};
  fz_try(s.ctx) {
    fz_register_document_handlers(s.ctx);
    s.doc=fz_open_document(s.ctx,path.c_str());
    if(fz_needs_password(s.ctx,s.doc))fz_throw(s.ctx,FZ_ERROR_ARGUMENT,"PDF protege par mot de passe");
    s.pages=fz_count_pages(s.ctx,s.doc);
    if(s.pages<1||s.pages>10000)fz_throw(s.ctx,FZ_ERROR_ARGUMENT,"Nombre de pages PDF invalide");
    fz_lookup_metadata(s.ctx,s.doc,FZ_META_INFO_TITLE,titleBuffer,sizeof(titleBuffer));
    fz_lookup_metadata(s.ctx,s.doc,FZ_META_INFO_AUTHOR,authorBuffer,sizeof(authorBuffer));
  }
  fz_catch(s.ctx) { throw std::runtime_error(std::string("PDF : ")+fz_caught_message(s.ctx)); }
  title=titleBuffer; author=authorBuffer;
}
PdfDocument::~PdfDocument()=default;
std::size_t PdfDocument::count() const { return std::size_t(state_->pages); }
std::shared_ptr<const ImageData> PdfDocument::page(std::size_t index) const {
  auto& s=*state_; if(index>=count())throw std::runtime_error("Page PDF invalide");
  for(auto it=s.cache.begin();it!=s.cache.end();++it)if(it->first==index) {
    auto hit=*it; s.cache.erase(it); s.cache.push_back(hit); return hit.second;
  }
  fz_page* page=nullptr; fz_pixmap* pixmap=nullptr;
  fz_var(page); fz_var(pixmap);
  fz_try(s.ctx) {
    page=fz_load_page(s.ctx,s.doc,int(index));
    const auto bounds=fz_bound_page(s.ctx,page);
    const float width=bounds.x1-bounds.x0,height=bounds.y1-bounds.y0;
    if(!std::isfinite(width)||!std::isfinite(height)||width<=0||height<=0)
      fz_throw(s.ctx,FZ_ERROR_ARGUMENT,"Dimensions PDF invalides");
    const float scale=std::min(1024.0f/width,1400.0f/height);
    pixmap=fz_new_pixmap_from_page(s.ctx,page,fz_scale(scale,scale),fz_device_rgb(s.ctx),0);
  }
  fz_always(s.ctx) { fz_drop_page(s.ctx,page); }
  fz_catch(s.ctx) { fz_drop_pixmap(s.ctx,pixmap); throw std::runtime_error(std::string("PDF : ")+fz_caught_message(s.ctx)); }
  // Keep C++ allocations outside MuPDF's longjmp exception frames.
  auto drop=[&](fz_pixmap* p){fz_drop_pixmap(s.ctx,p);};
  std::unique_ptr<fz_pixmap,decltype(drop)> guard(pixmap,drop);
  auto image=std::make_shared<ImageData>(); image->key=s.path+"#pdf:"+std::to_string(index);
  image->width=fz_pixmap_width(s.ctx,pixmap); image->height=fz_pixmap_height(s.ctx,pixmap);
  const int components=fz_pixmap_components(s.ctx,pixmap),stride=fz_pixmap_stride(s.ctx,pixmap);
  if(image->width<=0||image->height<=0||image->width>2048||image->height>2048||components!=3)
    throw std::runtime_error("Format de page PDF invalide");
  image->rgba=true; image->bytes.resize(std::size_t(image->width)*image->height*4);
  const auto* samples=fz_pixmap_samples(s.ctx,pixmap);
  for(int y=0;y<image->height;++y)for(int x=0;x<image->width;++x) {
    const auto* rgb=samples+y*stride+x*3;
    auto* dst=&image->bytes[(std::size_t(y)*image->width+x)*4];
    dst[0]=char(rgb[0]); dst[1]=char(rgb[1]); dst[2]=char(rgb[2]); dst[3]=char(255);
  }
  constexpr std::size_t budget=16*1024*1024;
  while(!s.cache.empty()&&(s.cache.size()>=3||s.cachedBytes+image->bytes.size()>budget)) {
    s.cachedBytes-=s.cache.front().second->bytes.size(); s.cache.pop_front();
  }
  if(image->bytes.size()<=budget) { s.cache.emplace_back(index,image); s.cachedBytes+=image->bytes.size(); }
  return image;
}
}
