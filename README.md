# Kagami Project

Just another wild flower.

## Intro
It's a project of experimental script language. The name of this project is from Japanese kanji "鏡"(かがみ).

## Design
I want to create a simple language for simple visual novel developing. Kagami is design for logic 
implementation, not CPU intensive job. This project contains a try-catch free implementation with C++17.

## Build Kagami Interpreter

### Dependencies
SDL2/SDL2_image/SDL2_mixer/SDL2_ttf are required.

If you want a interpreter without graphic support, just edit DISABLE_SDL option and relax.

### Windows Platform
Compile them in Visual Studio 2017 or later version.(MSVC 14+).

Just add all source files into a new solution, and configure SDL2 include and library directory in project settings.

### Linux/BSD Platform
I recommend using gcc 8.0+ or clang 8.0+.

```
# Archlinux/Manjaro Linux
sudo pacman -Syyu sdl2 sdl2_image sdl2_mixer sdl2_ttf

#clone & build
git clone https://github.com/kagami-project/kagami
cd kagami
git submodule init && git submodule update
cmake .
make
```
## Help me?
You can post issues or create pull request.

## License
BSD-2-Clause
