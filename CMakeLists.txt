CMAKE_MINIMUM_REQUIRED( VERSION 2.8 )

PROJECT( IrradianceEnvMap )

#cmake .. -DCMAKE_BUILD_TYPE:STRING=Release
SET( CMAKE_CXX_FLAGS_RELEASE "-O2 -DNDEBUG -msse")
SET( CMAKE_CXX_FLAGS_DEBUG   "-O0 -g -Wall")

add_subdirectory(external)

FILE( GLOB_RECURSE SRC src/* )

INCLUDE_DIRECTORIES( src/ 
	external/ 
	external/glfw-3.1.2/include/GLFW/
	external/glew-1.13.0/include/
	)

FILE( GLOB GLSW external/glsw/*.c external/glsw/*.h )
ADD_LIBRARY( glsw ${GLSW} )

if(APPLE)
    # macでglfwを利用するには
    # cocoa.framework
    # OpenGL.framework
    # IOKit.framework
    # CoreVideo.framework
    # が必要になるのでリンクするために
    # これらのライブラリを検索してやる
    find_library( COCOA_LIBRARY Cocoa )
    find_library( OpenGL_LIBRARY OpenGL )
    find_library( IOKIT_LIBRARY IOKit )
    find_library( COREVIDEO_LIBRARY CoreVideo )
    SET( EXTRA_LIBS ${COCOA_LIBRARY} ${OpenGL_LIBRARY} ${IOKIT_LIBRARY} ${COREVIDEO_LIBRARY} )
ENDIF(APPLE)

SET(PLATFORM_LIBS freeimage m glfw GLEW_1130)

ADD_EXECUTAbLE( iem ${SRC} )

TARGET_LINK_LIBRARIES( iem ${PLATFORM_LIBS} glsw ${EXTRA_LIBS})
