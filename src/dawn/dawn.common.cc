#include "dawn.common.h"

namespace dawn {
  bool &GetAudioSubsystemState() {
    static std::mutex state_lock;
    std::lock_guard<std::mutex> guard(state_lock);
    static bool state = false;
    return state;
  }

  int EnvironmentSetup(AudioOption audio) {
    if (SDL_Init(SDL_INIT_EVERYTHING) != 0) {
      return -1;
    }

    if (Mix_Init(audio.flags) == 0) {
      return -2;
    }

    if (Mix_OpenAudio(
      audio.frequency, 
      audio.format, 
      audio.channels, 
      audio.chunksize) != 0) {
      return -3;
    }

    GetAudioSubsystemState() = true;

    return 0;
  }

  void EnvironmentCleanup() {
    IMG_Quit();
    TTF_Quit();
    Mix_CloseAudio();
    Mix_Quit();

    GetAudioSubsystemState() = false;

    SDL_Quit();
  }

  bool IsAudioSubsystemLoaded() {
    return GetAudioSubsystemState();
  }
}