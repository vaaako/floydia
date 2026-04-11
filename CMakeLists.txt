cmake_minimum_required(VERSION 3.10)
project(floydia VERSION 1.0)

# Set C++ standard
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

option(DEBUG "BUild with debug flags" OFF)

# Include directories
include_directories(
	include
	include/external
)

# Collect all source files
file(GLOB_RECURSE SOURCES "src/*.cpp" "src/gl.c")

# Create executable
add_executable(${PROJECT_NAME} ${SOURCES})

target_link_libraries(${PROJECT_NAME} PUBLIC
	opengl32
	winmm
	imm32
	gdi32
)

# Set build types
#if(CMAKE_BUILD_TYPE STREQUAL "Debug")
#    target_compile_options(${PROJECT_NAME} PRIVATE /g /O0)
#elseif(CMAKE_BUILD_TYPE STREQUAL "Release")
#    target_compile_options(${PROJECT_NAME} PRIVATE -O2)
#endif()
target_compile_options(${PROJECT_NAME} PRIVATE
	/W3                  # Warning level 3
	/permissive-         # Standard conformance
	$<$<CONFIG:Debug>:
		/Od /Zi /MDd     # No optimization, Debug info, Debug Runtime
	>
	$<$<CONFIG:Release>:
	/O2 /DNDEBUG /MD /LTCG # Max optimization, Release Runtime
	>
)
