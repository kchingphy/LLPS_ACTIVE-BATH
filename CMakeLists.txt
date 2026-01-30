# Define a variable
set(PROJ_NAME "LLPS_analysis")	#set project name

cmake_minimum_required(VERSION 3.20)
project(${PROJ_NAME} LANGUAGES CXX)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Require C++20
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# List all source files
# set(SOURCES
    # src/main.cpp
    # src/add.cpp
    # src/multiply.cpp
# )

#Automatic detect source file, if any changes it will reconfigure CMake
file(GLOB SOURCES CONFIGURE_DEPENDS "${CMAKE_SOURCE_DIR}/src/*.cpp")
if (NOT SOURCES)
  message(FATAL_ERROR "No .cpp files found in ${CMAKE_SOURCE_DIR}/src")
endif()

# Create executable
add_executable(${PROJ_NAME} ${SOURCES})

# Tell compiler where to find headers
target_include_directories(${PROJ_NAME} PRIVATE ${CMAKE_SOURCE_DIR}/include)

# Use vcpkg's Boost.Math config package (no FindBoost, no CMP0167 warning)
#find_package(boost_math CONFIG REQUIRED)

# NEW: link the imported target (header-only, so this mainly adds include dirs)
#target_link_libraries(${PROJ_NAME} PRIVATE Boost::math)
