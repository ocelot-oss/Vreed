#include "storage/library_settings.hpp"
#include "storage/library.hpp"
#include <filesystem>
#include <fstream>
#include <stdexcept>
static void require(bool v) { if(!v)throw std::runtime_error("Library directory regression"); }
int main(int argc,char** argv) {
  using namespace vr; namespace fs=std::filesystem;
  if(argc!=2)return 2;
  const fs::path root=argv[1]; fs::create_directories(root/"Mes livres"/"Romans");
  fs::create_directories(root/"Mes livres"/"BD");
  const auto cfg=(root/"library.cfg").string();
  require(loadLibraryDirectory(cfg)==config::books);
  require(normalizeLibraryDirectory("uma0:/Mes livres/EPUB/")=="uma0:/Mes livres/EPUB");
  require(normalizeLibraryDirectory("ux0:/")=="ux0:/");
  for(auto p:{"relative/books","ux0:/../secret","ux0:/data//books","ux0:/./books","ux0:/bad\npath","ux0:/other:/books"})require(normalizeLibraryDirectory(p).empty());
  require(parentDirectory("uma0:/Mes livres/EPUB")=="uma0:/Mes livres");
  require(parentDirectory("ux0:/data")=="ux0:/"); require(parentDirectory("ux0:/")=="ux0:/");
  require(joinDirectory("ux0:/","books")=="ux0:/books");
  require(saveLibraryDirectory(cfg,"uma0:/Mes livres/EPUB/"));
  require(loadLibraryDirectory(cfg)=="uma0:/Mes livres/EPUB");
  require(!saveLibraryDirectory(cfg,"relative")); require(loadLibraryDirectory(cfg)=="uma0:/Mes livres/EPUB");
  fs::rename(cfg,cfg+".bak"); require(loadLibraryDirectory(cfg)=="uma0:/Mes livres/EPUB");
  { std::ofstream f(cfg); f<<"VRL1\nux0:/../invalid\n"; }
  require(loadLibraryDirectory(cfg)==config::books);
  { std::ofstream f(root/"Mes livres"/"Test.EPUB"); f<<"fixture"; }
  { std::ofstream f(root/"Mes livres"/"Plan.PDF"); f<<"fixture"; }
  { std::ofstream f(root/"Mes livres"/"ignore.txt"); f<<"fixture"; }
  { std::ofstream f(root/"Mes livres"/"Romans"/"Nested.epub"); f<<"fixture"; }
  const auto dirs=directories((root/"Mes livres").string()); require(dirs==std::vector<std::string>({"BD","Romans"}));
  const auto books=discover((root/"Mes livres").string()); require(books.size()==2);
  require(books[0].format==Book::Format::Pdf&&books[1].format==Book::Format::Epub);
  bool rejected=false; try { directories((root/"missing").string()); }catch(const std::exception&){rejected=true;} require(rejected);
}
