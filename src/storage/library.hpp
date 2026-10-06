#pragma once
#include <string>
#include <vector>
namespace vr {
struct Position { std::size_t chapter=0,page=0,offset=0; bool hasOffset=false; };
struct Book {
  std::string path,title,author,cover;
  Position position;
  enum class Format { Epub }; Format format=Format::Epub;
};
std::vector<Book> discover(const std::string& directory);
void prepareStorage();
Position loadPosition(const std::string& path);
bool savePosition(const std::string& path,Position position);
}
