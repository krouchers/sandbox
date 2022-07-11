include(./.env.cmake RESULT_VARIABLE LOCAL_ENV)
cmake_minimum_required(VERSION 3.11.0)

if(${CMAKE_GENERATOR} STREQUAL "MinGW Makefiles")
    set(CMAKE_CXX_COMPILER ${MINGW_PATH}/bin/g++.exe)
endif()

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++2a -Werror -Wall -Wpedantic -Wextra -Wshadow")

option(USE_DEBUG "Enabling debug mod" OFF)
if(USE_DEBUG)
    add_definitions(-DDEBUG)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g") 
endif()
project(window)
add_definitions(-D_CRT_NONSTDC_NO_DEPRECATE -D_CRT_SECURE_NO_DEPRECATE -D_CRT_SECURE_NO_WARNINGS)

file(GLOB_RECURSE SOURCES
${PROJECT_SOURCE_DIR}/source/*.cpp
${PROJECT_SOURCE_DIR}/source/*.h
)
add_subdirectory(${PROJECT_SOURCE_DIR}/3dparty/glfw ${GLFW_PATH}/libs)

file(GLOB IMGUI_SOURCES 
"${IMGUI_DIR}/backends/imgui_impl_glfw.cpp"
"${IMGUI_DIR}/backends/imgui_impl_vulkan.cpp"
"${IMGUI_DIR}/imgui.cpp"
"${IMGUI_DIR}/imgui_draw.cpp"
"${IMGUI_DIR}/imgui_demo.cpp"
"${IMGUI_DIR}/imgui_tables.cpp"
"${IMGUI_DIR}/imgui_widgets.cpp"
)


file(GLOB_RECURSE IMGUI_HEADERS
"${IMGUI_DIR}/*.h")

set(SOURCES ${SOURCES} ${IMGUI_SOURCES} ${IMGUI_HEADERS})

add_executable(window ${SOURCES})

set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)

find_library(
    PATH_TO_GLFW_LIBRARY
    glfw
    PATHS ${GLFW_PATH}/libs 
    NO_CACHE
)
target_link_directories(window PRIVATE 
${PATH_TO_GLFW_LIBRARY}
${VULKAN_PATH}/lib)

target_link_libraries(window PRIVATE glfw vulkan-1) 

target_include_directories(window PRIVATE 
    ${GLFW_PATH}/include 
    ${VULKAN_PATH}/Include
    ${CMAKE_CURRENT_SOURCE_DIR}/source
    ${CMAKE_CURRENT_SOURCE_DIR}/source/imgui
    ${GLM_PATH}
    ${DIR_3DPARTY}/imgui
    ${DIR_3DPARTY}/imgui/backends
    ${DIR_3DPARTY}
    ${STB_PATH}
    ${IMGUI_DIR}/backends
    ${IMGUI_DIR}
    ${TINY_OJB_LOADER_DIR}
    )

find_program(
    GLSL_VALIDATOR glslc.exe 
    HINTS
    ${VULKAN_PATH}/Bin
    REQUIRED
)


file(GLOB_RECURSE SHADER_FILES
"${PROJECT_SOURCE_DIR}/*/shaders/*.vert"
"${PROJECT_SOURCE_DIR}/*/shaders/*.frag"
)

foreach(GLSL ${SHADER_FILES})
    cmake_path(GET GLSL FILENAME FILE_NAME)
    set(SPIRV "${PROJECT_SOURCE_DIR}/source/shaders/${FILE_NAME}.spv")

    add_custom_command(
        OUTPUT ${SPIRV}
        COMMAND ${GLSL_VALIDATOR} ${GLSL} -o ${SPIRV}
        DEPENDS ${GLSL}
        )
        list(APPEND SPIRV_BINARY_FILES ${SPIRV})
endforeach(GLSL)


add_custom_target(
    Shaders
    DEPENDS ${SPIRV_BINARY_FILES}
)

add_dependencies(window Shaders)



