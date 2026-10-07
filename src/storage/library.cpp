#include "storage/library.hpp"
#include "app/config.hpp"
#include "storage/file.hpp"
#include <dirent.h>
#include <sys/stat.h>
#include <algorithm>
#include <cctype>
#include <cerrno>
#include <stdexcept>
namespace vr {
void prepareStorage() {
#ifdef VITAREADER_VITA
  if(!ensureParent(std::string(config::positions)+"/checkpoint"))throw std::runtime_error(storageFailure);
  if(!ensureParent(std::string(config::books)+"/book"))throw std::runtime_error(storageFailure);
  if(!ensureParent(std::string(config::screenshots)+"/capture.png"))throw std::runtime_error(storageFailure);
#else
  for(auto p:{"ux0:/data",config::data,config::books,config::positions})
    if(mkdir(p,0777)!=0&&errno!=EEXIST)throw std::runtime_error("Cannot create application directories");
#endif
}
std::vector<Book> discover(const std::string& directory) {
  DIR* dir=opendir(directory.c_str()); if(!dir)throw std::runtime_error("Cannot scan books directory");
  std::vector<Book> books;
  while(auto e=readdir(dir)) {
    std::string name=e->d_name; if(name.size()<4)continue;
    auto dot=name.find_last_of('.'); if(dot==std::string::npos)continue;
    auto extension=name.substr(dot); for(char& c:extension)c=static_cast<char>(std::tolower(static_cast<unsigned char>(c)));
    if(extension!=".epub"&&extension!=".pdf")continue;
    auto path=directory+"/"+name; struct stat st{}; if(stat(path.c_str(),&st)!=0||!S_ISREG(st.st_mode))continue;
    Book b; b.path=path; b.title=name.substr(0,dot); b.format=extension==".pdf"?Book::Format::Pdf:Book::Format::Epub; b.position=loadPosition(path); books.push_back(std::move(b));
    if(books.size()==config::maxBooks)break;
  }
  closedir(dir); std::sort(books.begin(),books.end(),[](const Book& a,const Book& b){return a.title<b.title;}); return books;
}
}
