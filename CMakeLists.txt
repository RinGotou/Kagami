cmake_minimum_required(VERSION 3.8)

option(NOCUI "Disable the console user interface" OFF)
option(ENABLE_DEBUGGING "Enable debugging functionality" OFF)
option(DISABLE_SDLv"Disable SDL Stuff" OFF)

add_subdirectory(src)