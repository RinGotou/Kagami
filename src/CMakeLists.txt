project(kagami)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O2")

set (EXECUTABLE_OUTPUT_PATH ${CMAKE_CURRENT_SOURCE_DIR}/../bin)

file(GLOB PROJECT_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/*.cc)
file(GLOB LOG_LIB_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/minatsuki.log/src/*.cc)


file(GLOB DAWN_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/dawn/src/*.cc)
add_executable(${PROJECT_NAME} ${PROJECT_SOURCES} ${LOG_LIB_SOURCES} ${DAWN_SOURCES})

list(APPEND CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/sdl2-cmake-modules)

find_package(SDL2 REQUIRED)
target_include_directories(${PROJECT_NAME} PRIVATE ${SDL2_INCLUDE_DIRS})
target_link_libraries(${PROJECT_NAME} ${SDL2_LIBRARIES})

find_package(SDL2_image REQUIRED)
target_include_directories(${PROJECT_NAME} PRIVATE ${SDL2_IMAGE_INCLUDE_DIRS})
target_link_libraries(${PROJECT_NAME} ${SDL2_IMAGE_LIBRARIES})
  
find_package(SDL2_ttf REQUIRED)
target_include_directories(${PROJECT_NAME} PRIVATE ${SDL2_TTF_INCLUDE_DIRS})
target_link_libraries(${PROJECT_NAME} ${SDL2_TTF_LIBRARIES})
  
find_package(SDL2_mixer REQUIRED)
target_include_directories(${PROJECT_NAME} PRIVATE ${SDL2_MIXER_INCLUDE_DIRS})
target_link_libraries(${PROJECT_NAME} ${SDL2_MIXER_LIBRARIES})

if(WIN32)
  add_definitions(-DWIN32)
else()
  target_link_libraries(${PROJECT_NAME} ${CMAKE_DL_LIBS})
  if(CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
    target_link_libraries(${PROJECT_NAME} c++fs)
  elseif(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    target_link_libraries(${PROJECT_NAME} stdc++fs)
  endif()
endif()

if(MSVC)
  add_definitions(-DUNICODE -D_UNICODE)
elseif(MINGW)
  add_definitions(-municode)
endif()


