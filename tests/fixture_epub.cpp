// Synthetic provider, intentionally separate from the real EPUB parser.
#include "epub/epub.hpp"
#include <stdexcept>
namespace vr {
void Epub::open(const std::string& path) {
  if(path=="invalid")throw std::runtime_error("Synthetic open failure");
  path_=path; title="Fixture"; spine={"one","two","three"};
}
Chapter Epub::chapter(std::size_t index) const {
  if(path_=="broken"&&index==1)throw std::runtime_error("Synthetic chapter failure");
  Chapter c;
  const int lengths[]={3,2,1};
  for(int i=0;i<lengths[index];++i) {
    Block block; block.runs.push_back({std::string(1,char('A'+index))+std::to_string(i)}); c.blocks.push_back(block);
  }
  return c;
}
}
