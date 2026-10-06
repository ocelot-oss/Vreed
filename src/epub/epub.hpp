#pragma once
#include <string>
#include <vector>
namespace vr {
struct Run { std::string text; bool bold=false, italic=false; };
struct Block { enum class Kind { Paragraph, Heading, Image }; Kind kind=Kind::Paragraph; std::vector<Run> runs; };
struct Chapter { std::vector<Block> blocks; };
class Epub {
public:
  void open(const std::string& path);
  Chapter chapter(std::size_t index) const;
  std::string title, author, cover;
  std::vector<std::string> spine;
private:
  std::string path_;
  std::string entry(const std::string& name) const;
};
}
