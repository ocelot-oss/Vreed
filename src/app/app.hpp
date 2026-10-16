#pragma once
#include "platform/vita.hpp"
#include "rendering/renderer.hpp"
#include "reader/session.hpp"
#include "reader/turn_transaction.hpp"
#include "platform/vita_audio.hpp"
#include "platform/frame_stats.hpp"
#include "input/pdf_gesture.hpp"
#include "storage/library_settings.hpp"
namespace vr {
class App {
public: void run();
private:
  enum class State { Library,Opening,Reader }; State state_=State::Library;
  double openingStarted_=0;
  Vita platform_; Renderer renderer_; Session session_;
  VitaAudio audio_;
  ReaderTurn turn_;
  SwipeGesture swipe_;
  PdfGesture pdfTouch_;
  RibbonMotion ribbon_;
  EdgeFeedback feedback_;
  FrameStats frameStats_;
  double lastFrame_=0,turnStarted_=0;
  std::vector<Book> books_; std::size_t selected_=0; std::string error_;
  ReadingSettings settings_; Presentation presentation_{};
  Session::VisiblePage second_;
  Session::VisiblePage candidateSecond_;
  bool refreshPages_=true;
  bool overlay_=false;
  bool jumpOpen_=false;
  std::size_t jumpPage_=0,jumpCount_=1;
  int settingRow_=0;
  std::string libraryDirectory_=config::books,browseDirectory_;
  bool libraryOptions_=false,directoryBrowser_=false;
  int libraryOption_=0,directoryOption_=0;
  std::vector<std::string> directoryChoices_;
  void browseDirectory(const std::string& path);
  void chooseDirectory(const std::string& path);
  void libraryOptionsInput(const Input& input);
  struct TemporaryBookmark { bool set=false; std::size_t chapter=0,offset=0; } bookmark_;
  bool bookmarked() const;
  void changeSetting(int direction,const Measure& measure);
  void refreshSecond(const Measure& measure);
  void requestTurn(int direction,const Measure& measure,const TouchPoint* touch=nullptr);
  void turnEvents(TurnEvents events,double nowMs);
  void finishTurn(double nowMs);
  void checkpoint();
};
}
