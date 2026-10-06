#pragma once
#include <string>
#include <memory>
#include <cstdint>
namespace vr {
struct ImageData { std::string key,bytes; int width=0,height=0; bool png=false; };
inline std::shared_ptr<const ImageData> imageData(std::string key,std::string bytes) {
  auto image=std::make_shared<ImageData>(); image->key=std::move(key); image->bytes=std::move(bytes);
  const auto* p=reinterpret_cast<const unsigned char*>(image->bytes.data()); const auto n=image->bytes.size();
  auto be16=[&](std::size_t i){return int(p[i])*256+p[i+1];};
  if(n>=33&&image->bytes.compare(0,8,"\x89PNG\r\n\x1a\n",8)==0&&image->bytes.compare(12,4,"IHDR")==0) {
    auto be32=[&](int i){return (std::uint32_t(p[i])<<24)|(std::uint32_t(p[i+1])<<16)|(std::uint32_t(p[i+2])<<8)|p[i+3];};
    const auto w=be32(16),h=be32(20); if(w>2048||h>2048)return {};
    image->width=int(w); image->height=int(h); image->png=true;
  } else if(n>=4&&p[0]==255&&p[1]==216) {
    for(std::size_t i=2;i+4<n;) {
      if(p[i++]!=255)return {};
      while(i<n&&p[i]==255)++i;
      if(i>=n)return {};
      const auto marker=p[i++]; if(marker==217||marker==218)break;
      if(marker==1||(marker>=208&&marker<=215))continue;
      if(i+2>n)return {};
      const auto size=be16(i); if(size<2||i+size>n)return {};
      if((marker>=192&&marker<=195)||(marker>=197&&marker<=199)||(marker>=201&&marker<=203)||(marker>=205&&marker<=207)) {
        if(size<8)return {};
        image->height=be16(i+3); image->width=be16(i+5); break;
      }
      i+=size;
    }
  }
  if(image->width<=0||image->height<=0||image->width>2048||image->height>2048)return {};
  return image;
}
}
