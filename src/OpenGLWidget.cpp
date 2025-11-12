#include "OpenGLWidget.h"
#include "Model.h"
#include "AABB.h"
#include <QMouseEvent>
#include <QWheelEvent>
#include <QPainter>
#include <cmath>
#include <limits>
#include <algorithm> // 用于点云内部排序排名

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

OpenGLWidget::OpenGLWidget(QWidget *parent)
        : QOpenGLWidget(parent), cameraDistance_(10.0f), cameraYaw_(0.0f), 
            cameraPitch_(0.0f), mousePressed_(false), showGrid_(true), 
            showAxes_(true), pseudoColorEnabled_(false), coordinateAxis_(0),
            colorMapMode_(0), // 默认 Rainbow
            selectedModelIndex_(-1) {
    
    cameraPosition_ = QVector3D(0, 0, 10);
    cameraTarget_ = QVector3D(0, 0, 0);
    cameraUp_ = QVector3D(0, 1, 0);
}

OpenGLWidget::~OpenGLWidget() {
    makeCurrent();
    // 清理OpenGL资源
    doneCurrent();
}

void OpenGLWidget::initializeGL() {
    initializeOpenGLFunctions();
    
    // 设置背景色
    glClearColor(0.2f, 0.2f, 0.2f, 1.0f);
    
    // 启用深度测试
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_MULTISAMPLE);
    
    // 设置光照
    setupLighting();
    
    // 创建网格和坐标轴
    setupShaders();
}

void OpenGLWidget::resizeGL(int w, int h) {
    glViewport(0, 0, w, h);
    updateProjection();
}

void OpenGLWidget::paintGL() {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    
    // 设置模型视图矩阵
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    
    // 手动实现 gluLookAt (替代已弃用的函数)
    QVector3D f = (cameraTarget_ - cameraPosition_).normalized();
    QVector3D s = QVector3D::crossProduct(f, cameraUp_).normalized();
    QVector3D u = QVector3D::crossProduct(s, f);
    
    float m[16] = {
        s.x(), u.x(), -f.x(), 0,
        s.y(), u.y(), -f.y(), 0,
        s.z(), u.z(), -f.z(), 0,
        0, 0, 0, 1
    };
    
    glMultMatrixf(m);
    glTranslatef(-cameraPosition_.x(), -cameraPosition_.y(), -cameraPosition_.z());
    
    // 绘制网格
    if (showGrid_) {
        drawGrid();
    }
    
    // 绘制坐标轴
    if (showAxes_) {
        drawAxes();
    }
    
    // 绘制模型
    drawModels();

    // （已移除调试文本覆盖层，以避免自动提示干扰渲染与终端输出）
}

void OpenGLWidget::mousePressEvent(QMouseEvent *event) {
    lastMousePos_ = event->pos();
    mousePressed_ = true;
    mouseButton_ = event->button();
}

void OpenGLWidget::mouseMoveEvent(QMouseEvent *event) {
    if (!mousePressed_) return;
    
    QPoint delta = event->pos() - lastMousePos_;
    
    if (mouseButton_ == Qt::LeftButton) {
        // 旋转相机
        cameraYaw_ += delta.x() * 0.5f;
        cameraPitch_ += delta.y() * 0.5f;
        
        // 限制俯仰角
        cameraPitch_ = std::max(-89.0f, std::min(89.0f, cameraPitch_));
        
        // 更新相机位置
        float yawRad = cameraYaw_ * M_PI / 180.0f;
        float pitchRad = cameraPitch_ * M_PI / 180.0f;
        
        cameraPosition_.setX(cameraTarget_.x() + cameraDistance_ * std::cos(pitchRad) * std::cos(yawRad));
        cameraPosition_.setY(cameraTarget_.y() + cameraDistance_ * std::sin(pitchRad));
        cameraPosition_.setZ(cameraTarget_.z() + cameraDistance_ * std::cos(pitchRad) * std::sin(yawRad));
        
        update();
    } else if (mouseButton_ == Qt::RightButton) {
        // 平移相机
        QVector3D right = QVector3D::crossProduct(cameraTarget_ - cameraPosition_, cameraUp_).normalized();
        QVector3D up = QVector3D::crossProduct(right, cameraTarget_ - cameraPosition_).normalized();
        
        float moveSpeed = 0.01f;
        cameraTarget_ += right * delta.x() * moveSpeed;
        cameraTarget_ += up * delta.y() * moveSpeed;
        cameraPosition_ += right * delta.x() * moveSpeed;
        cameraPosition_ += up * delta.y() * moveSpeed;
        
        update();
    }
    
    lastMousePos_ = event->pos();
}

