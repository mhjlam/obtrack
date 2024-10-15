# For Debug build, perform the following commands:
#   cmake ..
#   cmake build .
# For Release build:
#   cmake .. -DCMAKE_BUILD_TYPE=Release
#   cmake build . --config Release

cmake_minimum_required(VERSION 3.5)
include(CMakePrintHelpers)

# Set project name
project(obtrack)

# Set output directory relative to build directory
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

# Set executable name
add_executable(obtrack)

# C++ Standard
set_target_properties(obtrack PROPERTIES
    CXX_STANDARD 20
    CXX_STANDARD_REQUIRED ON
    CXX_EXTENSIONS OFF
)

# Compiler flags
target_compile_options(obtrack PRIVATE
    $<$<CXX_COMPILER_ID:Clang,GNU>:-Wall -Wextra -Wpedantic>
    $<$<CXX_COMPILER_ID:MSVC>:/W3 /WX>
)

# OpenGL
find_package(OpenGL REQUIRED)

# OpenCV
set(OpenCV_DIR ${CMAKE_CURRENT_SOURCE_DIR}/lib/opencv-4.10.0)
find_package(OpenCV REQUIRED)

# FreeGLUT
set(FreeGLUT_DIR ${CMAKE_CURRENT_SOURCE_DIR}/lib/freeglut-3.6.0)

# Source files
target_sources(obtrack PRIVATE
    src/Main.cpp src/Camera.cpp src/Histogram.cpp src/Line2f.cpp src/Renderer.cpp src/Tracker.cpp src/VoxelGrid.cpp
)

# Include directories
target_include_directories(obtrack PRIVATE
    ${FreeGLUT_DIR}/include
    ${OpenCV_INCLUDE_DIRS}
)

# Target link directories
target_link_directories(obtrack PRIVATE
    ${FreeGLUT_DIR}/lib)

# Target link libraries
set(OpenCV_LIBS_DEBUG)
set(v ${OpenCV_VERSION_MAJOR}${OpenCV_VERSION_MINOR}${OpenCV_VERSION_PATCH})

if (CMAKE_BUILD_TYPE STREQUAL "Release")
    target_link_libraries(obtrack PRIVATE
        freeglut_static
        ${OpenCV_LIBS}
    )
else()
    foreach(x ${OpenCV_LIBS})
        list(APPEND OpenCV_LIBS_DEBUG "${OpenCV_DIR}/x64/vc17/lib/${x}${v}d.lib")
    endforeach()
    
    target_link_libraries(obtrack PRIVATE
        freeglut_static
        ${OpenCV_LIBS_DEBUG})
endif()

# Copy OpenCV binaries (dlls) to output directory
file(GLOB OPENCV_DLLS "${OpenCV_DIR}/x64/vc17/bin/*.dll")
foreach(DLL IN LISTS OPENCV_DLLS)
    add_custom_command(TARGET obtrack POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different ${DLL} "$<TARGET_FILE_DIR:obtrack>")
endforeach()

# Copy FreeGLUT binaries (dlls) to output directory
add_custom_command(TARGET obtrack POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory "${FreeGLUT_DIR}/bin" "$<TARGET_FILE_DIR:obtrack>"
)

# Copy resource files to output directory
add_custom_command(TARGET obtrack POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory "${PROJECT_SOURCE_DIR}/data" "$<TARGET_FILE_DIR:obtrack>/data"
)
