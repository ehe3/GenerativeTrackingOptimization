cmake_minimum_required (VERSION 3.5)

project (OpenGL)

# This is so YouCompleteMe can work
set (CMAKE_EXPORT_COMPILE_COMMANDS ON)

set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -std=c++14")
set (source_dir "${PROJECT_SOURCE_DIR}/src")
set (dep_dir "${PROJECT_SOURCE_DIR}/dep")
set (include_dir "${PROJECT_SOURCE_DIR}/include")

set (HEADER_FILES "${include_dir}/objloader.hpp")
set (SOURCE_FILES "${include_dir}/objloader.cpp")
set (ALL_DEPENDENCIES ${HEADER_FILES} ${SOURCE_FILES})
add_executable (out "${source_dir}/Application.cpp" ${ALL_DEPENDENCIES})
target_include_directories(out PRIVATE ${include_dir})

#GLFW
add_subdirectory ("${dep_dir}/glfw-3.2.1")
target_link_libraries (out glfw)

#GLEW
find_package (GLEW REQUIRED)
if (GLEW_FOUND)
	include_directories(${GLEW_INCLUDE_DIRS})
	target_link_libraries(out ${GLEW_LIBRARIES})
endif()

#GLM
add_subdirectory("${dep_dir}/glm")
target_link_libraries(out glm)

#OpenGL
find_package (OpenGL REQUIRED)
target_include_directories(out PRIVATE ${OPENGL_INCLUDE_DIR})
target_link_libraries(out ${OPENGL_gl_LIBRARY})
target_link_libraries(out ${OPENGL_glu_LIBRARY})

