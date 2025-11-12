#include "PointCloud.h"
#include <QDebug>

int PointCloud::pointCloudCount_ = 0;

PointCloud::PointCloud(const QString& name)
    : Model(name),
      statsDirty_(true),
      cachedCenter_(0.0f, 0.0f, 0.0f),
      cachedAABB_() {
    pointCloudCount_++;
}

PointCloud::~PointCloud() {
    pointCloudCount_--;
}

void PointCloud::addPoint(const QVector3D& point, const QColor& color) {
    vertices_.emplace_back(point, color);
    markDirty();
}

void PointCloud::addPoint(const Vertex& vertex) {
    vertices_.push_back(vertex);
    markDirty();
}

void PointCloud::clear() {
    vertices_.clear();
    cachedAABB_.reset();
    cachedCenter_ = QVector3D(0.0f, 0.0f, 0.0f);
    markDirty();
}

QVector3D PointCloud::computeCenter() const {
    updateStatistics();
    return cachedCenter_;
}

AABB PointCloud::computeAABB() const {
    updateStatistics();
    return cachedAABB_;
}

void PointCloud::translate(const QVector3D& offset) {
    Model::translate(offset);
    markDirty();
}

void PointCloud::translateTo(const QVector3D& position) {
    Model::translateTo(position);
    markDirty();
}

void PointCloud::rotate(const QVector3D& axis, float angle) {
    Model::rotate(axis, angle);
    markDirty();
}

void PointCloud::scale(const QVector3D& factors) {
    Model::scale(factors);
    markDirty();
}

void PointCloud::update() {
    updateStatistics();
}

void PointCloud::render() {
    // Rendering is centralized inside OpenGLWidget; keep interface consistent.
    qDebug() << "PointCloud render call:" << name_
             << "point count:" << vertices_.size();
}

void PointCloud::markDirty() const {
    statsDirty_ = true;
}

void PointCloud::updateStatistics() const {
    if (!statsDirty_) {
        return;
    }
    
    if (vertices_.empty()) {
        cachedCenter_ = QVector3D(0.0f, 0.0f, 0.0f);
        cachedAABB_.reset();
        statsDirty_ = false;
        return;
    }
    
    QVector3D sum(0.0f, 0.0f, 0.0f);
    cachedAABB_.reset();
    
    for (const auto& vertex : vertices_) {
        sum += vertex.position;
        cachedAABB_.expand(vertex.position);
    }
    
    cachedCenter_ = sum / static_cast<float>(vertices_.size());
    statsDirty_ = false;
}
