#pragma once
#include "reader/layout.hpp"
#include "reader/session.hpp"
#include "storage/library.hpp"
#include "rendering/page_style.hpp"
#include "rendering/curl.hpp"
#include <vita2d.h>
#include <map>
#include <initializer_list>
namespace vr {
class Renderer {
public:
  Renderer(); ~Renderer();
  Renderer(const Renderer&)=delete; Renderer& operator=(const Renderer&)=delete;
  int measure(const std::string& text,float scale) const;
  void prepareLayout();
  void prepareImages(std::initializer_list<const Page*> pages);
  Presentation readingPresentation(const ReadingSettings& settings) const;
  void configure(const Presentation& presentation);
  void begin(bool reading); void finish();
  void library(const std::vector<Book>& books,std::size_t selected);
  void reader(const Book& book,const Page& page,Position position,std::size_t pageCount,
              std::size_t chapters,const Session::VisiblePage& second,float ribbon=0);
  bool captureTurn(const Book& book,Session& current,const Session::VisiblePage& currentSecond,
                   Session& next,const Session::VisiblePage& nextSecond);
  void transition(const PageTransition& transition);
  void drawRibbon(float amount);
  void boundaryFeedback(int direction,float amount);
  void settings(const ReadingSettings& settings,int selected);
  void message(const std::string& message);
private:
  struct ImageTexture { vita2d_texture* texture=nullptr; std::size_t bytes=0; };
  std::map<std::string,ImageTexture> images_;
  void clearImages();
  vita2d_pgf* font_=nullptr;
  vita2d_texture* logo_=nullptr;
  Presentation presentation_{};
  bool reading_=false;
  std::vector<PaintRect> paper_,ribbon_;
  vita2d_texture* source_=nullptr;
  vita2d_texture* destination_=nullptr;
  bool surfacesFailed_=false;
  bool ensureSurfaces();
  void capture(vita2d_texture* target,const Book& book,Session& session,const Session::VisiblePage& second);
  void paint(const std::vector<PaintRect>& rectangles);
  void drawPage(const Page& page,int index,Position position,std::size_t count,std::size_t chapters);
  void text(int x,int y,const std::string& s,float scale=1,unsigned color=0xffe8e8e8);
  std::string fit(const std::string& s,int width,float scale=1) const;
};
}
