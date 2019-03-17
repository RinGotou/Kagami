/*
  Common header of Dawn
*/
#pragma once
#include <SDL.h>
#include <SDL_image.h>
#include <SDL_mixer.h>
#include <SDL_ttf.h>
#include <string>
#include <utility>
#include <memory>
#include <thread>
#include <mutex>

#if defined(_WIN32)
#pragma comment(lib,"SDL2.lib")
#pragma comment(lib,"SDL2main.lib")
#if defined(_DEBUG)
#pragma comment(lib,"SDL2test.lib")
#endif
#pragma comment(lib,"SDL2_image.lib")
#pragma comment(lib,"SDL2_mixer.lib")
#pragma comment(lib,"SDL2_ttf.lib")
#else
/* Reserved for unix-like environment */
#endif

#define FRAMEWORK_ID "Dawn"

namespace dawn {
  using std::string;
  using std::shared_ptr;
  using std::make_shared;

  struct AudioOption {
    int frequency;
    uint16_t format;
    int channels;
    int chunksize;
    int flags;
  };

  const AudioOption kDefaultAudioOpt = {
    44100,
    MIX_DEFAULT_FORMAT,
    MIX_DEFAULT_CHANNELS,
    2048,
    MIX_INIT_FLAC | MIX_INIT_MOD | MIX_INIT_MP3 | MIX_INIT_OGG
  };

  int EnvironmentSetup(AudioOption audio = kDefaultAudioOpt);
  void EnvironmentCleanup();
  bool IsAudioSubsystemLoaded();
}