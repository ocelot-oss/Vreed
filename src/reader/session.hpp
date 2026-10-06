#pragma once
#include "reader/layout.hpp"
#include "storage/library.hpp"
#include <memory>
namespace vr {
class Session {
public:
  void open(Book& book,const Measure& measure,const Layout& layout=Layout{});
  bool turn(int direction,const Measure& measure,int count=1);
  void repaginate(const Layout& layout,const Measure& measure);
  const Page& page() const { return current_->pages.at(position.page); }
  struct VisiblePage { std::shared_ptr<const Page> page; Position position; std::size_t count=0; };
  VisiblePage secondPage(const Measure& measure);
  std::size_t pageCount() const { return current_->pages.size(); }
  std::size_t chapterCount() const { return epub_.spine.size(); }
  bool currentPageContains(std::size_t chapter,std::size_t offset) const;
  Position position;
private:
  struct ChapterPages { std::size_t index; Chapter content; std::vector<Page> pages; };
  Epub epub_; Layout layout_;
  std::shared_ptr<const ChapterPages> current_,preview_;
  std::shared_ptr<const ChapterPages> load(std::size_t index,const Measure& measure) const;
  void updateOffset();
};
}
