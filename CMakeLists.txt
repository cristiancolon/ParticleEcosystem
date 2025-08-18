cmake_minimum_required(VERSION 3.10)
project(ParticleSim)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Find required packages
find_package(OpenGL REQUIRED)
find_package(glfw3 CONFIG REQUIRED)
find_package(glad CONFIG REQUIRED)

# Add executable
add_executable(ParticleSim
	main.cpp
	Renderer.cpp
	Color.cpp
	GPUParticle.h
)

# Link libraries
target_link_libraries(ParticleSim 
	OpenGL::GL
	glfw
	glad::glad
)

# Include directories
target_include_directories(ParticleSim PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})

# Set compiler flags
if(MSVC)
	target_compile_options(ParticleSim PRIVATE /W4)
else()
	target_compile_options(ParticleSim PRIVATE -Wall -Wextra -Wpedantic)
endif()