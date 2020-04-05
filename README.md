# Kagami Project Core

## Intro
It's the experimental implementaion of Kagami Scripting Language.
- It's EXTREMELY SLOW while doing some CPU-intensive works.(Such as sorting. VERY SLOW!!!)
- No heap management. This impl only contains a simple scope-based object storage.
- This impl will be replaced if new interpreter is ready to use.
- This project may be renamed in the future.

## Build Binary
### Dependencies
SDL2/SDL2_image/SDL2_mixer/SDL2_ttf are required.

### Windows Platform
Compile them in Visual Studio 2017 or later versions.(MSVC 14+).
Just add all source files into a new solution, and configure SDL2 include and library directory in project settings.
You can also build them with cl.exe manually.

Notice: Mingw/Cygwin is NOT OFFICIAL SUPPORTED for now. You can post issues for these resolutions but I may not fix 
them until I start working on them.

### Linux/BSD Platform
gcc 8.0+ or clang 8.0+ are recommended, and you also need cmake/make to finish configurations.
MacOS is NOT OFFICIAL SUPPORTED for now and I don't have Apple device for testing.

```
# Archlinux/Manjaro Linux
sudo pacman -S sdl2 sdl2_image sdl2_mixer sdl2_ttf

# Ubuntu
sudo apt install libsdl2-2.0* libsdl2-image* libsdl2-mixer* libsdl2-ttf*

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
