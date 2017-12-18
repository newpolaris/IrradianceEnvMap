# CMake entry point
cmake_minimum_required (VERSION 3.0)
project (IrradianceEnvMap)

find_package(OpenGL REQUIRED)
 
# Compile external dependencies 
add_subdirectory (external)

# On Visual 2005 and above, this module can set the debug working directory
cmake_policy(SET CMP0026 OLD)
list(APPEND CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/external/rpavlik-cmake-modules-fe2273")
include(CreateLaunchers)
include(MSVCMultipleProcessCompile) # /MP

if(INCLUDE_DISTRIB)
	add_subdirectory(distrib)
endif(INCLUDE_DISTRIB)



include_directories(
	src/
	external/
	external/FreeImage/src/
	external/AntTweakBar-1.16/include/
	external/glfw-3.1.2/include/GLFW/
	external/glm-0.9.7.1/
	external/glew-1.13.0/include/
	external/assimp-3.0.1270/include/
	external/bullet-2.81-rev2613/src/
	.
)

set(ALL_LIBS
	${OPENGL_LIBRARY}
	glfw
	GLEW_1130
	FreeImage
)

add_definitions(
	-DTW_STATIC
	-DTW_NO_LIB_PRAGMA
	-DTW_NO_DIRECT3D
	-DGLEW_STATIC
	-D_CRT_SECURE_NO_WARNINGS
)

file( GLOB GLSW external/glsw/*.c external/glsw/*.h )
add_library( glsw ${GLSW} )

file( GLOB_RECURSE SRC src/* )

add_executable(iem ${SRC})

target_link_libraries(iem glsw ${ALL_LIBS})

# Xcode and Visual working directories
set_target_properties(iem PROPERTIES XCODE_ATTRIBUTE_CONFIGURATION_BUILD_DIR "${CMAKE_CURRENT_SOURCE_DIR}/")
create_target_launcher(iem WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/")
