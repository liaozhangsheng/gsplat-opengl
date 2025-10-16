cmake_minimum_required(VERSION 3.16)
project(gsplat_viewer)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(glfw3 REQUIRED)
find_package(glm REQUIRED)
find_package(OpenGL REQUIRED)

set(GLAD_DIR ${CMAKE_SOURCE_DIR}/3rdparty/glad)
add_library(glad STATIC ${GLAD_DIR}/src/glad.c)
target_include_directories(glad PUBLIC ${GLAD_DIR}/include)

set(TINYPLY_DIR ${CMAKE_SOURCE_DIR}/3rdparty/tinyply)
add_subdirectory(${TINYPLY_DIR})

add_executable(${PROJECT_NAME})

target_sources(${PROJECT_NAME} PRIVATE
    src/main.cpp
    src/Renderer.cpp
    src/Camera.cpp
    src/PLYLoader.cpp
    src/GaussianData.cpp
    src/OrbitControls.cpp
    src/SplatSort.cpp
)

target_include_directories(${PROJECT_NAME} PRIVATE
    ${CMAKE_SOURCE_DIR}/include
    ${TINYPLY_DIR}/source
)

target_link_libraries(${PROJECT_NAME}
    glfw
    glad
    glm::glm
    tinyply
    OpenGL::GL
)

# Copy shaders to build directory
file(COPY ${CMAKE_SOURCE_DIR}/shaders DESTINATION ${CMAKE_BINARY_DIR})
