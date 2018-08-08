cmake_minimum_required(VERSION 3.8)

# Add options
option(NOCUI "Disable the console user interface" OFF)
option(ENABLE_DEBUGGING "Enable debugging functionality" OFF)

# We want shared libraries (and modules) without any prefix in name
set(CMAKE_SHARED_LIBRARY_PREFIX "")
set(CMAKE_SHARED_MODULE_PREFIX "")

# Add subdirectories
add_subdirectory(src)
add_subdirectory(plugin)