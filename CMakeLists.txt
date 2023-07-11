cmake_minimum_required(VERSION 3.20)
project(VulkanEngine VERSION 1.0 LANGUAGES CXX)
include(ExternalProject)

#--------------------------------------------------------------------#
#                           Find libraries                           #

find_package(Vulkan REQUIRED COMPONENTS glslc)
#--------------------------------------------------------------------#

#--------------------------------------------------------------------#
#                          Get source files                          #

file(GLOB_RECURSE SOURCE_FILES ${CMAKE_CURRENT_SOURCE_DIR}/src/*.cc)
#--------------------------------------------------------------------#

add_executable(${PROJECT_NAME} ${SOURCE_FILES})

#--------------------------------------------------------------------#
#                     Check Internet Connection                      #

execute_process(
    COMMAND ping www.google.com -c 1
    ERROR_QUIET
    RESULT_VARIABLE NO_CONNECTION
)

if(NOT NO_CONNECTION EQUAL 0)
    set(OFLINE_BUILD ON)
else()
    set(OFLINE_BUILD OFF)
endif()
#--------------------------------------------------------------------#

#--------------------------------------------------------------------#
#                         External Projects                          #

if(OFLINE_BUILD OR EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/extern)
    set_property(GLOBAL PROPERTY EP_UPDATE_DISCONNECTED ON)
endif()

ExternalProject_Add(GLFW
    GIT_REPOSITORY https://github.com/glfw/glfw.git
    CMAKE_ARGS
        -DGLFW_BUILD_EXAMPLES=OFF
        -DGLFW_BUILD_TESTS=OFF
        -DGLFW_BUILD_DOCS=OFF
        -DCMAKE_BUILD_TYPE=release
        -DCMAKE_C_COMPILER=${CMAKE_C_COMPILER}
        -DCMAKE_INSTALL_PREFIX:PATH=${CMAKE_CURRENT_SOURCE_DIR}/extern/glfw
)

ExternalProject_Add(GLM
    GIT_REPOSITORY https://github.com/g-truc/glm.git
    CMAKE_ARGS
        -DBUILD_TESTING=OFF
        -DCMAKE_BUILD_TYPE=release
        -DCMAKE_CXX_COMPILER=${CMAKE_CXX_COMPILER}
        -DCMAKE_INSTALL_PREFIX:PATH=${CMAKE_CURRENT_SOURCE_DIR}/extern/glm
)

ExternalProject_Add(TINYOBJLOADER
    GIT_REPOSITORY https://github.com/tinyobjloader/tinyobjloader.git
    GIT_TAG release
    CMAKE_ARGS
        -DTINYOBJLOADER_BUILD_TEST_LOADER=OFF
        -DCMAKE_BUILD_TYPE=release
        -DCMAKE_CXX_COMPILER=${CMAKE_CXX_COMPILER}
        -DCMAKE_INSTALL_PREFIX:PATH=${CMAKE_CURRENT_SOURCE_DIR}/extern/tinyObjLoader
)
#--------------------------------------------------------------------#

#--------------------------------------------------------------------#
#                        Compile shader code                         #

find_program(GLSLC_EXECUTABLE NAMES glslc HINTS Vulkan::glslc)

# get all .vert and .frag files in shaders directory
file(GLOB_RECURSE GLSL_SOURCE_FILES
    "${PROJECT_SOURCE_DIR}/shaders/*.frag"
    "${PROJECT_SOURCE_DIR}/shaders/*.vert"
)

foreach(GLSL ${GLSL_SOURCE_FILES})
    get_filename_component(FILE_NAME ${GLSL} NAME)
    set(SPIRV "${PROJECT_SOURCE_DIR}/shaders/${FILE_NAME}.spv")
    add_custom_command(
        OUTPUT ${SPIRV}
        COMMAND ${GLSLC_EXECUTABLE} -O ${GLSL} -o ${SPIRV}
        DEPENDS ${GLSL}
    )
    list(APPEND SPIRV_BINARY_FILES ${SPIRV})
endforeach(GLSL)

add_custom_target(
    Shaders
    DEPENDS ${SPIRV_BINARY_FILES}
)
#--------------------------------------------------------------------#

#--------------------------------------------------------------------#
#                            Dependencies                            #

add_dependencies(${PROJECT_NAME} Shaders GLFW GLM TINYOBJLOADER)
#--------------------------------------------------------------------#

#--------------------------------------------------------------------#
#                           Link libraries                           #

target_link_libraries(${PROJECT_NAME} PUBLIC
    ${Vulkan_LIBRARIES}
    ${CMAKE_CURRENT_SOURCE_DIR}/extern/glfw/lib/libglfw3.a
    ${CMAKE_CURRENT_SOURCE_DIR}/extern/tinyObjLoader/lib/libtinyobjloader.a
)
#--------------------------------------------------------------------#

#--------------------------------------------------------------------#
#                        Include directories                         #

target_include_directories(${PROJECT_NAME} PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/inc
    ${Vulkan_INCLUDE_DIRS}
    ${CMAKE_CURRENT_SOURCE_DIR}/extern/glfw/include
    ${CMAKE_CURRENT_SOURCE_DIR}/extern/glm/include
    ${CMAKE_CURRENT_SOURCE_DIR}/extern/tinyObjLoader/include
)
#--------------------------------------------------------------------#

#--------------------------------------------------------------------#
#                           Set properties                           #

set_target_properties(${PROJECT_NAME} PROPERTIES
    # Specify directories
    ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/lib"
    LIBRARY_OUTPUT_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/lib"
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/bin"

    # Set C++ slandered
    CXX_STANDARD 20
    CXX_STANDARD_REQUIRED ON

    OUTPUT_NAME ${PROJECT_NAME}
)
#--------------------------------------------------------------------#

#--------------------------------------------------------------------#
#                              Use mold                              #

if(NOT WIN32)
    target_link_options(${PROJECT_NAME} PUBLIC -fuse-ld=mold)
endif()
#--------------------------------------------------------------------#

#--------------------------------------------------------------------#
#                               Debug                                #

SET(CMAKE_BUILD_TYPE Debug)

# For memory
# SET(CMAKE_CXX_FLAGS_DEBUG "-gfull -ggdb3 -O0 -pedantic-errors -Weverything -Wno-c++98-compat -Wno-c++98-compat-pedantic -Wno-padded -Wno-documentation -Wno-documentation-unknown-command -fsanitize=memory -fno-omit-frame-pointer -fno-optimize-sibling-calls -fsanitize-memory-track-origins=2")

# For address
SET(CMAKE_CXX_FLAGS_DEBUG "-gfull -ggdb3 -O0 -pedantic-errors -Weverything -Wno-c++98-compat -Wno-c++98-compat-pedantic -Wno-padded -Wno-documentation -Wno-documentation-unknown-command -fsanitize=address -fno-omit-frame-pointer -fno-optimize-sibling-calls")
#-Werror Treat warnings as errors
#--------------------------------------------------------------------#

#--------------------------------------------------------------------#
#                              Release                               #

# SET(CMAKE_BUILD_TYPE Release)
SET(CMAKE_CXX_FLAGS_RELEASE "-O3 -DNDEBUG")
#--------------------------------------------------------------------#
