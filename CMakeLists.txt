cmake_minimum_required(VERSION 3.8)
project(tw)

set (CMAKE_CXX_STANDARD 17)

set (CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})
set (CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})

# this function preppends a path to all files in a list
FUNCTION(PREPEND var prefix)
SET(listVar "")
   FOREACH(f ${ARGN})
   LIST(APPEND listVar "${prefix}/${f}")
   ENDFOREACH(f)
   SET(${var} "${listVar}" PARENT_SCOPE)
ENDFUNCTION(PREPEND)

find_package(OpenGL REQUIRED)
if(OPENGL_FOUND)
	include_directories(${OPENGL_INCLUDE_DIRS})
	link_libraries(${OPENGL_LIBRARIES})
endif()

add_subdirectory(libs/glad)
add_subdirectory(libs/SDL2)
add_subdirectory(libs/glm)
add_subdirectory(libs/imgui)
add_subdirectory(libs/stb)
add_subdirectory(libs/tl)
add_subdirectory(libs/tgl)
add_subdirectory(libs/cgltf)

set(SOURCES
    main.cpp
)
PREPEND(SOURCES "src/" ${SOURCES})

add_subdirectory(tests)

add_executable(tw
    ${SOURCES}
)

target_link_libraries(tw
    glad
    SDL2 SDL2main
    glm
    imgui
    stb
    tl
    tgl
    cgltf
)
