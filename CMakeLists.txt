cmake_minimum_required(VERSION 3.23)
project(quaternion)

set(CMAKE_CXX_STANDARD 17)

# NOTE: Adapt theses flags depending on your os and configuration.
set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS}")

# Add executable
add_executable(quaternion main.cpp library.cpp tiny_obj_loader.h)

target_link_libraries(quaternion glew32)
target_link_libraries(quaternion glfw3)
target_link_libraries(quaternion opengl32)
