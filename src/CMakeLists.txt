project(sapphire)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O2")

set (EXECUTABLE_OUTPUT_PATH ${CMAKE_CURRENT_SOURCE_DIR}/../bin)

file(GLOB PROJECT_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/*.cc)

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


