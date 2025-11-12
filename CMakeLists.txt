cmake_minimum_required(VERSION 3.16)
project(3DDataVisualization)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Enable Qt MOC, UIC, RCC
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

# Find required packages
find_package(Qt6 REQUIRED COMPONENTS Core Widgets OpenGL OpenGLWidgets)
find_package(OpenGL REQUIRED)

# Include directories
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/src)

# Source files
set(SOURCES
    src/main.cpp
    src/MainWindow.cpp
    src/Model.cpp
    src/PointCloud.cpp
    src/Mesh.cpp
    src/OpenGLWidget.cpp
    src/ModelAnalyzer.cpp
    src/ColorMapper.cpp
    src/FileImporter.cpp
    src/TransformTool.cpp
)

# Header files
set(HEADERS
    include/MainWindow.h
    include/Model.h
    include/PointCloud.h
    include/Mesh.h
    include/OpenGLWidget.h
    include/ModelAnalyzer.h
    include/ColorMapper.h
    include/FileImporter.h
    include/TransformTool.h
    include/Vertex.h
    include/AABB.h
)

# Create executable
add_executable(${PROJECT_NAME} ${SOURCES} ${HEADERS})

# Link libraries
target_link_libraries(${PROJECT_NAME} 
    Qt6::Core 
    Qt6::Widgets 
    Qt6::OpenGL 
    Qt6::OpenGLWidgets
    ${OPENGL_LIBRARIES}
)