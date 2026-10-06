#pragma once
#include <string>
#include <algorithm>
#include "reader/layout.hpp"
namespace vr {
template<class Draw> void readingLine(const Line& line,int width,const Measure& measure,Draw draw) {
  if(!line.justify) { draw(0,line.text); return; }
  std::vector<std::string> words;
  for(std::size_t start=0;start<line.text.size();) {
    const auto end=line.text.find(' ',start);
    words.push_back(line.text.substr(start,end==std::string::npos?end:end-start));
    if(end==std::string::npos)break;
    start=end+1;
  }
  if(words.size()<2) { draw(0,line.text); return; }
  int wordWidth=0; for(const auto& word:words)wordWidth+=measure(word,line.scale);
  if(wordWidth>=width) { draw(0,line.text); return; }
  const int gaps=int(words.size()-1),spacing=width-wordWidth;
  int x=0;
  for(std::size_t i=0;i<words.size();++i) {
    draw(x+spacing*int(i)/gaps,words[i]); x+=measure(words[i],line.scale);
  }
}
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
