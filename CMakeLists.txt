cmake_minimum_required (VERSION 3.5)

project (OpenGL)

# This is so YouCompleteMe can work
set (CMAKE_EXPORT_COMPILE_COMMANDS ON)

set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -std=c++14")
set (source_dir "${PROJECT_SOURCE_DIR}/src")
set (dep_dir "${PROJECT_SOURCE_DIR}/dep")
set (include_dir "${PROJECT_SOURCE_DIR}/include")

set (HEADER_FILES "${source_dir}/pso.h")
set (SOURCE_FILES)
set (ALL_DEPENDENCIES ${HEADER_FILES} ${SOURCE_FILES})
add_executable (runme "${source_dir}/main.cpp" ${ALL_DEPENDENCIES})
target_include_directories(runme PRIVATE ${include_dir})

#GLFW
add_subdirectory ("${dep_dir}/glfw-3.2.1")
target_link_libraries (runme glfw)

#GLEW
find_package (GLEW REQUIRED)
if (GLEW_FOUND)
	include_directories(${GLEW_INCLUDE_DIRS})
	target_link_libraries(runme ${GLEW_LIBRARIES})
endif()

#GLM
add_subdirectory("${dep_dir}/glm")
target_link_libraries(runme glm)

#OpenGL
find_package (OpenGL REQUIRED)
target_include_directories(runme PRIVATE ${OPENGL_INCLUDE_DIR})
target_link_libraries(runme ${OPENGL_gl_LIBRARY})
target_link_libraries(runme ${OPENGL_glu_LIBRARY})

#ASSIMP
find_package (ASSIMP REQUIRED)
include_directories(${ASSIMP_INCLUDE_DIRS})
target_link_libraries (runme ${ASSIMP_LIBRARIES})
