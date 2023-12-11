# Set the minimum required version of cmake for this project
cmake_minimum_required (VERSION 3.12)

# Create a project called 'HW2b'
project(Assignment2B, C CXX)
set(CMAKE_CXX_STANDARD 20)

set(TARGET_NAME HW2b)

# Find OpenGL, set link library names and include paths
find_package(OpenGL REQUIRED)
set(OPENGL_LIBRARIES ${OPENGL_gl_LIBRARY} ${OPENGL_glu_LIBRARY})
set(OPENGL_INCLUDE_DIRS ${OPENGL_INCLUDE_DIR})
include_directories(${OPENGL_INCLUDE_DIRS})

# Disable building some of the extra things GLFW has (examples, tests, docs)
set(GLFW_BUILD_EXAMPLES  OFF CACHE BOOL " " FORCE)
set(GLFW_BUILD_TESTS     OFF CACHE BOOL " " FORCE)
set(GLFW_BUILD_DOCS      OFF CACHE BOOL " " FORCE)

add_definitions( -DMY_SRC_DIR="${CMAKE_CURRENT_SOURCE_DIR}/src/" )
add_definitions( -DMY_DATA_DIR="${CMAKE_CURRENT_SOURCE_DIR}/data/" )

# Run cmake on the CMakeLists.txt file found inside of the GLFW directory
add_subdirectory(ext/glfw)

# Make a list of all the source files
set(SOURCES
    src/main.cpp
    ext/glad/src/glad.c
)

# Make a list of all the header files
set(INCLUDES
    src/shader.hpp
    src/trimesh.hpp
    src/core/Vector3D.hpp
    src/core/Matrix.hpp
)

# Make a list of all of the directories to look in when doing #include "whatever.h"
set(INCLUDE_DIRS
    ext/
    ext/glfw/include
    ext/glad/include
)

# Make a list of the libraries
set(LIBS
    glfw
    ${OPENGL_LIBRARIES}
)

# Define what we are trying to produce here (an executable),
# and what items are needed to create it (the header and source files)
add_executable(${TARGET_NAME} ${SOURCES} ${INCLUDES})

# Tell cmake which directories to look in when you #include a file
# Equivalent to the "-I" option for g++
include_directories(${INCLUDE_DIRS})

# Tell cmake which libraries to link to
# Equivalent to the "-l" option for g++
target_link_libraries(${TARGET_NAME} PRIVATE ${LIBS})

# For Visual Studio only
if (MSVC)
    # Do a parallel compilation of this project
    target_compile_options(${PROJECT_NAME} PRIVATE "/MP")
    # Have this project be the default startup project (the one to build/run when hitting F5)
    set_property(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT ${PROJECT_NAME})
endif()