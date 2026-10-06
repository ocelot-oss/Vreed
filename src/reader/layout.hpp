#pragma once
#include "epub/epub.hpp"
#include <functional>
namespace vr {
struct Line { std::string text; int y=0; float scale=1; std::size_t offset=0; };
struct PageImage { std::shared_ptr<const ImageData> image; int y=0,width=0,height=0; std::size_t offset=0; };
struct Page { std::vector<Line> lines; std::size_t offset=0; std::vector<PageImage> images; };
struct Layout { int width=832, height=400, lineHeight=30; float scale=1.25f; };
using Measure=std::function<int(const std::string&,float)>;
std::vector<Page> paginate(const Chapter& chapter,const Layout& layout,const Measure& measure);
std::size_t pageForOffset(const std::vector<Page>& pages,std::size_t offset);
}
