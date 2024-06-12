cmake_minimum_required(VERSION 3.27)
project(quaternion)

set(CMAKE_CXX_STANDARD 17)

# NOTE: Adapt theses flags depending on your os and configuration.
set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -framework OpenGL")

# NOTE: Change these paths to match current glew/glfw/glm libs installation location. 
include_directories(/opt/homebrew/opt/glew/include)
link_directories(/opt/homebrew/opt/glew/lib)

include_directories(/opt/homebrew/opt/glfw/include)
link_directories(/opt/homebrew/opt/glfw/lib)

include_directories(/opt/homebrew/opt/glm/include)
link_directories(/opt/homebrew/opt/glm/lib)

# Find Assimp package
find_package(assimp REQUIRED)

# Add executable
add_executable(quaternion main.cpp library.cpp)

# Link libraries
target_link_libraries(quaternion glew)
target_link_libraries(quaternion glfw)
target_link_libraries(quaternion glm)
target_link_libraries(quaternion assimp::assimp)
