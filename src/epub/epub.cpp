#include "epub/epub.hpp"
#include "app/config.hpp"
#include <archive.h>
#include <archive_entry.h>
#include <expat.h>
#include <algorithm>
#include <map>
#include <memory>
#include <stdexcept>
#include <functional>
#include <sstream>
namespace vr {
namespace {
std::string local(const char* s) { std::string n(s); auto p=n.find_last_of(':'); return p==std::string::npos?n:n.substr(p+1); }
std::string attr(const char** a,const char* key) { for(;*a;a+=2) if(local(a[0])==key) return a[1]; return {}; }
std::string resolve(const std::string& base,const std::string& href) {
  std::string decoded;
  for(std::size_t i=0;i<href.size() && href[i]!='#';++i) {
    if(href[i]=='%' && i+2<href.size()) { auto h=[](char c)->int { if(c>='0'&&c<='9')return c-'0'; if(c>='a'&&c<='f')return c-'a'+10; if(c>='A'&&c<='F')return c-'A'+10; return -1; }; int a=h(href[i+1]),b=h(href[i+2]); if(a<0||b<0)throw std::runtime_error("Invalid EPUB URI"); decoded+=char(a*16+b); i+=2; } else decoded+=href[i];
  }
  if(decoded.empty()||decoded[0]=='/'||decoded.find(':')!=std::string::npos||decoded.find('\\')!=std::string::npos||decoded.find('\0')!=std::string::npos) throw std::runtime_error("Unsupported EPUB URI");
  std::vector<std::string> parts; std::stringstream in(base+decoded); std::string p;
  while(std::getline(in,p,'/')) { if(p=="..") { if(parts.empty())throw std::runtime_error("EPUB path escapes archive"); parts.pop_back(); } else if(!p.empty()&&p!=".") parts.push_back(p); }
  std::string out; for(auto& v:parts) { if(!out.empty())out+='/'; out+=v; } return out;
}
std::string parent(const std::string& p) { auto i=p.find_last_of('/'); return i==std::string::npos?"":p.substr(0,i+1); }
struct Xml {
  std::function<void(std::string,const char**)> start;
  std::function<void(std::string)> end;
  std::function<void(std::string)> text;
  XML_Parser parser=nullptr;
  bool forbidden=false, failed=false;
};
void parse(const std::string& data,Xml& ctx) {
  auto parser=XML_ParserCreate(nullptr); if(!parser)throw std::runtime_error("XML allocation failed");
  ctx.parser=parser;
  XML_SetUserData(parser,&ctx);
  XML_SetElementHandler(parser,[](void* p,const char* n,const char** a) { auto& c=*static_cast<Xml*>(p); try { if(c.start)c.start(local(n),a); } catch(...) { c.failed=true; } },[](void* p,const char* n) { auto& c=*static_cast<Xml*>(p); try { if(c.end)c.end(local(n)); } catch(...) { c.failed=true; } });
  XML_SetCharacterDataHandler(parser,[](void* p,const char* s,int n) { auto& c=*static_cast<Xml*>(p); try { if(c.text)c.text(std::string(s,n)); } catch(...) { c.failed=true; } });
  XML_SetEntityDeclHandler(parser,[](void* p,const XML_Char*,int,const XML_Char*,int,const XML_Char*,const XML_Char*,const XML_Char*,const XML_Char*) { auto& c=*static_cast<Xml*>(p); c.forbidden=true; XML_StopParser(c.parser,XML_FALSE); });
  XML_SetParamEntityParsing(parser,XML_PARAM_ENTITY_PARSING_NEVER);
  int ok=XML_Parse(parser,data.data(),static_cast<int>(data.size()),XML_TRUE); XML_ParserFree(parser);
  if(!ok||ctx.failed||ctx.forbidden)throw std::runtime_error("Unsupported or malformed EPUB XML (entities/HTML not supported)");
}
}
std::string Epub::entry(const std::string& name) const {
  std::unique_ptr<archive,decltype(&archive_read_free)> ar(archive_read_new(),archive_read_free);
  if(!ar)throw std::runtime_error("Archive allocation failed");
  archive_read_support_format_zip(ar.get()); archive_read_support_filter_none(ar.get());
  if(archive_read_open_filename(ar.get(),path_.c_str(),16384)!=ARCHIVE_OK)throw std::runtime_error("Cannot open EPUB archive");
  archive_entry* e=nullptr; std::size_t count=0; int status;
  while((status=archive_read_next_header(ar.get(),&e))==ARCHIVE_OK) {
    if(++count>config::maxEntries)throw std::runtime_error("Too many ZIP entries");
    const char* p=archive_entry_pathname(e);
    if(p && name==p) {
      if(archive_entry_is_encrypted(e)>0)throw std::runtime_error("Encrypted EPUB is unsupported");
      std::string out; char buf[8192]; la_ssize_t n;
      while((n=archive_read_data(ar.get(),buf,sizeof buf))>0) { if(out.size()+n>config::maxEntry)throw std::runtime_error("EPUB entry exceeds 4 MiB limit"); out.append(buf,static_cast<std::size_t>(n)); }
      if(n<0)throw std::runtime_error("Corrupt EPUB entry"); return out;
    }
    archive_read_data_skip(ar.get());
  }
  throw std::runtime_error("Missing or corrupt EPUB entry: "+name);
}
void Epub::open(const std::string& path) {
  path_=path; title.clear(); author.clear(); cover.clear(); spine.clear();
  std::string package; Xml container;
  container.start=[&](std::string n,const char** a) { if(n=="rootfile"&&package.empty())package=resolve("",attr(a,"full-path")); };
  parse(entry("META-INF/container.xml"),container); if(package.empty())throw std::runtime_error("EPUB has no package");
  struct Item { std::string href,type; }; std::map<std::string,Item> items; std::vector<std::string> order; std::string field,coverId; Xml opf;
  opf.start=[&](std::string n,const char** a) {
    if(n=="title"||n=="creator")field=n;
    if(n=="item") { auto id=attr(a,"id"),href=attr(a,"href"); if(!href.empty())items[id]={resolve(parent(package),href),attr(a,"media-type")}; if(attr(a,"properties").find("cover-image")!=std::string::npos)coverId=id; }
    if(n=="meta"&&attr(a,"name")=="cover")coverId=attr(a,"content");
    if(n=="itemref"&&attr(a,"linear")!="no")order.push_back(attr(a,"idref"));
  };
  opf.end=[&](std::string n) { if(n==field)field.clear(); };
  opf.text=[&](std::string s) { if(field=="title")title+=s; else if(field=="creator")author+=s; };
  parse(entry(package),opf);
  for(auto& id:order) { auto it=items.find(id); if(it==items.end())throw std::runtime_error("Broken EPUB spine"); if(it->second.type!="application/xhtml+xml")throw std::runtime_error("Only XHTML spine content is supported"); spine.push_back(it->second.href); }
  if(spine.empty())throw std::runtime_error("EPUB has no readable spine");
  auto c=items.find(coverId); if(c!=items.end())cover=c->second.href;
}
Chapter Epub::chapter(std::size_t index) const {
  if(index>=spine.size())throw std::runtime_error("Invalid chapter");
  Chapter result; Block current; int body=0,skip=0,bold=0,italic=0; bool space=false; std::size_t imageBytes=0;
  auto flush=[&] { if(!current.runs.empty())result.blocks.push_back(std::move(current)); current=Block{}; space=false; };
  auto append=[&](const std::string& s) { if(current.runs.empty()||current.runs.back().bold!=(bold>0)||current.runs.back().italic!=(italic>0))current.runs.push_back({{},bold>0,italic>0}); current.runs.back().text+=s; };
  Xml x; x.start=[&](std::string n,const char** attrs) {
    if(n=="body")++body; if(!body)return;
    if(n=="script"||n=="style")++skip; if(skip)return;
    if(n=="p"||n=="div"||n=="li"||n=="blockquote"||(n.size()==2&&n[0]=='h'&&n[1]>='1'&&n[1]<='6')) { flush(); if(n[0]=='h')current.kind=Block::Kind::Heading; }
    if(n=="b"||n=="strong")++bold; if(n=="i"||n=="em")++italic;
    if(n=="br") { append("\n"); space=false; }
    if(n=="img"||n=="image"||n=="object") {
      flush(); Block image; image.kind=Block::Kind::Image;
      auto source=attr(attrs,n=="img"?"src":n=="object"?"data":"href");
      try {
        const auto name=resolve(parent(spine[index]),source); auto bytes=entry(name);
        if(imageBytes+bytes.size()<=8*1024*1024) { image.image=imageData(path_+":"+name,std::move(bytes)); if(image.image)imageBytes+=image.image->bytes.size(); }
      } catch(const std::exception&) {}
      if(!image.image)image.runs.push_back({"[Illustration non prise en charge]"});
      result.blocks.push_back(std::move(image));
    }
  };
  x.end=[&](std::string n) {
    if(n=="script"||n=="style") { --skip; return; } if(!body||skip)return;
    if(n=="b"||n=="strong")bold=std::max(0,bold-1); if(n=="i"||n=="em")italic=std::max(0,italic-1);
    if(n=="p"||n=="div"||n=="li"||n=="blockquote"||(n.size()==2&&n[0]=='h'))flush();
    if(n=="body") { flush(); --body; }
  };
  x.text=[&](std::string s) { if(!body||skip)return; for(unsigned char c:s) { if(c==' '||c=='\n'||c=='\r'||c=='\t')space=true; else {
    if(space&&!current.runs.empty()) {
      std::string tail; for(auto it=current.runs.rbegin();it!=current.runs.rend()&&tail.size()<24;++it)tail=it->text.substr(it->text.size()>24?it->text.size()-24:0)+tail;
      const bool apostrophe=!tail.empty()&&(tail.back()=='\''||(tail.size()>=3&&tail.compare(tail.size()-3,3,"’")==0));
      bool elision=false;
      if(apostrophe) {
        tail.resize(tail.size()-(tail.back()=='\''?1:3)); const auto last=tail.find_last_of(" \n\t"); if(last!=std::string::npos)tail=tail.substr(last+1);
        for(char& ch:tail)if(ch>='A'&&ch<='Z')ch+=32;
        for(const auto* prefix:{"l","d","j","t","m","s","n","c","qu","jusqu","lorsqu","puisqu"})if(tail==prefix)elision=true;
      }
      if(!elision)append(" ");
    }
    space=false; append(std::string(1,static_cast<char>(c)));
  } } };
  parse(entry(spine[index]),x); flush(); return result;
}
}
