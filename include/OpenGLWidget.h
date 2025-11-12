#pragma once

#include <QOpenGLWidget>
#include <QOpenGLFunctions>
#include <QMatrix4x4>
#include <QVector3D>
#include <memory>
#include <vector>

class Model;

class OpenGLWidget : public QOpenGLWidget, protected QOpenGLFunctions {
    Q_OBJECT

public:
    explicit OpenGLWidget(QWidget *parent = nullptr);
    ~OpenGLWidget();
    
    void addModel(std::shared_ptr<Model> model);
    void removeModel(std::shared_ptr<Model> model);
    void clearModels();
    
    void setPseudoColorEnabled(bool enabled) { pseudoColorEnabled_ = enabled; update(); }
    void setCoordinateAxis(int axis) { coordinateAxis_ = axis; update(); }
    // 设置伪彩色坐标尺度（影响归一化范围：[-scale, scale]）
    
    void setSelectedModelIndex(int index) { selectedModelIndex_ = index; update(); }
    
    // 伪彩色色图模式：0=Rainbow, 1=Viridis, 2=Red-Blue
    void setColorMapMode(int mode) { colorMapMode_ = mode; update(); }
    int colorMapMode() const { return colorMapMode_; }
    
    // 计算当前所有模型在指定轴的最小/最大值，返回是否有效
    bool computeAxisRange(int axis, float& minV, float& maxV) const;
    
    void resetCamera();
    
protected:
    void initializeGL() override;
    void resizeGL(int w, int h) override;
    void paintGL() override;
    
    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;
    void wheelEvent(QWheelEvent *event) override;
    
private:
    void setupShaders();
    void setupLighting();
    void drawGrid();
    void drawAxes();
    void drawModels();
    void updateProjection();
    void computePseudoColor(float t, float& r, float& g, float& b) const;
    // 已弃用的 mapCoordToT 移除，采用帧内局部快速映射（见 drawModels）
    
    // 相机参数
    QVector3D cameraPosition_;
    QVector3D cameraTarget_;
    QVector3D cameraUp_;
    float cameraDistance_;
    float cameraYaw_;
    float cameraPitch_;
    
    // 投影矩阵
    QMatrix4x4 projectionMatrix_;
    QMatrix4x4 viewMatrix_;
    
    // 鼠标交互
    QPoint lastMousePos_;
    bool mousePressed_;
    Qt::MouseButton mouseButton_;
    
    // 模型数据
    std::vector<std::shared_ptr<Model>> models_;
    int selectedModelIndex_;
    
    // 可视化选项
    bool showGrid_;
    bool showAxes_;
    bool pseudoColorEnabled_;
    int coordinateAxis_; // 0=X, 1=Y, 2=Z
    int colorMapMode_; // 0=Rainbow, 1=Viridis, 2=Red-Blue
    
    // 单位换算：内部几何按米(m)存储，显示/伪彩色统一用厘米(cm)
    // 该因子用于把内部值（米）转换为厘米。
    float unitToCm_ = 100.0f;
    
    // OpenGL对象
    unsigned int gridVAO_, gridVBO_;
    unsigned int axesVAO_, axesVBO_;
};