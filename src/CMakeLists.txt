project(Kagami)

if(WIN32)
	add_definitions(-DWIN32)
endif()

if(MSVC)
	add_definitions(-DUNICODE -D_UNICODE)
elseif(MINGW)
	add_definitions(-municode)
endif()

if(NOCUI)
	add_definitions(-D_NO_CUI_)
endif()

if(DISABLE_SDL)
	add_definitions(-D_DISABLE_SDL_)
endif()

if(ENABLE_DEBUGGING)
	add_definitions(-D_ENABLE_DEBUGGING_)
endif()

file(GLOB PROJECT_SOURCES
	${CMAKE_CURRENT_SOURCE_DIR}/*.cpp
)

#if(WIN32)
	#add_executable(${PROJECT_NAME} WIN32 ${PROJECT_SOURCES})
#else()
	add_executable(${PROJECT_NAME} ${PROJECT_SOURCES})
#endif()