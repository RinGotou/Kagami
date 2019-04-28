cmake_minimum_required(VERSION 3.5)
option(DISABLE_SDL "Disable SDL2 library for non-graphical environment" OFF)
option(DEBUG "Enable Debugging Message and Feature" OFF)
add_subdirectory(src)
