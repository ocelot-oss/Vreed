#pragma once
#include <string>
#include <vector>
#include "epub/image.hpp"
namespace vr {
class PdfDocument;
struct Run { std::string text; bool bold=false, italic=false; };
struct Block { enum class Kind { Paragraph, Heading, Image }; Kind kind=Kind::Paragraph; std::vector<Run> runs; std::shared_ptr<const ImageData> image; };
struct Chapter { std::vector<Block> blocks; };
class Epub {
public:
  void open(const std::string& path);
  Chapter chapter(std::size_t index) const;
  std::string title, author, cover;
  std::vector<std::string> spine;
  bool isPdf() const { return bool(pdf_); }
private:
  std::shared_ptr<PdfDocument> pdf_;
  std::string path_;
  std::string entry(const std::string& name) const;
};
}
