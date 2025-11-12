#include "Mesh.h"
#include <QDebug>
#include <cmath>

int Mesh::meshCount_ = 0;

Mesh::Mesh(const QString& name) : Model(name) {
    meshCount_++;
}

Mesh::~Mesh() {
    meshCount_--;
}

void Mesh::addVertex(const QVector3D& vertex, const QVector3D& normal, const QColor& color) {
    vertices_.emplace_back(vertex, normal, color);
}

void Mesh::addTriangle(unsigned int i1, unsigned int i2, unsigned int i3) {
    triangles_.push_back(i1);
    triangles_.push_back(i2);
    triangles_.push_back(i3);
}

void Mesh::clear() {
    vertices_.clear();
    triangles_.clear();
}

float Mesh::computeSurfaceArea() const {
    // 返回单位：平方厘米（假设内部顶点单位为米，需要换算）
    double areaM2 = 0.0;
    for (size_t i = 0; i + 2 < triangles_.size(); i += 3) {
        unsigned int i1 = triangles_[i];
        unsigned int i2 = triangles_[i + 1];
        unsigned int i3 = triangles_[i + 2];
        if (i1 >= vertices_.size() || i2 >= vertices_.size() || i3 >= vertices_.size()) continue;
        const QVector3D& v1 = vertices_[i1].position;
        const QVector3D& v2 = vertices_[i2].position;
        const QVector3D& v3 = vertices_[i3].position;
        QVector3D edge1 = v2 - v1;
        QVector3D edge2 = v3 - v1;
        double triArea = 0.5 * QVector3D::crossProduct(edge1, edge2).length(); // m²
        areaM2 += triArea;
    }
    return static_cast<float>(areaM2 * 10000.0); // m² -> cm²
}

void Mesh::update() {
    // 更新网格的边界信息、法线等
    // 这里可以添加网格特定的更新逻辑
}

void Mesh::render() {
    // 实际的渲染逻辑将在OpenGLWidget中实现
    // 这里只是标记需要重新渲染
}