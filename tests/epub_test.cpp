#include "epub/epub.hpp"
#include "reader/session.hpp"
#include <cassert>
#include <iostream>
int main(int argc,char** argv) {
  assert(argc==2);
  vr::Epub epub; epub.open(argv[1]);
  assert(epub.title=="Le carnet de lecture"); assert(epub.author=="VitaReader"); assert(epub.spine.size()==2);
  auto chapter=epub.chapter(0); assert(chapter.blocks.size()>=32);
  assert(chapter.blocks[0].kind==vr::Block::Kind::Heading);
  bool bold=false,italic=false,accents=false;
  for(auto& b:chapter.blocks)for(auto& r:b.runs) { bold|=r.bold; italic|=r.italic; accents|=r.text.find("œ")!=std::string::npos; }
  assert(bold&&italic&&accents);
  vr::Book book; book.path=argv[1]; book.position={999999,999999}; vr::Session reader;
  auto measure=[](const std::string& s,float){return static_cast<int>(s.size())*10;};
  reader.open(book,measure); assert(reader.position.chapter==1); assert(reader.position.page+1==reader.pageCount());
  assert(!reader.turn(1,measure));
  while(reader.turn(-1,measure)){}
  assert(reader.position.chapter==0&&reader.position.page==0);
  while(reader.turn(1,measure)){}
  assert(reader.position.chapter==1&&reader.position.page+1==reader.pageCount());
  auto saved=reader.position;
  vr::Book invalid; invalid.path="missing.epub";
  bool rejected=false; try { reader.open(invalid,measure); } catch(...) { rejected=true; }
  assert(rejected&&reader.position.chapter==saved.chapter&&reader.position.page==saved.page);
  std::cout<<"epub: metadata, spine, semantics, navigation, stale resume and failed-open preservation OK\n";
}
