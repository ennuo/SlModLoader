cmake_minimum_required(VERSION 3.25.2)
project(RemoveIntroCinematics VERSION 0.0.1)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY $<1:${CMAKE_BINARY_DIR}/bin> CACHE INTERNAL "")

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${PROJECT_SOURCE_DIR}/cmake")

find_file(WINDOWS_HEADER windows.h)
if (NOT WINDOWS_HEADER)
	message(FATAL_ERROR "Couldn't find windows.h!")
endif()

file(GLOB_RECURSE SRC_FILES "${PROJECT_SOURCE_DIR}/src/*.cpp")
file(GLOB_RECURSE HEADER_FILES "${PROJECT_SOURCE_DIR}/code/src/*.h")

add_library(${PROJECT_NAME} SHARED ${SRC_FILES} ${HEADER_FILES})

target_include_directories(${PROJECT_NAME} PRIVATE 
	"${PROJECT_SOURCE_DIR}/src"
	"${PROJECT_SOURCE_DIR}/../../code/include"
)

if (MSVC)
	add_definitions(-D_CRT_SECURE_NO_WARNINGS)
endif()