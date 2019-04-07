#include "sound.h"

namespace kagami {
#if not defined(_DISABLE_SDL_)
  Message NewMusicObject(ObjectMap &p) {
    EXPECT_TYPE(p, "path", kTypeIdString);
    string path = p.Cast<string>("path");

#if defined(_WIN32) && defined(HAVE_STDIO_H)
    wstring wpath = s2ws(path);
    auto *fp = _wfopen(wpath.c_str(), L"r");
    auto *ops = SDL_RWFromFP(fp, SDL_TRUE);
    dawn::ManagedMusic music(new dawn::Music(ops));
#else
    dawn::ManagedMusic music(new dawn::Music(path));
#endif

    if (!music->Good()) return Message().SetObject(false);

    return Message().SetObject(Object(music, kTypeIdMusic));
  }

  Message MusicPlay(ObjectMap &p) {
    dawn::Music &music = p.Cast<dawn::Music>(kStrObject);
    return Message().SetObject(static_cast<long>(music.Play()));
    
  }

  Message MusicPause(ObjectMap &p) {
    if (Mix_PlayingMusic() == 1) {
      Mix_PauseMusic();
    }
    
    return Message();
  }

  Message MusicResume(ObjectMap &p) {
    if (Mix_PausedMusic() == 1) {
      Mix_ResumeMusic();
    }

    return Message();
  }

  Message MusicHalt(ObjectMap &p) {
    Mix_HaltMusic();
    return Message();
  }

  void InitSoundComponents() {
    using management::type::NewTypeSetup;
    using management::CreateNewInterface;

    NewTypeSetup(kTypeIdMusic,FakeCopy<dawn::Music>)
      .InitConstructor(
       Interface(NewMusicObject,"path",kTypeIdMusic)
      )
      .InitMethods(
        {
          Interface(MusicPlay,"","Play")
        }
    );

    CreateNewInterface(Interface(MusicPause, "", "PauseMusic"));
    CreateNewInterface(Interface(MusicResume, "", "ResumeMusic"));
    CreateNewInterface(Interface(MusicHalt, "", "HaltMusic"));
  }
#endif
}