project(kagami)

set (CMAKE_CXX_STANDARD 17)
set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O2")

if(WIN32)
	add_definitions(-DWIN32)
endif()

if(MSVC)
	add_definitions(-DUNICODE -D_UNICODE)
elseif(MINGW)
	add_definitions(-municode)
endif()

if(DISABLE_SDL)
	add_definitions(-D_DISABLE_SDL_)
endif()

if(DEBUG)
  add_definitions(-D_DEBUG_)
endif()

file(GLOB PROJECT_SOURCES
	${CMAKE_CURRENT_SOURCE_DIR}/*.cc
)

file(GLOB LOG_LIB_SOURCES
	${CMAKE_CURRENT_SOURCE_DIR}/minatsuki.log/src/*.cc
)

if(NOT DEFINED DISABLE_SDL)
	file(GLOB DAWN_SOURCES
	${CMAKE_CURRENT_SOURCE_DIR}/dawn/src/*.cc
)
	add_executable(${PROJECT_NAME} ${PROJECT_SOURCES} ${LOG_LIB_SOURCES}${DAWN_SOURCES} )
endif()



add_executable(${PROJECT_NAME} ${PROJECT_SOURCES} ${LOG_LIB_SOURCES})
