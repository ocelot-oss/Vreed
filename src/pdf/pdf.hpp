#pragma once
#include "epub/image.hpp"
#include <memory>
namespace vr {
class PdfDocument {
public:
  explicit PdfDocument(const std::string& path);
  ~PdfDocument();
  PdfDocument(const PdfDocument&)=delete;
  PdfDocument& operator=(const PdfDocument&)=delete;
  std::size_t count() const;
  std::shared_ptr<const ImageData> page(std::size_t index) const;
  std::string title,author;
private:
  struct State;
  std::unique_ptr<State> state_;
};
}
