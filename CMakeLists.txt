cmake_minimum_required(VERSION 3.4...3.28 FATAL_ERROR)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# CREATE WORKSPACE
set(WORKSPACE_NAME "rt")
project(${WORKSPACE_NAME})

add_compile_definitions(RT_DIRECTORY=\"${PROJECT_SOURCE_DIR}/rt\")

set_property(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT ${WORKSPACE_NAME})

add_compile_definitions(ASSETS_DIRECTORY=\"${PROJECT_SOURCE_DIR}/${WORKSPACE_NAME}/assets\")

# IDENTIFY SYSTEM
if(CMAKE_SYSTEM_NAME STREQUAL "Darwin")
    message(STATUS "Building on macOS")
    add_compile_definitions(PLATFORM_MACOS)
    set(PLATFORM_MACOS TRUE)
elseif(CMAKE_SYSTEM_NAME STREQUAL "Linux")
    message(STATUS "Building on Linux")
    set(PLATFORM_LINUX TRUE)
    add_compile_definitions(PLATFORM_LINUX)
elseif(CMAKE_SYSTEM_NAME STREQUAL "Windows")
    message(STATUS "Building on Windows")
    set(PLATFORM_WINDOWS TRUE)
    add_compile_definitions(PLATFORM_WINDOWS)
else()
    message(STATUS "Building on an unknown system: ${CMAKE_SYSTEM_NAME}")
endif()

# STB
add_subdirectory("vendor/stb")
# STB
add_subdirectory("vendor/imgui")
# GLM
add_subdirectory("vendor/glm")
# GLAD
add_subdirectory("vendor/glad")
# GLFW
set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
add_subdirectory("vendor/glfw")
# OpenGL
find_package(OpenGL REQUIRED)

# Project Files
file(GLOB_RECURSE ALL_SOURCES
    "./${WORKSPACE_NAME}/src/*.cpp"
    "./${WORKSPACE_NAME}/src/*.hpp"
    "./${WORKSPACE_NAME}/src/*.h"
)
foreach (src ${ALL_SOURCES})
    get_filename_component(dir ${src} DIRECTORY)
	string(REPLACE "${PROJECT_SOURCE_DIR}/./" "" desired_portion ${dir})
    source_group(${desired_portion} FILES ${src})
endforeach()
add_executable(${WORKSPACE_NAME}
    ${ALL_SOURCES}
)

# Libraries
target_link_libraries(${WORKSPACE_NAME} glfw)
target_link_libraries(${WORKSPACE_NAME} glad)
target_link_libraries(${WORKSPACE_NAME} glm)
target_link_libraries(${WORKSPACE_NAME} stb)
target_link_libraries(${WORKSPACE_NAME} imgui)
target_link_libraries(${WORKSPACE_NAME} OpenGL::GL)

# Headers
target_include_directories(${WORKSPACE_NAME} PUBLIC "vendor/glfw/include")
target_include_directories(${WORKSPACE_NAME} PUBLIC "vendor/glad/include")
target_include_directories(${WORKSPACE_NAME} PUBLIC "vendor/imgui")
target_include_directories(${WORKSPACE_NAME} PUBLIC "vendor/imgui/backends")
target_include_directories(${WORKSPACE_NAME} PUBLIC "vendor/stb/include")
