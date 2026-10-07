// Synthetic EPUB provider tests session behavior independently of ZIP/XML libraries.
// This does not validate the real EPUB parser or Vita APIs.
#include "reader/session.hpp"
#include <cassert>
#include <iostream>
#include <stdexcept>
int main() {
  using namespace vr;
  auto measure=[](const std::string& text,float){return int(text.size())*10;};
  Layout layout; layout.width=60; layout.height=20; layout.lineHeight=20; layout.scale=1;
  Book book; book.path="good"; Session session; session.open(book,measure,layout);
  assert(session.pageCount()==3);
  assert(session.totalPageCount(measure)==6&&session.globalPageIndex()==0);
  assert(session.goToPage(5,measure)&&session.position.chapter==2&&session.position.page==0);
  assert(session.globalPageIndex()==5);
  assert(!session.goToPage(6,measure)&&session.position.chapter==2);
  assert(session.goToPage(0,measure));
  auto right=session.secondPage(measure); assert(right.position.chapter==0&&right.position.page==1);
  auto retained=right;
  assert(session.turn(1,measure,2)); assert(session.position.chapter==0&&session.position.page==2);
  right=session.secondPage(measure); assert(right.position.chapter==1&&right.position.page==0);
  assert(right.page->lines[0].text=="B0");
  assert(session.turn(1,measure,2)); assert(session.position.chapter==1&&session.position.page==1);
  right=session.secondPage(measure); assert(right.position.chapter==2&&right.position.page==0);
  assert(!session.turn(1,measure,2)); // Both final pages are already visible.
  assert(session.position.chapter==1&&session.position.page==1);
  assert(session.turn(-1,measure,2)); assert(session.position.chapter==0&&session.position.page==2);
  auto anchor=session.position.offset;
  Layout tall=layout; tall.height=100;
  session.repaginate(tall,measure); assert(session.position.page==0);
  assert(session.totalPageCount(measure)==3);
  assert(retained.page->lines[0].text=="A1"); // A visible-page handle survives cache replacement safely.
  assert(session.currentPageContains(0,anchor));
  session.repaginate(layout,measure);
  assert(session.totalPageCount(measure)==6);
  assert(session.position.page==2&&session.position.offset==anchor);
  auto rejecting=[](const std::string&,float)->int {throw std::runtime_error("Metrics failed");};
  bool failed=false; try { session.repaginate(tall,rejecting); } catch(...) { failed=true; }
  assert(failed&&session.pageCount()==3&&session.position.page==2);
  Book broken; broken.path="broken"; session.open(broken,measure,layout);
  assert(session.turn(1,measure)); // Page 1; a two-step turn would partially move before encountering bad chapter.
  failed=false; try { session.turn(1,measure,2); } catch(...) { failed=true; }
  assert(failed&&session.position.chapter==0&&session.position.page==1);
  std::cout<<"session: spreads across chapters, final spread, backward turns, reflow and failure rollback OK\n";
}
