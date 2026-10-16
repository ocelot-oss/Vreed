#include "input/orientation.hpp"
#include "app/app.hpp"
#include "app/config.hpp"
#include "storage/settings.hpp"
#include "storage/file.hpp"
#include <exception>
#include <cstdio>
#include <algorithm>
namespace vr {
bool App::bookmarked() const {
  return bookmark_.set&&session_.currentPageContains(bookmark_.chapter,bookmark_.offset);
}
void App::changeSetting(int direction,const Measure& measure) {
  auto candidate=settings_;
  if(!adjustSetting(candidate,settingRow_,direction))return;
  const bool reflow=settingRow_<5||settingRow_==8||settingRow_==10;
  if(reflow) {
    renderer_.prepareLayout();
    try {
      renderer_.selectFont(candidate.fontFamily); const auto next=renderer_.readingPresentation(candidate,books_[selected_].format==Book::Format::Pdf);
      auto updated=session_; updated.repaginate(next.layout,measure); renderer_.configure(next); session_=std::move(updated); presentation_=next; refreshPages_=true;
    } catch(...) { renderer_.selectFont(settings_.fontFamily); throw; }
  }
  if(candidate.orientation!=settings_.orientation) { swipe_=SwipeGesture{}; pdfTouch_=PdfGesture{}; renderer_.defaultPdfZoom(); }
  settings_=candidate; audio_.settings(settings_.pageSound,settings_.soundVolume);
  error_.clear(); if(reflow)checkpoint();
  if(!saveSettings(config::settings,settings_))error_="Réglages : "+storageFailure;
}
void App::refreshSecond(const Measure& measure) {
  if(!refreshPages_)return;
  second_={}; refreshPages_=false;
  if(presentation_.pageCount==2) {
    try { renderer_.prepareLayout(); second_=session_.secondPage(measure); }
    catch(const std::exception& e) { error_=e.what(); }
  }
}
void App::turnEvents(TurnEvents events,double nowMs) {
  if(events.sound)audio_.emit(events.direction>0?AudioEvent::PageForward:AudioEvent::PageBackward);
  if(events.commit) {
    renderer_.resetPdfPan();
    second_=std::move(candidateSecond_); refreshPages_=false;
    frameStats_.lastTurnMs=nowMs-turnStarted_; checkpoint();
  }
  if(events.cancel)candidateSecond_={};
}
void App::finishTurn(double nowMs) { turnEvents(turn_.finish(),nowMs); }
void App::requestTurn(int direction,const Measure& measure,const TouchPoint* touch) {
  finishTurn(platform_.nowMs());
  renderer_.prepareLayout(); refreshSecond(measure);
  if(!turn_.prepare(session_,direction,measure,presentation_.pageCount)) {
    feedback_.trigger(direction,platform_.nowMs()); return;
  }
  try {
    candidateSecond_={};
    if(presentation_.pageCount==2)candidateSecond_=turn_.candidate()->secondPage(measure);
    renderer_.prepareImages({&session_.page(),second_.page.get(),&turn_.candidate()->page(),candidateSecond_.page.get()});
    auto kind=books_[selected_].format==Book::Format::Pdf?TransitionKind::None:animationFor(settings_);
    if(kind!=TransitionKind::None&&!renderer_.captureTurn(books_[selected_],session_,second_,*turn_.candidate(),candidateSecond_)) {
      kind=TransitionKind::None; error_="Animation indisponible : changement instantané";
    } else error_.clear();
    turnStarted_=platform_.nowMs();
    if(touch) {
      const auto page=presentation_.paper[direction>0?presentation_.pageCount-1:0];
      turn_.beginDrag(kind,turnStarted_,touch->x,touch->y,float(page.width),float(page.y),float(page.height));
    } else turn_.start(kind,turnStarted_);
  } catch(...) { turn_.abort(); candidateSecond_={}; throw; }
}
void App::checkpoint() {
  if(books_[selected_].position.resumePinned)return;
  books_[selected_].position=session_.position;
  books_[selected_].position.resumePinned=false;
  if(!savePosition(books_[selected_].path,books_[selected_].position))error_="Position : "+storageFailure;
}
void App::run() {
  try {
    prepareStorage();
  } catch(const std::exception& e) { error_="Stockage : "+std::string(e.what()); }
  try {
    settings_=loadSettings(config::settings); audio_.settings(settings_.pageSound,settings_.soundVolume);
    try { renderer_.selectFont(settings_.fontFamily); }
    catch(const std::exception& e) { settings_.fontFamily=0; error_=e.what(); }
    books_=discover(config::books); audio_.emit(AudioEvent::StartupMusic);
  } catch(const std::exception& e) { error_=e.what(); }
  auto measure=[&](const std::string& s,float scale){return renderer_.measure(s,scale);};
  bool running=true;
  while(running) {
    auto input=platform_.poll();
    if(state_!=State::Library)orientInput(input,presentation_.orientation);
    if(state_!=State::Reader||overlay_||jumpOpen_||input.rescan||input.back||input.jump)pdfTouch_=PdfGesture{};
    if(lastFrame_>0) {
      const auto delta=input.nowMs-lastFrame_; frameStats_.frame(delta);
      if(delta>interaction::pauseGapMs)finishTurn(input.nowMs);
    }
    lastFrame_=input.nowMs;
    try {
      if(state_==State::Opening) {
        if(input.quit)running=false;
        if(input.back) { state_=State::Library; audio_.emit(AudioEvent::StartupMusic); }
        else if(input.nowMs-openingStarted_>=650)state_=State::Reader;
      } else if(state_==State::Library) {
        if(input.quit)running=false;
        if((input.up||input.previous)&&selected_>0)--selected_;
        if((input.down||input.next)&&selected_+1<books_.size())++selected_;
        if(input.rescan) { books_=discover(config::books); selected_=0; error_.clear(); }
        if(input.back)error_.clear();
        if(input.touch.phase==TouchPhase::Down&&input.touch.x>=48&&input.touch.x<918) {
          const int row=input.touch.y>=290?1:0,col=int((input.touch.x-48)/87);
          const auto index=(selected_/20)*20+row*10+col;
          if(col<10&&index<books_.size()&&input.touch.y>=100+row*208&&input.touch.y<=260+row*208) {
            if(index==selected_)input.open=true; else selected_=index;
          }
        }
        if(input.open&&!books_.empty()) {
          renderer_.prepareLayout();
          const auto next=renderer_.readingPresentation(settings_,books_[selected_].format==Book::Format::Pdf);
          session_.open(books_[selected_],measure,next.layout);
          renderer_.resetPdfPan();
          renderer_.configure(next); presentation_=next;
          renderer_.defaultPdfZoom(); pdfTouch_=PdfGesture{}; swipe_=SwipeGesture{};
          state_=State::Reader; overlay_=false; jumpOpen_=false;
          const auto saved=books_[selected_].position;
          bookmark_={saved.resumePinned,saved.chapter,saved.offset}; ribbon_.setVisible(bookmarked(),input.nowMs,true);
          audio_.emit(AudioEvent::StopMusic);
          refreshPages_=true; refreshSecond(measure); renderer_.prepareImages({&session_.page(),second_.page.get()});
          if(renderer_.captureOpening(books_,selected_,session_,second_)) { state_=State::Opening; openingStarted_=platform_.nowMs(); }
          error_.clear();
        }
      } else {
        turnEvents(turn_.tick(input.nowMs),input.nowMs);
        if(input.quit) { finishTurn(input.nowMs); checkpoint(); running=false; }
        else if(input.jump) {
          finishTurn(input.nowMs); overlay_=false;
          if(jumpOpen_)jumpOpen_=false;
          else {
            renderer_.prepareLayout(); jumpCount_=session_.totalPageCount(measure);
            jumpPage_=session_.globalPageIndex(); jumpOpen_=true;
          }
        } else if(jumpOpen_) {
          if(input.back||input.rescan)jumpOpen_=false;
          else if(input.open) {
            if(session_.goToPage(jumpPage_,measure)) {
              jumpOpen_=false; refreshPages_=true; candidateSecond_={}; error_.clear(); renderer_.resetPdfPan(); checkpoint();
            }
          } else {
            const std::size_t step=input.up||input.down?10:1;
            if(input.previous||input.up)jumpPage_=step>jumpPage_?0:jumpPage_-step;
            if(input.next||input.down)jumpPage_+=std::min(step,jumpCount_-1-jumpPage_);
            const auto track=pageChooserTrack(presentation_);
            if((input.touch.phase==TouchPhase::Down||input.touch.phase==TouchPhase::Move)&&input.touch.y>=track.y-25&&input.touch.y<=track.y+30)
              jumpPage_=std::size_t(std::clamp(double(input.touch.x-track.x)/track.width,0.0,1.0)*(jumpCount_-1)+0.5);
          }
        } else if(input.rescan) { finishTurn(input.nowMs); overlay_=!overlay_; }
        else if(overlay_) {
          if(input.back)overlay_=false;
          else if(input.up)settingRow_=(settingRow_+settingsRowCount-1)%settingsRowCount;
          else if(input.down)settingRow_=(settingRow_+1)%settingsRowCount;
          else if(input.previous||input.next)changeSetting(input.next?1:-1,measure);
        } else if(input.back) { finishTurn(input.nowMs); checkpoint(); state_=State::Library; audio_.emit(AudioEvent::StartupMusic); }
        else if(input.bookmark&&settings_.mode==ReadingMode::Book) {
          finishTurn(input.nowMs);
          const bool active=bookmarked();
          auto saved=session_.position; saved.resumePinned=!active;
          if(savePosition(books_[selected_].path,saved)) {
            books_[selected_].position=saved;
            bookmark_={!active,saved.chapter,saved.offset}; ribbon_.setVisible(!active,input.nowMs);
            audio_.emit(active?AudioEvent::BookmarkRemove:AudioEvent::BookmarkInsert); error_.clear();
            if(!active&&settings_.bookmarkScreenshot) {
              refreshSecond(measure); renderer_.prepareImages({&session_.page(),second_.page.get()});
              if(!renderer_.saveScreenshot(books_[selected_],session_,second_,config::screenshots))
                error_="Signet enregistré, capture : "+storageFailure;
            }
          } else error_="Signet : "+storageFailure;
        } else if(books_[selected_].format==Book::Format::Pdf&&(input.open||input.up||input.down)) {
          finishTurn(input.nowMs);
          if(input.open)renderer_.togglePdfZoom();
          else renderer_.panPdf(input.down?1:-1);
        } else if(input.previous||input.next) {
          pdfTouch_=PdfGesture{};
          requestTurn(input.next?1:-1,measure);
        } else if(books_[selected_].format==Book::Format::Pdf) {
          const auto action=pdfTouch_.update(input.touch,input.nowMs,presentation_.width);
          if(action.turn)requestTurn(action.turn,measure);
          else if(action.pan)renderer_.panPdfPixels(action.pan);
        } else {
          const auto direction=swipe_.update(input.touch);
          if(direction) { requestTurn(direction,measure,&swipe_.start()); turn_.motion().drag(input.touch.x,platform_.nowMs()); }
          else if(input.touch.phase==TouchPhase::Move)turn_.motion().drag(input.touch.x,input.nowMs);
          else if(input.touch.phase==TouchPhase::Up)turn_.motion().release(input.nowMs);
          else if(input.touch.phase==TouchPhase::Cancel&&turn_.motion().state()==TurnState::Dragging)turn_.motion().cancel(input.nowMs);
        }
      }
    } catch(const std::exception& e) { error_=e.what(); }
    const auto renderNow=platform_.nowMs();
    if(state_==State::Reader) {
      turnEvents(turn_.tick(renderNow),renderNow);
      const bool changed=refreshPages_; refreshSecond(measure);
      if(changed)renderer_.prepareImages({&session_.page(),second_.page.get()});
      ribbon_.setVisible(bookmarked()&&settings_.mode==ReadingMode::Book,renderNow); ribbon_.tick(renderNow);
    }
    renderer_.begin(state_!=State::Library);
    if(state_==State::Library)renderer_.library(books_,selected_,renderNow);
    else if(state_==State::Opening)renderer_.opening(float((renderNow-openingStarted_)/650),selected_);
    else {
      if(turn_.motion().active()&&turn_.motion().kind()!=TransitionKind::None) {
        renderer_.transition(turn_.motion()); renderer_.drawRibbon(ribbon_.amount());
      } else renderer_.reader(books_[selected_],session_.page(),session_.position,session_.pageCount(),session_.chapterCount(),second_,ribbon_.amount());
      renderer_.boundaryFeedback(feedback_.direction(),feedback_.amount(renderNow));
      if(overlay_)renderer_.settings(settings_,settingRow_);
      if(jumpOpen_)renderer_.pageChooser(jumpPage_,jumpCount_);
    }
    if(!error_.empty())renderer_.message(error_);
    renderer_.finish();
#ifndef NDEBUG
    if(frameStats_.frames&&frameStats_.frames%300==0)std::fprintf(stderr,"VitaReader frames=%llu mean=%.2fms max=%.2fms gaps=%llu turn=%.2fms\n",
      static_cast<unsigned long long>(frameStats_.frames),frameStats_.totalMs/frameStats_.frames,frameStats_.maxMs,
      static_cast<unsigned long long>(frameStats_.longGaps),frameStats_.lastTurnMs);
#endif
  }
}
}
