#include "sound.h"

namespace kagami {
#ifndef _DISABLE_SDL_
  Message NewMusicObject(ObjectMap &p) {
    EXPECT_TYPE(p, "path", kTypeIdString);
    string path = p.Cast<string>("path");
    dawn::ManagedMusic music(new dawn::Music(path));

    if (!music->Good()) return Message().SetObject(false);

    return Message().SetObject(Object(music, kTypeIdMusic));
  }

  Message MusicPlay(ObjectMap &p) {
    dawn::Music &music = p.Cast<dawn::Music>(kStrMe);
    return Message().SetObject(static_cast<int64_t>(music.Play(-1)));
  }

  Message MusicGood(ObjectMap &p) {
    dawn::Music &music = p.Cast<dawn::Music>(kStrMe);
    return Message().SetObject(music.Good());
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

    ObjectTraitsSetup(kTypeIdMusic, ShallowDelivery, PointerHasher)
      .InitConstructor(
        FunctionImpl(NewMusicObject, "path", kTypeIdMusic)
      )
      .InitMethods(
        {
          FunctionImpl(MusicPlay,"","play"),
          FunctionImpl(MusicGood,"","good")
        }
    );

    CreateImpl(FunctionImpl(MusicPause, "", "PauseMusic"));
    CreateImpl(FunctionImpl(MusicResume, "", "ResumeMusic"));
    CreateImpl(FunctionImpl(MusicHalt, "", "HaltMusic"));
  }
#endif
}