void OpenGLWidget::mouseReleaseEvent(QMouseEvent *event) {
    mousePressed_ = false;
}

void OpenGLWidget::wheelEvent(QWheelEvent *event) {
    float delta = event->angleDelta().y() / 120.0f;
    cameraDistance_ *= (1.0f - delta * 0.1f);
    cameraDistance_ = std::max(0.1f, std::min(100.0f, cameraDistance_));
    
    // 更新相机位置
    QVector3D direction = (cameraPosition_ - cameraTarget_).normalized();
    cameraPosition_ = cameraTarget_ + direction * cameraDistance_;
    
    update();
}

void OpenGLWidget::addModel(std::shared_ptr<Model> model) {
    models_.push_back(model);
    update();
}

void OpenGLWidget::removeModel(std::shared_ptr<Model> model) {
    auto it = std::find(models_.begin(), models_.end(), model);
    if (it != models_.end()) {
        models_.erase(it);
        update();
    }
}

void OpenGLWidget::clearModels() {
    models_.clear();
    update();
}

void OpenGLWidget::resetCamera() {
    cameraPosition_ = QVector3D(0, 0, 10);
    cameraTarget_ = QVector3D(0, 0, 0);
    cameraUp_ = QVector3D(0, 1, 0);
    cameraDistance_ = 10.0f;
    cameraYaw_ = 0.0f;
    cameraPitch_ = 0.0f;
    update();
}

void OpenGLWidget::setupLighting() {
    glEnable(GL_LIGHTING);
    glEnable(GL_LIGHT0);
    
    // 设置光源
    GLfloat lightPos[] = { 10.0f, 10.0f, 10.0f, 1.0f };
    GLfloat lightAmbient[] = { 0.2f, 0.2f, 0.2f, 1.0f };
    GLfloat lightDiffuse[] = { 0.8f, 0.8f, 0.8f, 1.0f };
    GLfloat lightSpecular[] = { 1.0f, 1.0f, 1.0f, 1.0f };
    
    glLightfv(GL_LIGHT0, GL_POSITION, lightPos);
    glLightfv(GL_LIGHT0, GL_AMBIENT, lightAmbient);
    glLightfv(GL_LIGHT0, GL_DIFFUSE, lightDiffuse);
    glLightfv(GL_LIGHT0, GL_SPECULAR, lightSpecular);
}

void OpenGLWidget::setupShaders() {
    // 这里将设置着色器程序
    // 为简化实现，暂时使用固定管线
}

void OpenGLWidget::updateProjection() {
    int width = this->width();
    int height = this->height();
    float aspect = width / static_cast<float>(height);
    
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    
    // 手动实现透视投影 (替代 gluPerspective)
    float fovy = 45.0f * 3.14159265f / 180.0f; // 转换为弧度
    float f = 1.0f / tan(fovy / 2.0f);
    float zNear = 0.1f;
    float zFar = 100.0f;
    
    float matrix[16] = {
        f / aspect, 0, 0, 0,
        0, f, 0, 0,
        0, 0, (zFar + zNear) / (zNear - zFar), -1,
        0, 0, (2 * zFar * zNear) / (zNear - zFar), 0
    };
    glMultMatrixf(matrix);
    
    glMatrixMode(GL_MODELVIEW);
}

void OpenGLWidget::drawGrid() {
    glDisable(GL_LIGHTING);
    glColor3f(0.5f, 0.5f, 0.5f);
    
    glBegin(GL_LINES);
    for (int i = -10; i <= 10; ++i) {
        glVertex3f(i, 0, -10);
        glVertex3f(i, 0, 10);
        glVertex3f(-10, 0, i);
        glVertex3f(10, 0, i);
    }
    glEnd();
    
    glEnable(GL_LIGHTING);
}

