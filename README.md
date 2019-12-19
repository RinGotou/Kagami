# Kagami Project

## Intro
It's a project of experimental scripting language. 

## Build Binary
### Dependencies
SDL2/SDL2_image/SDL2_mixer/SDL2_ttf are required.

If you want an interpreter without multimedia support, just edit DISABLE_SDL option and relax.

### Windows Platform
Compile them in Visual Studio 2017 or later versions.(MSVC 14+).
Just add all source files into a new solution, and configure SDL2 include and library directory in project settings.
You can also build them with cl.exe manually.

Notice: Mingw/Cygwin is NOT OFFICIAL SUPPORTED for now. You can post issues for these resolutions but I may not fix 
them until I start working on them.

### Linux/BSD Platform
gcc 8.0+ or clang 8.0+ are recommended, and you also need cmake/make to finish configurations.
MacOS is NOT OFFICIAL SUPPORTED for now.

```
# Archlinux/Manjaro Linux
sudo pacman -Syyu sdl2 sdl2_image sdl2_mixer sdl2_ttf

# clone & build
git clone https://github.com/kagami-project/kagami --depth=1
cd kagami
git submodule init && git submodule update
cmake .
make
```
## Help me?
You can post issues or create pull request.

## License
BSD-2-Clause
