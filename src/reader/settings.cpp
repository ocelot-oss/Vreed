#include "reader/settings.hpp"
#include <algorithm>
#include <stdexcept>
namespace vr {
namespace {
constexpr std::uint32_t rgb(int r,int g,int b) { return 0xff000000u|(b<<16)|(g<<8)|r; }
int cycle(int value,int direction,int count) { return (value+direction+count)%count; }
}
const char* modeName(ReadingMode m) {
  switch(m) { case ReadingMode::Book:return "BOOK"; case ReadingMode::Clean:return "CLEAN"; case ReadingMode::Oled:return "OLED NIGHT"; }
  return "BOOK";
}
Presentation presentation(const ReadingSettings& s,const HeightMeasure& measure) {
  if(s.fontSize<0||s.fontSize>2||s.lineSpacing<0||s.lineSpacing>2||s.margins<0||s.margins>2)
    throw std::runtime_error("Invalid reading settings");
  Presentation p;
  if(s.mode==ReadingMode::Book)p.theme={rgb(20,23,25),rgb(242,230,205),rgb(48,41,32),rgb(128,111,87),rgb(138,47,48),true,true};
  else if(s.mode==ReadingMode::Clean)p.theme={rgb(250,250,247),rgb(250,250,247),rgb(34,39,43),rgb(121,126,130),rgb(62,102,121),false,false};
  else p.theme={rgb(0,0,0),rgb(0,0,0),rgb(187,188,181),rgb(88,91,95),rgb(124,149,158),false,false};
  p.spreadLimited=s.mode==ReadingMode::Book&&s.bookLayout==BookLayout::Spread&&s.fontSize==2;
  p.pageCount=s.mode==ReadingMode::Book&&s.bookLayout==BookLayout::Spread&&!p.spreadLimited?2:1;
  const float scale[]={1.10f,1.25f,1.50f}; p.layout.scale=scale[s.fontSize];
  const int gap[]={3,7,12};
  p.layout.lineHeight=std::max(24,measure(p.layout.scale*1.15f)+gap[s.lineSpacing]);
  if(p.theme.physical) {
    if(p.pageCount==2) { p.paper[0]={30,22,450,500}; p.paper[1]={480,22,450,500}; }
    else p.paper[0]={72,18,816,506};
    const int margin=p.pageCount==2?24+s.margins*8:40+s.margins*12;
    for(int i=0;i<p.pageCount;++i) {
      auto paper=p.paper[i];
      p.content[i]={paper.x+margin,paper.y+57,paper.width-2*margin,paper.height-108};
    }
  } else {
    p.paper[0]={0,0,960,544};
    const int margin=(s.mode==ReadingMode::Oled?80:64)+s.margins*16;
    p.content[0]={margin,s.mode==ReadingMode::Oled?72:64,960-2*margin,s.mode==ReadingMode::Oled?396:408};
  }
  p.layout.width=p.content[0].width; p.layout.height=p.content[0].height;
  if(p.layout.lineHeight>p.layout.height)throw std::runtime_error("Font is too tall for the reading viewport");
  return p;
}
bool adjustSetting(ReadingSettings& s,int row,int direction) {
  if(direction!=1&&direction!=-1)return false;
  switch(row) {
    case 0:s.mode=static_cast<ReadingMode>(cycle(static_cast<int>(s.mode),direction,3));break;
    case 1:if(s.mode!=ReadingMode::Book)return false; s.bookLayout=s.bookLayout==BookLayout::Single?BookLayout::Spread:BookLayout::Single;break;
    case 2:s.fontSize=cycle(s.fontSize,direction,3);break;
    case 3:s.lineSpacing=cycle(s.lineSpacing,direction,3);break;
    case 4:s.margins=cycle(s.margins,direction,3);break;
    case 5: {
      auto& kind=s.animation[static_cast<int>(s.mode)];
      kind=static_cast<TransitionKind>(cycle(static_cast<int>(kind),direction,s.mode==ReadingMode::Book?3:2));break;
    }
    case 6:s.pageSound=!s.pageSound;break;
    case 7:s.soundVolume=cycle(s.soundVolume,direction,4);break;
    case 8:s.fontFamily=cycle(s.fontFamily,direction,4);break;
    case 9:s.bookmarkScreenshot=!s.bookmarkScreenshot;break;
    default:return false;
  }
  return true;
}
std::string settingValue(const ReadingSettings& s,int row) {
  const char* sizes[]={"Petite","Moyenne","Grande"};
  const char* spacing[]={"Serré","Normal","Aéré"};
  const char* margins[]={"Étroites","Normales","Larges"};
  switch(row) {
    case 0:return modeName(s.mode);
    case 1:return s.mode!=ReadingMode::Book?"BOOK uniquement":s.bookLayout==BookLayout::Single?"Une page":"Deux pages";
    case 2:return sizes[s.fontSize]; case 3:return spacing[s.lineSpacing]; case 4:return margins[s.margins];
    case 5:switch(animationFor(s)) { case TransitionKind::Curl:return "Page Curl"; case TransitionKind::Slide:return "Slide"; case TransitionKind::None:return "Aucune"; } break;
    case 6:return s.pageSound?"Activé":"Désactivé";
    case 7:return std::to_string(s.soundVolume*25)+" %";
    case 8: { const char* fonts[]={"Système","Serif","Sans","Mono"}; return fonts[s.fontFamily]; }
    case 9:return s.bookmarkScreenshot?"Activée":"Désactivée";
    default:return {};
  }
  return {};
}
TransitionKind animationFor(const ReadingSettings& s) {
  auto kind=s.animation[static_cast<int>(s.mode)];
  return s.mode!=ReadingMode::Book&&kind==TransitionKind::Curl?TransitionKind::Slide:kind;
}
bool validSettings(const ReadingSettings& s) {
  const auto mode=static_cast<int>(s.mode),layout=static_cast<int>(s.bookLayout);
  if(s.fontFamily<0||s.fontFamily>3)return false;
  if(mode<0||mode>2||layout<0||layout>1||s.fontSize<0||s.fontSize>2||s.lineSpacing<0||s.lineSpacing>2||s.margins<0||s.margins>2||s.soundVolume<0||s.soundVolume>3)return false;
  for(int i=0;i<3;++i)if(static_cast<int>(s.animation[i])<0||static_cast<int>(s.animation[i])>(i==0?2:1))return false;
  return true;
}
}
