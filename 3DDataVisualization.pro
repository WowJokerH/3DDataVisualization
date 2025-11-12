QT += core gui widgets opengl openglwidgets

CONFIG += c++17

TARGET = 3DDataVisualization
TEMPLATE = app

# 源文件
SOURCES += \
    src/main.cpp \
    src/MainWindow.cpp \
    src/Model.cpp \
    src/PointCloud.cpp \
    src/Mesh.cpp \
    src/OpenGLWidget.cpp \
    src/ModelAnalyzer.cpp \
    src/ColorMapper.cpp \
    src/FileImporter.cpp \
    src/TransformTool.cpp

# 头文件
HEADERS += \
    include/MainWindow.h \
    include/Model.h \
    include/PointCloud.h \
    include/Mesh.h \
    include/OpenGLWidget.h \
    include/Vertex.h \
    include/AABB.h \
    include/ModelAnalyzer.h \
    include/ColorMapper.h \
    include/FileImporter.h \
    include/TransformTool.h

# OpenGL库
LIBS += -lopengl32

# 包含路径
INCLUDEPATH += include