void OpenGLWidget::drawAxes() {
    glDisable(GL_LIGHTING);
    glLineWidth(3.0f);
    
    glBegin(GL_LINES);
    // X轴 - 红色
    glColor3f(1.0f, 0.0f, 0.0f);
    glVertex3f(0, 0, 0);
    glVertex3f(2, 0, 0);
    
    // Y轴 - 绿色
    glColor3f(0.0f, 1.0f, 0.0f);
    glVertex3f(0, 0, 0);
    glVertex3f(0, 2, 0);
    
    // Z轴 - 蓝色
    glColor3f(0.0f, 0.0f, 1.0f);
    glVertex3f(0, 0, 0);
    glVertex3f(0, 0, 2);
    glEnd();
    
    glLineWidth(1.0f);
    glEnable(GL_LIGHTING);
}

void OpenGLWidget::drawModels() {
    // 预计算当前帧在所选轴上的全局范围（厘米）并构造 O(1) 的映射函数
    float minCm = 0.0f, maxCm = 0.0f;
    computeAxisRange(coordinateAxis_, minCm, maxCm);
    float rangeCm = maxCm - minCm;
    if (rangeCm <= 1e-8f) rangeCm = 1.0f; // 防止除零
    auto mapCoordToTFast = [&](float coord_m) -> float {
        float c_cm = coord_m * unitToCm_;
        float t = (c_cm - minCm) / rangeCm;
        if (t < 0.0f) t = 0.0f; else if (t > 1.0f) t = 1.0f;
        return t;
    };

    for (size_t i = 0; i < models_.size(); ++i) {
        const auto& model = models_[i];
        if (!model) continue;
        
        // 保存当前矩阵
        glPushMatrix();
        
        // 不需要平移到重心，直接绘制在原始位置
        
        // 绘制模型
        const auto& vertices = model->getVertices();
        const auto& triangles = model->getTriangles();
        
        if (model->getType() == "PointCloud") {
            // 点云：仅根据绝对坐标位置映射伪彩色（不依赖内部关系）
            glDisable(GL_LIGHTING);
            glPointSize(3.0f);
            glBegin(GL_POINTS);
            for (size_t vi = 0; vi < vertices.size(); ++vi) {
                const auto& v = vertices[vi];
                if (pseudoColorEnabled_) {
                    float coord = (coordinateAxis_ == 0 ? v.position.x() : (coordinateAxis_ == 1 ? v.position.y() : v.position.z()));
                    float t = mapCoordToTFast(coord);
                    float r, g, b; computePseudoColor(t, r, g, b); glColor3f(r, g, b);
                } else {
                    const QColor& c = v.color; glColor3f(c.redF(), c.greenF(), c.blueF());
                }
                glVertex3f(v.position.x() * unitToCm_,
                           v.position.y() * unitToCm_,
                           v.position.z() * unitToCm_);
            }
            glEnd();
            glPointSize(1.0f);
            glEnable(GL_LIGHTING);
        } else if (model->getType() == "Mesh") {
            // 绘制网格
            bool disabledLighting = false;
            bool enabledColorMaterial = false;
            if (pseudoColorEnabled_) {
                glDisable(GL_LIGHTING);
                disabledLighting = true;
            } else {
                // 非伪彩色时启用颜色材质，让 glColor 影响光照下的材质颜色
                glEnable(GL_COLOR_MATERIAL);
                glColorMaterial(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE);
                enabledColorMaterial = true;
            }
            glBegin(GL_TRIANGLES);
            for (size_t i = 0; i < triangles.size(); i += 3) {
                if (i + 2 < triangles.size()) {
                    unsigned int i1 = triangles[i];
                    unsigned int i2 = triangles[i + 1];
                    unsigned int i3 = triangles[i + 2];
                    
                    if (i1 < vertices.size() && i2 < vertices.size() && i3 < vertices.size()) {
                        const Vertex& v1 = vertices[i1];
                        const Vertex& v2 = vertices[i2];
                        const Vertex& v3 = vertices[i3];
                        
                        // 设置法线
                        glNormal3f(v1.normal.x(), v1.normal.y(), v1.normal.z());
                        if (pseudoColorEnabled_) {
                            float coord = (coordinateAxis_==0? v1.position.x() : (coordinateAxis_==1? v1.position.y() : v1.position.z()));
                            float t = mapCoordToTFast(coord);
                            float r,g,b; computePseudoColor(t, r, g, b);
                            glColor3f(r,g,b);
                        } else {
                            glColor3f(v1.color.redF(), v1.color.greenF(), v1.color.blueF());
                        }
                        glVertex3f(v1.position.x()*unitToCm_, v1.position.y()*unitToCm_, v1.position.z()*unitToCm_);
                        
                        glNormal3f(v2.normal.x(), v2.normal.y(), v2.normal.z());
                        if (pseudoColorEnabled_) {
                            float coord = (coordinateAxis_==0? v2.position.x() : (coordinateAxis_==1? v2.position.y() : v2.position.z()));
                            float t = mapCoordToTFast(coord);
                            float r,g,b; computePseudoColor(t, r, g, b);
                            glColor3f(r,g,b);
                        } else {
                            glColor3f(v2.color.redF(), v2.color.greenF(), v2.color.blueF());
                        }
                        glVertex3f(v2.position.x()*unitToCm_, v2.position.y()*unitToCm_, v2.position.z()*unitToCm_);
                        
                        glNormal3f(v3.normal.x(), v3.normal.y(), v3.normal.z());
                        if (pseudoColorEnabled_) {
                            float coord = (coordinateAxis_==0? v3.position.x() : (coordinateAxis_==1? v3.position.y() : v3.position.z()));
                            float t = mapCoordToTFast(coord);
                            float r,g,b; computePseudoColor(t, r, g, b);
                            glColor3f(r,g,b);
                        } else {
                            glColor3f(v3.color.redF(), v3.color.greenF(), v3.color.blueF());
                        }
                        glVertex3f(v3.position.x()*unitToCm_, v3.position.y()*unitToCm_, v3.position.z()*unitToCm_);
                    }
                }
            }
            glEnd();
            if (disabledLighting) glEnable(GL_LIGHTING);
            if (enabledColorMaterial) glDisable(GL_COLOR_MATERIAL);
        }
        
        // 如果是选中的模型，绘制包围盒高亮
        if (static_cast<int>(i) == selectedModelIndex_) {
            AABB aabb = model->computeAABB();
            QVector3D minP = aabb.min * unitToCm_;
            QVector3D maxP = aabb.max * unitToCm_;
            
            glDisable(GL_LIGHTING);
            glLineWidth(2.0f);
            glColor3f(1.0f, 1.0f, 0.0f); // 黄色边框
            
            // 绘制包围盒
            glBegin(GL_LINES);
            // 底面
            glVertex3f(minP.x(), minP.y(), minP.z()); glVertex3f(maxP.x(), minP.y(), minP.z());
            glVertex3f(maxP.x(), minP.y(), minP.z()); glVertex3f(maxP.x(), minP.y(), maxP.z());
            glVertex3f(maxP.x(), minP.y(), maxP.z()); glVertex3f(minP.x(), minP.y(), maxP.z());
            glVertex3f(minP.x(), minP.y(), maxP.z()); glVertex3f(minP.x(), minP.y(), minP.z());
            // 顶面
            glVertex3f(minP.x(), maxP.y(), minP.z()); glVertex3f(maxP.x(), maxP.y(), minP.z());
            glVertex3f(maxP.x(), maxP.y(), minP.z()); glVertex3f(maxP.x(), maxP.y(), maxP.z());
            glVertex3f(maxP.x(), maxP.y(), maxP.z()); glVertex3f(minP.x(), maxP.y(), maxP.z());
            glVertex3f(minP.x(), maxP.y(), maxP.z()); glVertex3f(minP.x(), maxP.y(), minP.z());
            // 垂直边
            glVertex3f(minP.x(), minP.y(), minP.z()); glVertex3f(minP.x(), maxP.y(), minP.z());
            glVertex3f(maxP.x(), minP.y(), minP.z()); glVertex3f(maxP.x(), maxP.y(), minP.z());
            glVertex3f(maxP.x(), minP.y(), maxP.z()); glVertex3f(maxP.x(), maxP.y(), maxP.z());
            glVertex3f(minP.x(), minP.y(), maxP.z()); glVertex3f(minP.x(), maxP.y(), maxP.z());
            glEnd();
            
            glLineWidth(1.0f);
            glEnable(GL_LIGHTING);
        }
        
        // 恢复矩阵
        glPopMatrix();
    }
}

    // （移除重复的 mapCoordToT 与 computeAxisRange 定义，保留单一实现）

