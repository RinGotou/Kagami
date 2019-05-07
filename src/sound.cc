#include "sound.h"

namespace kagami {
#if not defined(_DISABLE_SDL_)
  Message NewMusicObject(ObjectMap &p) {
    EXPECT_TYPE(p, "path", kTypeIdString);
    string path = p.Cast<string>("path");

#if defined(_WIN32) && defined(HAVE_STDIO_H)
    wstring wpath = s2ws(path);
    auto *fp = _wfopen(wpath.data(), L"r");
    auto *ops = SDL_RWFromFP(fp, SDL_TRUE);
    dawn::ManagedMusic music(new dawn::Music(ops));
#else
    dawn::ManagedMusic music(new dawn::Music(path));
#endif

    if (!music->Good()) return Message().SetObject(false);

    return Message().SetObject(Object(music, kTypeIdMusic));
  }

  Message MusicPlay(ObjectMap &p) {
    dawn::Music &music = p.Cast<dawn::Music>(kStrMe);
    return Message().SetObject(static_cast<int64_t>(music.Play()));
    
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
    using management::type::ObjectTraitsSetup;
    using management::CreateImpl;
    using management::type::PointerHasher;

    ObjectTraitsSetup(kTypeIdMusic,ShallowDelivery, PointerHasher())
      .InitConstructor(
       FunctionImpl(NewMusicObject,"path",kTypeIdMusic)
      )
      .InitMethods(
        {
          FunctionImpl(MusicPlay,"","play")
        }
    );

    CreateImpl(FunctionImpl(MusicPause, "", "PauseMusic"));
    CreateImpl(FunctionImpl(MusicResume, "", "ResumeMusic"));
    CreateImpl(FunctionImpl(MusicHalt, "", "HaltMusic"));
  }
#endif
}