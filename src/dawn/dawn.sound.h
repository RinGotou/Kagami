#pragma once
#include "dawn.common.h"

namespace dawn {
  using Channel = int;

  class Music {
  protected:
    Mix_Music *ptr_;

  public:
    virtual ~Music() {
      if (IsAudioSubsystemLoaded()) {
        Mix_FreeMusic(ptr_);
      }
    }

    Music() = delete;

    Music(string path) : ptr_(Mix_LoadMUS(path.c_str())) {}

    Music(SDL_RWops *ops) : ptr_(Mix_LoadMUS_RW(ops,0)) {}

    Channel Play(int loop = 0) {
      return Mix_PlayMusic(ptr_, loop);
    }

    auto GetType() {
      return Mix_GetMusicType(ptr_);
    }

    bool Good() const { return ptr_ != nullptr; }
  };

  class SoundEffect {
  protected:
    Mix_Chunk *ptr_;

  public:
    virtual ~SoundEffect() {
      if (IsAudioSubsystemLoaded()) {
        Mix_FreeChunk(ptr_);
      }
    }

    SoundEffect() = delete;

    SoundEffect(string path) : ptr_(Mix_LoadWAV(path.c_str())) {}

    Channel Play(Channel channel = -1, int loop = 0, int tick = -1) {
      return Mix_PlayChannelTimed(channel, ptr_, loop, tick);
    }
  };

  using ManagedMusic = shared_ptr<Music>;
  using ManagedSoundEffect = shared_ptr<SoundEffect>;
}