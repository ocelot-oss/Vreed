#pragma once
#include <string>
#include <algorithm>
#include "reader/layout.hpp"
namespace vr {
inline std::string latinPunctuation(const std::string& text) {
  std::string out;
  for(std::size_t i=0;i<text.size();) {
    if(text.compare(i,3,"’")==0||text.compare(i,3,"‘")==0||text.compare(i,3,"＇")==0) { out+='\''; i+=3; }
    else if(text.compare(i,2,"ʼ")==0) { out+='\''; i+=2; }
    else out+=text[i++];
  }
  return out;
}
// Use the same explicit advance in measurement and drawing. PGF punctuation
// fallback may have a much wider advance than a Latin apostrophe.
template<class Draw> int textAdvance(const std::string& source,float scale,const Measure& raw,Draw draw) {
  const auto text=latinPunctuation(source); int x=0; std::size_t begin=0;
  while(begin<text.size()) {
    const auto quote=text.find('\'',begin);
    const auto end=quote==std::string::npos?text.size():quote;
    if(end>begin) { const auto piece=text.substr(begin,end-begin); draw(x,piece); x+=raw(piece,scale); }
    if(quote==std::string::npos)break;
    draw(x,"'"); x+=std::max(1,std::min(raw("'",scale),int(raw("n",scale)*0.45f))); begin=quote+1;
  }
  return x;
}
}
