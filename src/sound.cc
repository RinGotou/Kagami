#include "sound.h"

namespace kagami {
  Message NewMusicObject(ObjectMap &p) {
    EXPECT_TYPE(p, "path", kTypeIdString);
    string path = p.Cast<string>("path");

    dawn::ManagedMusic music(new dawn::Music(path));

    if (!music->Good()) return Message().SetObject(false);

    return Message().SetObject(Object(music, kTypeIdMusic));
  }

  Message MusicPlay(ObjectMap &p) {
    dawn::Music &music = p.Cast<dawn::Music>(kStrObject);
    return Message().SetObject(static_cast<long>(music.Play()));
    
  }

  Message MusicPause(ObjectMap &p) {
    if (Mix_PlayingMusic() == 0) {
      Mix_PauseMusic();
    }

    return Message();
  }

  Message MusicResume(ObjectMap &p) {
    if (Mix_PausedMusic() == 0) {
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
}