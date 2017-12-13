CMAKE_MINIMUM_REQUIRED( VERSION 2.8 )

PROJECT( IrradianceEnvMap )

#cmake .. -DCMAKE_BUILD_TYPE:STRING=Release
SET( CMAKE_CXX_FLAGS_RELEASE "-O2 -DNDEBUG -msse")
SET( CMAKE_CXX_FLAGS_DEBUG   "-O0 -g -Wall")

FILE( GLOB_RECURSE SRC src/* )

INCLUDE_DIRECTORIES( src/ external/ )

FILE( GLOB GLSW external/glsw/*.c external/glsw/*.h )
ADD_LIBRARY( glsw ${GLSW} )

find_package(OpenGL REQUIRED)
include_directories(${OPENGL_INCLUDE_DIR})

find_package(GLUT REQUIRED glut)
include_directories(${GLUT_INCLUDE_DIR})
link_directories(${GLUT_LIBRARY_DIRS})

find_package(GLEW REQUIRED glew)
include_directories(${GLEW_INCLUDE_DIR})
link_directories(${GLEW_LIBRARY_DIRS})

SET( PLATFORM_LIBS freeimage m )

ADD_EXECUTAbLE( iem ${SRC} )

TARGET_LINK_LIBRARIES( iem ${PLATFORM_LIBS} glew glsw ${OPENGL_LIBRARIES} ${GLUT_LIBRARIES} ${GLEW_LIBRARIES})
