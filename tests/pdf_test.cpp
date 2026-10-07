#include "pdf/pdf.hpp"
#include "epub/epub.hpp"
#include <fstream>
#include <iostream>
#include <stdexcept>
static void require(bool value,const char* message) { if(!value)throw std::runtime_error(message); }
int main(int argc,char** argv) {
  if(argc!=2)return 2;
  // Two different colored pages verify the actual renderer and page indexing.
  const std::string objects[]={
    "<< /Type /Catalog /Pages 2 0 R >>",
    "<< /Type /Pages /Kids [3 0 R 4 0 R] /Count 2 >>",
    "<< /Type /Page /Parent 2 0 R /MediaBox [0 0 200 300] /Contents 5 0 R >>",
    "<< /Type /Page /Parent 2 0 R /MediaBox [0 0 200 300] /Contents 6 0 R >>",
    "<< /Length 26 >>\nstream\n1 0 0 rg 0 0 200 300 re f\nendstream",
    "<< /Length 26 >>\nstream\n0 0 1 rg 0 0 200 300 re f\nendstream"};
  std::string pdf="%PDF-1.4\n"; std::size_t offsets[7]={};
  for(int i=0;i<6;++i) { offsets[i+1]=pdf.size(); pdf+=std::to_string(i+1)+" 0 obj\n"+objects[i]+"\nendobj\n"; }
  const auto xref=pdf.size(); pdf+="xref\n0 7\n0000000000 65535 f \n";
  for(int i=1;i<=6;++i) { const auto n=std::to_string(offsets[i]); pdf+=std::string(10-n.size(),'0')+n+" 00000 n \n"; }
  pdf+="trailer\n<< /Size 7 /Root 1 0 R >>\nstartxref\n"+std::to_string(xref)+"\n%%EOF\n";
  { std::ofstream file(argv[1],std::ios::binary); file<<pdf; }
  vr::PdfDocument document(argv[1]); require(document.count()==2,"page count");
  auto red=document.page(0),blue=document.page(1);
  require(red->rgba&&red->width>0&&red->height>0,"RGBA raster");
  require(red->bytes.size()==std::size_t(red->width)*red->height*4,"raster size");
  const auto center=(std::size_t(red->height/2)*red->width+red->width/2)*4;
  require(static_cast<unsigned char>(red->bytes[center])>240&&static_cast<unsigned char>(red->bytes[center+2])<10,"red page");
  require(static_cast<unsigned char>(blue->bytes[center+2])>240,"blue page");
  vr::Epub book; book.open(argv[1]); require(book.isPdf()&&book.spine.size()==2,"PDF book integration");
  require(book.chapter(1).blocks.size()==1,"PDF page integration");
  bool rejected=false; try {document.page(2);}catch(const std::exception&){rejected=true;}
  require(rejected,"invalid page rejected");
  std::cout<<"MuPDF PDF raster and integration passed\n";
}
