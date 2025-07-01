cmake_minimum_required(VERSION 3.25.2)
project(SlModLoader VERSION 0.0.1)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY $<1:${CMAKE_BINARY_DIR}/bin> CACHE INTERNAL "")
set(CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake" ${CMAKE_MODULE_PATH})

find_file(WINDOWS_HEADER windows.h)
if (NOT WINDOWS_HEADER)
	message(FATAL_ERROR "Couldn't find windows.h!")
endif()

file(GLOB_RECURSE SRC_FILES "${PROJECT_SOURCE_DIR}/code/src/*.cpp" "${PROJECT_SOURCE_DIR}/code/src/*.c")
file(GLOB_RECURSE HEADER_FILES "${PROJECT_SOURCE_DIR}/code/src/*.h" "${PROJECT_SOURCE_DIR}/code/src/*.hpp")

# add_executable(${CMAKE_PROJECT_NAME} ${SRC_FILES} ${HEADER_FILES})
add_library(${CMAKE_PROJECT_NAME} SHARED ${SRC_FILES} ${HEADER_FILES})

target_include_directories(${CMAKE_PROJECT_NAME} PRIVATE 
	"${PROJECT_SOURCE_DIR}/code/include"
	"${PROJECT_SOURCE_DIR}/code/src"
	"${PROJECT_SOURCE_DIR}/sdk"
	"${PROJECT_SOURCE_DIR}/sdk/fmt/include"
	"${PROJECT_SOURCE_DIR}/minhook/include"
)

target_precompile_headers(${CMAKE_PROJECT_NAME} PUBLIC 
	code/src/Native.hpp
	code/include/Sl/Core/Types.hpp	
)

if (MSVC)
	add_definitions(-D_CRT_SECURE_NO_WARNINGS)
endif()


set(BUILD_SHARED_LIBS OFF)

option(ZLIB_BUILD_EXAMPLES "Don't build Zlib examples" OFF)
set(ZLIB_BUILD_SHARED OFF)
set(SKIP_INSTALL_ALL ON)
add_subdirectory(sdk/zlib EXCLUDE_FROM_ALL)

add_subdirectory(sdk/minhook)
add_subdirectory(sdk/glm)


set(BUILD_TOOLS OFF)
set(BUILD_SAMPLE OFF)
set(BUILD_DX12 OFF)
set(BC_USE_OPENMP OFF)
set(ENABLE_LIBPNG_SUPPORT OFF)
add_subdirectory(sdk/DirectXTex)

target_link_libraries(${CMAKE_PROJECT_NAME} minhook glm DirectXTex zlibstatic)
set_target_properties(${CMAKE_PROJECT_NAME} PROPERTIES OUTPUT_NAME "dinput8")

set(INSTALL_PATH "C:/Program Files (x86)/Steam/steamapps/common/Sonic & All-Stars Racing Transformed")

add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
	COMMAND ${CMAKE_COMMAND} -E copy
	"${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/dinput8.dll"
	${INSTALL_PATH}/dinput8.dll)

add_subdirectory(plugins/RemoveIntroCinematics)