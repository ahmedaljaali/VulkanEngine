cmake_minimum_required(VERSION 3.20)
project(VulkanEngine VERSION 1.0)

#--------------------------------------------------------------------#
#                          Get source files                          #


file(GLOB_RECURSE SOURCE_FILES ${CMAKE_CURRENT_SOURCE_DIR}/src/*.cc)
#--------------------------------------------------------------------#

add_executable(${PROJECT_NAME} ${SOURCE_FILES})

#--------------------------------------------------------------------#
#                           Find libraries                           #


find_package(Vulkan REQUIRED)
find_package(glfw3 REQUIRED)
find_package(glm REQUIRED)
#--------------------------------------------------------------------#

#--------------------------------------------------------------------#
#                           Link libraries                           #


target_link_libraries(${PROJECT_NAME} PUBLIC
    ${Vulkan_LIBRARIES}
    glfw
)

#--------------------------------------------------------------------#

#--------------------------------------------------------------------#
#                        Include directories                         #


target_include_directories(${PROJECT_NAME} PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/inc
    ${VULKAN_INCLUDE_DIRS}
    ${GLM_INCLUDE_DIRS}
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

    OUTPUT_NAME "VulkanEngine"
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


SET(CMAKE_BUILD_TYPE debug)
SET(CMAKE_CXX_FLAGS_DEBUG "-gfull -ggdb3 -O0 -Weverything -Wno-c++98-compat -Wno-c++98-compat-pedantic -Wno-padded")
#-Werror Treat warnings as errors
#--------------------------------------------------------------------#

#--------------------------------------------------------------------#
#                              Release                               #


# SET(CMAKE_BUILD_TYPE release)
SET(CMAKE_CXX_FLAGS_RELEASE "-O3")
#--------------------------------------------------------------------#