// t in [0,1]
void OpenGLWidget::computePseudoColor(float t, float& r, float& g, float& b) const {
    if (t < 0.0f) t = 0.0f; if (t > 1.0f) t = 1.0f;
    switch (colorMapMode_) {
        case 0: {
            // Rainbow: HSV hue 240->0 (蓝到红)
            float hue = t * 240.0f; // 0..240
            float saturation = 1.0f;
            float value = 1.0f;
            float c = value * saturation;
            float x = c * (1 - std::fabs(fmod(hue / 60.0f, 2.0f) - 1));
            float m = value - c;
            float rr=0, gg=0, bb=0;
            if (hue < 60) { rr=c; gg=x; bb=0; }
            else if (hue < 120) { rr=x; gg=c; bb=0; }
            else if (hue < 180) { rr=0; gg=c; bb=x; }
            else if (hue < 240) { rr=0; gg=x; bb=c; }
            else if (hue < 300) { rr=x; gg=0; bb=c; }
            else { rr=c; gg=0; bb=x; }
            r = rr + m; g = gg + m; b = bb + m;
            break;
        }
        case 1: {
            // Viridis 近似（线性插值近似版）
            // 取三段控制点进行插值（粗略）
            struct C{float r,g,b;};
            const C c0{0.267f,0.004f,0.329f};
            const C c1{0.283f,0.141f,0.458f};
            const C c2{0.254f,0.265f,0.530f};
            const C c3{0.207f,0.372f,0.553f};
            const C c4{0.164f,0.471f,0.558f};
            const C c5{0.134f,0.566f,0.551f};
            const C c6{0.270f,0.699f,0.485f};
            const C c7{0.477f,0.821f,0.318f};
            const C c8{0.993f,0.906f,0.144f};
            const C stops[] = {c0,c1,c2,c3,c4,c5,c6,c7,c8};
            const int N = 8; // 8 segments
            float ft = t * N;
            int i = static_cast<int>(ft);
            if (i<0) i=0; if (i>N) i=N;
            float lt = ft - i;
            const C &a = stops[i], &d = stops[std::min(i+1, N)];
            r = a.r + (d.r - a.r)*lt;
            g = a.g + (d.g - a.g)*lt;
            b = a.b + (d.b - a.b)*lt;
            break;
        }
        case 2: {
            // Red-Blue: 0->蓝, 0.5->白, 1->红（对称）
            if (t < 0.5f) {
                float k = t/0.5f; // 0..1 蓝到白
                r = k; g = k; b = 1.0f;
            } else {
                float k = (t-0.5f)/0.5f; // 0..1 白到红
                r = 1.0f; g = 1.0f - k; b = 1.0f - k;
            }
            break;
        }
        default: {
            r = g = b = t; // 灰度保底
        }
    }
}

// 旧版 mapCoordToT 已移除：避免逐顶点重复遍历模型数据导致性能问题

bool OpenGLWidget::computeAxisRange(int axis, float& minV, float& maxV) const {
    bool has = false;
    minV = std::numeric_limits<float>::infinity();
    maxV = -std::numeric_limits<float>::infinity();
    for (const auto& m : models_) {
        if (!m) continue;
        const auto& verts = m->getVertices();
        for (const auto& v : verts) {
            float c_m = (axis==0? v.position.x() : (axis==1? v.position.y() : v.position.z()));
            float c = c_m * unitToCm_; // 转为厘米
            if (c < minV) minV = c;
            if (c > maxV) maxV = c;
            has = true;
        }
    }
    if (!has) { minV = 0.0f; maxV = 0.0f; }
    return has;
}
