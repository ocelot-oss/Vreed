#pragma once
#include "app/config.hpp"
#include "storage/atomic_file.hpp"
namespace vr {
inline std::string normalizeLibraryDirectory(std::string path) {
  if(path.size()>1024)return {};
  const auto colon=path.find(':');
  if(colon==std::string::npos||colon<2||colon>8||path.substr(colon,2)!=":/")return {};
  for(std::size_t i=0;i<colon;++i)if(!((path[i]>='a'&&path[i]<='z')||(path[i]>='0'&&path[i]<='9')))return {};
  for(unsigned char c:path)if(c<32||c==127||c=='\\')return {};
  if(path.find(':',colon+1)!=std::string::npos)return {};
  while(path.size()>colon+2&&path.back()=='/')path.pop_back();
  for(std::size_t i=colon+2;i<path.size();) {
    auto end=path.find('/',i); if(end==std::string::npos)end=path.size();
    const auto part=path.substr(i,end-i); if(part.empty()||part=="."||part=="..")return {};
    i=end+1;
  }
  return path;
}
inline std::string joinDirectory(const std::string& dir,const std::string& name) { return dir+(dir.back()=='/'?"":"/")+name; }
inline std::string parentDirectory(const std::string& dir) {
  const auto root=dir.find(':')+2;
  if(dir.size()<=root)return dir;
  const auto slash=dir.find_last_of('/'); return dir.substr(0,slash<root?root:slash);
}
inline std::string loadLibraryDirectory(const std::string& file) {
  std::string data; if(!readFile(file,data)&&!readFile(file+".bak",data))return config::books;
  if(data.rfind("VRL1\n",0)!=0)return config::books;
  auto path=data.substr(5); if(!path.empty()&&path.back()=='\n')path.pop_back();
  path=normalizeLibraryDirectory(path); return path.empty()?config::books:path;
}
inline bool saveLibraryDirectory(const std::string& file,const std::string& directory) {
  const auto path=normalizeLibraryDirectory(directory); if(path.empty()) { storageFailure="Dossier invalide"; return false; }
  return writeFile(file+".tmp","VRL1\n"+path+"\n")&&replaceFile(file+".tmp",file);
}
}
