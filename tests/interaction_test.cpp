#include "reader/turn_transaction.hpp"
#include "input/touch.hpp"
#include "rendering/curl.hpp"
#include "storage/settings.hpp"
#include "audio/events.hpp"
#include "audio/stereo_mixer.hpp"
#include <cassert>
#include <cmath>
#include <cstdio>
#include <fstream>
#include <iostream>
#include <algorithm>
#include <filesystem>
#include "rendering/typography.hpp"
using namespace vr;
int main(int argc,char** argv) {
  {
    StereoMixer mixer;
    std::vector<std::int16_t> recording(24000),effects(12000,0),output(24000);
    for(std::size_t i=0;i<12000;++i) { recording[i*2]=12000; recording[i*2+1]=-8000; }
    mixer.mix(output.data(),recording.data(),recording.size(),effects.data(),12000,true,true,2);
    assert(output[23998]==6000&&output[23999]==-4000); // Channel order and exact playback rate.
    assert(output[0]==0&&mixer.audible());
    mixer.mix(output.data(),recording.data(),recording.size(),effects.data(),12000,false,true,2);
    assert(output[23998]==0&&output[23999]==0&&!mixer.audible());
    mixer.mix(output.data(),recording.data(),0,effects.data(),12000,true,true,3);
    for(auto sample:output)assert(sample==0); // Short/missing reads cannot reuse old PCM.
  }
  assert(argc==2); const std::string file=argv[1];
  PageTransition motion;
  assert(motion.start(1,TransitionKind::Curl,0));
  assert(!motion.start(-1,TransitionKind::Slide,0));
  auto event=motion.tick(100); assert(!event.commit&&!event.sound&&motion.progress()>0);
  event=motion.tick(240); assert(event.sound&&!event.commit);
  assert(!motion.tick(260).sound);
  event=motion.tick(360); assert(event.commit&&!event.sound&&!motion.active());
  assert(!motion.tick(400).commit);
  assert(motion.start(-1,TransitionKind::Slide,500));
  event=motion.tick(660); assert(event.commit&&event.direction==-1&&event.sound);
  assert(motion.start(1,TransitionKind::None,700)); assert(motion.tick(700).commit);
  assert(motion.beginDrag(1,TransitionKind::Curl,1000,900,100,800,20,500));
  motion.drag(800,1200); assert(std::abs(motion.progress()-0.125f)<0.001f);
  assert(!motion.tick(1300).sound); motion.release(1400);
  assert(motion.state()==TurnState::Cancelling);
  event=motion.tick(2000); assert(event.cancel&&!event.commit&&!event.sound&&motion.progress()==0);
  assert(motion.beginDrag(1,TransitionKind::Curl,2100,900,500,800,20,500));
  motion.drag(580,2400); motion.release(2600); assert(motion.state()==TurnState::Completing);
  event=motion.tick(3000); assert(event.commit&&event.sound);
  assert(motion.beginDrag(-1,TransitionKind::Curl,3100,50,400,450,20,500));
  motion.drag(95,3120); motion.release(3120); assert(motion.state()==TurnState::Completing); // Fast 10% flick.
  assert(motion.finish().commit);
  assert(motion.beginDrag(1,TransitionKind::Curl,3200,900,100,800,20,500));
  motion.drag(200,3210); assert(motion.finish().cancel); // Exit/settings cancels an unaccepted drag, regardless of distance.

  TouchTracker tracker; Contact contact{7,890,300};
  assert(tracker.sample(&contact,1).phase==TouchPhase::Down);
  assert(tracker.sample(&contact,1).phase==TouchPhase::Move);
  assert(tracker.sample(&contact,2).phase==TouchPhase::Cancel);
  assert(tracker.sample(&contact,1).phase==TouchPhase::None);
  tracker.sample(nullptr,0); assert(tracker.sample(&contact,1).phase==TouchPhase::Down);
  contact.id=8; assert(tracker.sample(&contact,1).phase==TouchPhase::Cancel);
  tracker.sample(nullptr,0); tracker.sample(&contact,1);
  assert(tracker.sample(nullptr,0).phase==TouchPhase::Up);
  ReadingSettings settings; auto view=presentation(settings,[](float s){return int(s*20);});
  assert(pageEdge(view,{TouchPhase::Down,1,880,300})==1);
  assert(pageEdge(view,{TouchPhase::Down,1,75,300})==-1);
  assert(pageEdge(view,{TouchPhase::Down,1,480,300})==0);
  for(int spread=0;spread<2;++spread) {
    settings.bookLayout=spread?BookLayout::Spread:BookLayout::Single; view=presentation(settings,[](float s){return int(s*20);});
    for(int step=0;step<=20;++step) {
      const auto forward=curlGeometry(view,1,step/20.0f,0.15f),backward=curlGeometry(view,-1,step/20.0f,0.85f);
      for(int i=0;i<interaction::curlSegments;++i)for(int j=0;j<4;++j) {
        const auto a=forward.bands[i].vertices[j],b=backward.bands[i].vertices[j];
        assert(std::isfinite(a.x)&&std::isfinite(a.y)&&a.u>=0&&a.u<=1&&a.v>=0&&a.v<=1);
        assert(std::abs(a.x+b.x-960)<0.01f);
      }
      assert(forward.shadowStrength>=0&&forward.shadowStrength<=0.22f);
      if(step>10)assert(forward.bands[0].backside);
    }
  }
  RibbonMotion ribbon; ribbon.setVisible(true,0); ribbon.tick(110);
  assert(ribbon.amount()>0.4f&&ribbon.amount()<0.6f);
  ribbon.setVisible(false,110); ribbon.tick(330); assert(ribbon.amount()==0);
  ribbon.setVisible(true,400); ribbon.tick(620); assert(ribbon.amount()==1);

  assert(animationFor(settings)==TransitionKind::Curl);
  settings.animation[0]=TransitionKind::None; settings.pageSound=false; settings.soundVolume=3;
  settings.mode=ReadingMode::Oled; settings.fontSize=2;
  assert(saveSettings(file,settings)); auto loaded=loadSettings(file);
  assert(loaded.mode==settings.mode&&loaded.animation[0]==TransitionKind::None&&!loaded.pageSound&&loaded.soundVolume==3&&loaded.fontSize==2);
  settings.pageSound=true; settings.soundVolume=0; assert(saveSettings(file,settings));
  assert(loadSettings(file).pageSound&&loadSettings(file).soundVolume==0);
  { std::ofstream bad(file); bad<<"VRS1\n99 0 1 1 1 2 1 1 1 1\n"; }
  assert(loadSettings(file).mode==ReadingMode::Book);
  std::remove(file.c_str());
  const auto positions=(std::filesystem::path(file).parent_path()/"position-test").string();
  std::filesystem::create_directories(positions);
  Position resume{2,3,123,true,true};
  for(int i=0;i<10;++i) {
    resume.offset=123+i; assert(savePosition("test-book.epub",resume,positions));
    const auto loaded=loadPosition("test-book.epub",positions);
    assert(loaded.resumePinned&&loaded.hasOffset&&loaded.offset==resume.offset&&loaded.chapter==2);
  }
  resume.resumePinned=false; assert(savePosition("test-book.epub",resume,positions));
  assert(!loadPosition("test-book.epub",positions).resumePinned);

  AudioEvents sounds; AudioEvent sound;
  assert(!sounds.pop(sound));
  for(int i=0;i<15;++i)assert(sounds.push(AudioEvent::PageForward));
  assert(!sounds.push(AudioEvent::PageBackward));
  for(int i=0;i<15;++i)assert(sounds.pop(sound)&&sound==AudioEvent::PageForward);
  assert(!sounds.pop(sound));
  PaperSounds bank; std::array<std::int16_t,512> pcm{};
  bank.play(AudioEvent::PageForward); bank.render(pcm.data(),pcm.size(),1);
  assert(std::any_of(pcm.begin(),pcm.end(),[](auto sample){return sample!=0;}));
  bank.render(pcm.data(),pcm.size(),0); assert(std::all_of(pcm.begin(),pcm.end(),[](auto sample){return sample==0;}));
  bank.stop(); bank.render(pcm.data(),pcm.size(),3); assert(std::all_of(pcm.begin(),pcm.end(),[](auto sample){return sample==0;}));
  bank.play(AudioEvent::StartupMusic); bank.render(pcm.data(),pcm.size(),1);
  assert(std::any_of(pcm.begin(),pcm.end(),[](auto sample){return sample!=0;}));
  bank.play(AudioEvent::StopMusic); bank.render(pcm.data(),pcm.size(),3);
  assert(std::all_of(pcm.begin(),pcm.end(),[](auto sample){return sample==0;}));
  settings.mode=ReadingMode::Clean; view=presentation(settings,[](float s){return int(s*20);});
  assert(pageEdge(view,{TouchPhase::Down,1,950,300})==1);
  assert(pageEdge(view,{TouchPhase::Down,1,4,300})==-1);
  SwipeGesture swipe;
  for(int i=0;i<20;++i) {
    const float start=180+i*25;
    assert(swipe.update({TouchPhase::Down,1,start,250})==0);
    assert(swipe.update({TouchPhase::Move,1,start-30,251})==1);
    assert(swipe.start().x==start);
    swipe.update({TouchPhase::Up,1,start-200,251});
    swipe.update({TouchPhase::Down,1,start,250});
    assert(swipe.update({TouchPhase::Move,1,start+30,249})==-1);
    swipe.update({TouchPhase::Cancel,1,start,250});
  }
  swipe.update({TouchPhase::Down,1,400,250}); assert(!swipe.update({TouchPhase::Move,1,402,300})&&!swipe.pending());
  const auto raw=[](const std::string& s,float){return s=="'"?40:int(s.size())*10;};
  std::vector<int> glyphPositions;
  const auto width=textAdvance("L’homme",1,raw,[&](int x,const std::string&){glyphPositions.push_back(x);});
  assert(width==64&&glyphPositions.size()==3&&glyphPositions[0]==0&&glyphPositions[1]==10&&glyphPositions[2]==14);
  assert(latinPunctuation("l’homme, dʼabord, ＇oui＇")=="l'homme, d'abord, 'oui'");

  auto measure=[](const std::string& s,float){return int(s.size())*10;};
  Layout layout; layout.width=60; layout.height=20; layout.lineHeight=20;
  Book book; book.path="good"; Session session; session.open(book,measure,layout); ReaderTurn turn;
  assert(!turn.prepare(session,-1,measure,2)); // Beginning boundary.
  assert(turn.prepare(session,1,measure,2));
  auto old=session.position; assert(turn.start(TransitionKind::Curl,0));
  assert(session.position.chapter==old.chapter&&session.position.page==old.page);
  event=turn.tick(360); assert(event.commit&&session.position.page==2&&session.position.offset==session.page().offset);
  assert(turn.prepare(session,1,measure,2)); assert(turn.start(TransitionKind::Curl,400));
  event=turn.finish(); assert(event.commit&&session.position.chapter==1&&session.position.page==1);
  assert(!turn.prepare(session,1,measure,2)); // End boundary: final right page is already visible.
  assert(turn.prepare(session,-1,measure,2)); assert(turn.beginDrag(TransitionKind::Curl,500,50,300,450,20,500));
  turn.motion().drag(80,650); turn.motion().release(800); event=turn.tick(1300);
  assert(event.cancel&&session.position.chapter==1&&session.position.page==1);
  assert(turn.prepare(session,-1,measure,2)); assert(turn.start(TransitionKind::Slide,1400));
  assert(turn.finish().commit&&session.position.chapter==0&&session.position.page==2);
  assert(turn.prepare(session,-1,measure,2)); assert(turn.start(TransitionKind::Slide,1500));
  assert(turn.finish().commit&&session.position.chapter==0&&session.position.page==0);
  for(int i=0;i<5;++i) {
    assert(turn.prepare(session,1,measure,1)); assert(turn.start(TransitionKind::Curl,2000+i*20)); assert(turn.finish().commit);
  }
  assert(session.position.chapter==2&&session.position.page==0&&!turn.prepare(session,1,measure,1));
  std::cout<<"interaction: completion/cancel/flick, touch ownership, mirrored curl math, ribbon reversal, persisted settings, audio queue and transactional rapid/spread navigation OK\n";
}
