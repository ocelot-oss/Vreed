#pragma once
#include <string>
#include <vector>
namespace vr {
struct Position { std::size_t chapter=0,page=0,offset=0; bool hasOffset=false,resumePinned=false; };
struct Book {
  std::string path,title,author,cover;
  Position position;
  enum class Format { Epub, Pdf }; Format format=Format::Epub;
};
std::vector<Book> discover(const std::string& directory);
void prepareStorage();
Position loadPosition(const std::string& path,const std::string& directory="");
bool savePosition(const std::string& path,Position position,const std::string& directory="");
}
