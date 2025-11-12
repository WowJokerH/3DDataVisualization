#include "Model.h"
#include <QDebug>

int Model::totalModelCount_ = 0;

Model::Model(const QString& name) 
    : name_(name), color_(Qt::white), position_(0, 0, 0), 
      rotation_(0, 0, 0), scale_(1, 1, 1) {
    totalModelCount_++;
}

Model::~Model() {
    totalModelCount_--;
}

void Model::translate(const QVector3D& offset) {
    position_ += offset;
    // 更新所有顶点位置
    for (auto& vertex : vertices_) {
        vertex.position += offset;
    }
}

void Model::translateTo(const QVector3D& position) {
    QVector3D offset = position - position_;
    translate(offset);
}

void Model::translateCm(const QVector3D& offsetCm) {
    // 将厘米偏移转换为米后沿用现有逻辑
    QVector3D offsetM = offsetCm / 100.0f;
    translate(offsetM);
}

void Model::translateToCm(const QVector3D& positionCm) {
    QVector3D positionM = positionCm / 100.0f;
    translateTo(positionM);
}

void Model::rotate(const QVector3D& axis, float angle) {
    rotation_ += axis * angle;
    // 这里可以实现具体的旋转逻辑
    // 为了简化，暂时只记录旋转状态
}

void Model::scale(const QVector3D& factors) {
    scale_ *= factors;
    // 更新所有顶点位置
    for (auto& vertex : vertices_) {
        vertex.position *= factors;
    }
}

QVector3D Model::computeCenter() const {
    // 返回单位：厘米（假设内部存储为米）
    if (vertices_.empty()) {
        return QVector3D(0, 0, 0);
    }
    QVector3D center(0, 0, 0);
    for (const auto& vertex : vertices_) {
        center += vertex.position; // m
    }
    center /= static_cast<float>(vertices_.size());
    return center * 100.0f; // m -> cm
}

AABB Model::computeAABB() const {
    // 保持内部单位（米）以避免重复换算；渲染层负责按厘米显示
    AABB aabb;
    for (const auto& vertex : vertices_) {
        aabb.expand(vertex.position);
    }
    return aabb;
}

void Model::setColor(const QColor& color) {
    color_ = color;
    updateVertexColors(color);
}

void Model::updateVertexColors(const QColor& color) {
    for (auto& vertex : vertices_) {
        vertex.color = color;
    }
}