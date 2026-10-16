#include "storage/language.hpp"
#include "reader/settings.hpp"
#include <fstream>
#include <stdexcept>
#include <cstdio>
static void require(bool value) { if(!value)throw std::runtime_error("Language regression"); }
int main(int argc,char** argv) {
  using namespace vr;
  if(argc!=2)return 2;
  const std::string cfg=argv[1];
  std::remove(cfg.c_str()); std::remove((cfg+".bak").c_str());
  require(uiLanguage==Language::English&&loadLanguage(cfg)==Language::English);
  ReadingSettings settings;
  require(settingValue(settings,8)=="System");
  require(settingValue(settings,10)=="Landscape");
  require(saveLanguage(cfg,Language::French)); require(loadLanguage(cfg)==Language::French);
  uiLanguage=loadLanguage(cfg);
  require(std::string(tr("Library","Bibliothèque"))=="Bibliothèque");
  require(settingValue(settings,8)=="Système"); require(settingValue(settings,10)=="Horizontal");
  for(int row=0;row<settingsRowCount;++row)require(!settingValue(settings,row).empty());
  require(!saveLanguage(cfg,static_cast<Language>(99))); require(loadLanguage(cfg)==Language::French);
  require(std::rename(cfg.c_str(),(cfg+".bak").c_str())==0); require(loadLanguage(cfg)==Language::French);
  {std::ofstream file(cfg);file<<"VRLANG1\nde\n";} require(loadLanguage(cfg)==Language::English);
  {std::ofstream file(cfg);file<<"VRLANG1\nfr\nextra\n";} require(loadLanguage(cfg)==Language::English);
  require(saveLanguage(cfg,Language::English)); uiLanguage=loadLanguage(cfg);
  require(uiLanguage==Language::English&&settingValue(settings,8)=="System");
}
