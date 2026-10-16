#include "i18n/language.hpp"
#include "reader/session.hpp"
#include <algorithm>
#include <limits>
#include <stdexcept>
namespace vr {
std::shared_ptr<const Session::ChapterPages> Session::load(std::size_t index,const Measure& measure) const {
  if(current_&&current_->index==index)return current_;
  if(preview_&&preview_->index==index)return preview_;
  auto chapter=std::make_shared<ChapterPages>(); chapter->index=index;
  chapter->content=epub_.chapter(index); chapter->pages=paginate(chapter->content,layout_,measure);
  return chapter;
}
void Session::updateOffset() { position.offset=page().offset; position.hasOffset=true; }
bool Session::currentPageContains(std::size_t chapter,std::size_t offset) const {
  if(chapter!=position.chapter)return false;
  const auto end=position.page+1<current_->pages.size()?current_->pages[position.page+1].offset:std::numeric_limits<std::size_t>::max();
  return offset>=page().offset&&offset<end;
}
void Session::open(Book& book,const Measure& measure,const Layout& layout) {
  Epub next; next.open(book.path); auto p=book.position;
  p.chapter=std::min(p.chapter,next.spine.size()-1);
  auto chapter=std::make_shared<ChapterPages>(); chapter->index=p.chapter; chapter->content=next.chapter(p.chapter);
  chapter->pages=paginate(chapter->content,layout,measure);
  p.page=p.hasOffset?pageForOffset(chapter->pages,p.offset):std::min(p.page,chapter->pages.size()-1);
  epub_=std::move(next); current_=std::move(chapter); preview_.reset(); position=p; layout_=layout;
  pageCounts_.clear();
  if(!position.hasOffset)updateOffset();
  if(!epub_.title.empty())book.title=epub_.title;
  book.author=epub_.author; book.cover=epub_.cover;
}
bool Session::turn(int direction,const Measure& measure,int count) {
  if((direction!=1&&direction!=-1)||count<1||count>2)return false;
  auto next=current_; auto p=position; bool moved=false;
  for(int step=0;step<count;++step) {
    if(direction>0&&p.page+1<next->pages.size())++p.page;
    else if(direction<0&&p.page>0)--p.page;
    else {
      if(direction>0&&p.chapter+1>=epub_.spine.size())return false;
      if(direction<0&&p.chapter==0)break;
      p.chapter=direction>0?p.chapter+1:p.chapter-1;
      next=load(p.chapter,measure); p.page=direction>0?0:next->pages.size()-1;
    }
    moved=true;
  }
  if(!moved)return false;
  preview_=current_; current_=std::move(next); position=p; updateOffset(); return true;
}
void Session::repaginate(const Layout& layout,const Measure& measure) {
  auto next=std::make_shared<ChapterPages>(); next->index=position.chapter; next->content=current_->content;
  next->pages=paginate(next->content,layout,measure);
  const auto index=pageForOffset(next->pages,position.offset);
  current_=std::move(next); preview_.reset(); layout_=layout; position.page=index;
  pageCounts_.clear();
  // Preserve the same text anchor across repeated setting changes; navigation resets it.
}
Session::VisiblePage Session::secondPage(const Measure& measure) {
  if(position.page+1<current_->pages.size()) {
    auto p=position; ++p.page; p.offset=current_->pages[p.page].offset;
    return {std::shared_ptr<const Page>(current_,&current_->pages[p.page]),p,current_->pages.size()};
  }
  if(position.chapter+1>=epub_.spine.size())return {};
  preview_=load(position.chapter+1,measure);
  Position p{position.chapter+1,0,preview_->pages[0].offset,true};
  return {std::shared_ptr<const Page>(preview_,&preview_->pages[0]),p,preview_->pages.size()};
}
std::size_t Session::totalPageCount(const Measure& measure) {
  if(pageCounts_.empty()&&epub_.isPdf())pageCounts_.assign(epub_.spine.size(),1);
  if(pageCounts_.empty()) {
    std::vector<std::size_t> counts;
    for(std::size_t chapter=0;chapter<epub_.spine.size();++chapter)counts.push_back(load(chapter,measure)->pages.size());
    pageCounts_=std::move(counts);
  }
  std::size_t total=0;
  for(auto count:pageCounts_) {
    if(count>std::numeric_limits<std::size_t>::max()-total)throw std::runtime_error(tr("Too many book pages","Trop de pages dans le livre"));
    total+=count;
  }
  return total;
}
std::size_t Session::globalPageIndex() const {
  std::size_t index=position.page;
  for(std::size_t chapter=0;chapter<position.chapter&&chapter<pageCounts_.size();++chapter)index+=pageCounts_[chapter];
  return index;
}
bool Session::goToPage(std::size_t index,const Measure& measure) {
  if(index>=totalPageCount(measure))return false;
  std::size_t chapter=0;
  while(index>=pageCounts_[chapter]) { index-=pageCounts_[chapter]; ++chapter; }
  auto next=load(chapter,measure);
  if(index>=next->pages.size())return false;
  preview_=current_; current_=std::move(next); position.chapter=chapter; position.page=index; updateOffset(); return true;
}
}
