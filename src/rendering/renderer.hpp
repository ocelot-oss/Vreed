#pragma once
#include "reader/layout.hpp"
#include "reader/session.hpp"
#include "storage/library.hpp"
#include "rendering/page_style.hpp"
#include "rendering/curl.hpp"
#include <vita2d.h>
#include <map>
#include <initializer_list>
#include <algorithm>
namespace vr {
class Renderer {
public:
  Renderer(); ~Renderer();
  Renderer(const Renderer&)=delete; Renderer& operator=(const Renderer&)=delete;
  int measure(const std::string& text,float scale) const;
  void prepareLayout();
  void selectFont(int family);
  void togglePdfZoom() { pdfFitWidth_=!pdfFitWidth_; pdfPan_=0; }
  void panPdf(int direction);
  void resetPdfPan() { pdfPan_=0; }
  void prepareImages(std::initializer_list<const Page*> pages);
  Presentation readingPresentation(const ReadingSettings& settings,bool pdf=false) const;
  void defaultPdfZoom() { pdfFitWidth_=presentation_.orientation==Orientation::Horizontal; pdfPan_=0; }
  void panPdfPixels(float pixels) { pdfPan_=std::max(0.0f,std::min(pdfMaxPan_,pdfPan_+pixels)); }
  void configure(const Presentation& presentation);
  void begin(bool reading); void finish();
  void library(const std::vector<Book>& books,std::size_t selected,double nowMs=0);
  void libraryDirectory(const std::string& directory) { libraryDirectory_=directory; }
  void menuPanel(const std::string& title,const std::string& subtitle,const std::vector<std::string>& choices,int selected);
  bool captureOpening(const std::vector<Book>& books,std::size_t selected,Session& session,const Session::VisiblePage& second);
  void opening(float progress,std::size_t selected);
  void reader(const Book& book,const Page& page,Position position,std::size_t pageCount,
              std::size_t chapters,const Session::VisiblePage& second,float ribbon=0);
  bool captureTurn(const Book& book,Session& current,const Session::VisiblePage& currentSecond,
                   Session& next,const Session::VisiblePage& nextSecond);
  void transition(const PageTransition& transition);
  void drawRibbon(float amount);
  bool saveScreenshot(const Book& book,Session& session,const Session::VisiblePage& second,const std::string& directory);
  void boundaryFeedback(int direction,float amount);
  void settings(const ReadingSettings& settings,int selected);
  void pageChooser(std::size_t page,std::size_t count);
  void message(const std::string& message);
private:
  float landscapeProjection_[16]{};
  std::string libraryDirectory_="ux0:/data/VitaReader/books";
  void projection(Orientation orientation);
  void drawScene(vita2d_texture* texture,float offset=0,unsigned color=0xffffffff);
  void clip(const Rect& area);
  bool pdfFitWidth_=false;
  float pdfPan_=0,pdfMaxPan_=0;
  struct ImageTexture { vita2d_texture* texture=nullptr; std::size_t bytes=0; };
  std::map<std::string,ImageTexture> images_;
  void clearImages();
  vita2d_pgf* font_=nullptr;
  vita2d_font* readingFont_=nullptr;
  int fontFamily_=0;
  int rawWidth(const std::string& text,float scale) const;
  int fontHeight(float scale) const;
  vita2d_texture* logo_=nullptr;
  vita2d_texture* mascots_=nullptr;
  double libraryTime_=0;
  Presentation presentation_{};
  bool reading_=false;
  std::vector<PaintRect> paper_,ribbon_;
  vita2d_texture* source_=nullptr;
  vita2d_texture* destination_=nullptr;
  bool surfacesFailed_=false;
  std::string openingTitle_;
  unsigned openingCover_=0xff5f674c;
  bool ensureSurfaces();
  void capture(vita2d_texture* target,const Book& book,Session& session,const Session::VisiblePage& second);
  void paint(const std::vector<PaintRect>& rectangles);
  void drawPage(const Page& page,int index,Position position,std::size_t count,std::size_t chapters);
  void text(int x,int y,const std::string& s,float scale=1,unsigned color=0xffe8e8e8);
  std::string fit(const std::string& s,int width,float scale=1) const;
};
}
