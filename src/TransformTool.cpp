#include "TransformTool.h"
#include <cmath>

QVector3D TransformTool::translate(const QVector3D& point, const QVector3D& offset) {
    return point + offset;
}

std::vector<QVector3D> TransformTool::translate(const std::vector<QVector3D>& points, const QVector3D& offset) {
    std::vector<QVector3D> result;
    result.reserve(points.size());
    
    for (const auto& point : points) {
        result.push_back(translate(point, offset));
    }
    
    return result;
}

QVector3D TransformTool::rotate(const QVector3D& point, const QVector3D& axis, float angle) {
    QMatrix4x4 rotationMatrix = createRotationMatrix(axis, angle);
    return rotationMatrix.map(point);
}

std::vector<QVector3D> TransformTool::rotate(const std::vector<QVector3D>& points, const QVector3D& axis, float angle) {
    std::vector<QVector3D> result;
    result.reserve(points.size());
    
    QMatrix4x4 rotationMatrix = createRotationMatrix(axis, angle);
    for (const auto& point : points) {
        result.push_back(rotationMatrix.map(point));
    }
    
    return result;
}

QVector3D TransformTool::scale(const QVector3D& point, const QVector3D& factors) {
    return QVector3D(point.x() * factors.x(), point.y() * factors.y(), point.z() * factors.z());
}

std::vector<QVector3D> TransformTool::scale(const std::vector<QVector3D>& points, const QVector3D& factors) {
    std::vector<QVector3D> result;
    result.reserve(points.size());
    
    for (const auto& point : points) {
        result.push_back(scale(point, factors));
    }
    
    return result;
}

QMatrix4x4 TransformTool::createTransformMatrix(const QVector3D& translation, const QVector3D& rotation, const QVector3D& scale) {
    QMatrix4x4 matrix;
    matrix.setToIdentity();
    
    // 应用缩放
    matrix.scale(scale);
    
    // 应用旋转（简化版本，分别绕X、Y、Z轴旋转）
    matrix.rotate(rotation.x(), QVector3D(1, 0, 0));
    matrix.rotate(rotation.y(), QVector3D(0, 1, 0));
    matrix.rotate(rotation.z(), QVector3D(0, 0, 1));
    
    // 应用平移
    matrix.translate(translation);
    
    return matrix;
}

QMatrix4x4 TransformTool::createRotationMatrix(const QVector3D& axis, float angle) {
    QMatrix4x4 matrix;
    matrix.setToIdentity();
    
    // 标准化轴向量
    QVector3D normalizedAxis = axis.normalized();
    
    // 使用Rodrigues旋转公式
    float radians = angle * M_PI / 180.0f;
    float cosAngle = std::cos(radians);
    float sinAngle = std::sin(radians);
    float oneMinusCos = 1.0f - cosAngle;
    
    float x = normalizedAxis.x();
    float y = normalizedAxis.y();
    float z = normalizedAxis.z();
    
    matrix(0, 0) = cosAngle + x * x * oneMinusCos;
    matrix(0, 1) = x * y * oneMinusCos - z * sinAngle;
    matrix(0, 2) = x * z * oneMinusCos + y * sinAngle;
    
    matrix(1, 0) = y * x * oneMinusCos + z * sinAngle;
    matrix(1, 1) = cosAngle + y * y * oneMinusCos;
    matrix(1, 2) = y * z * oneMinusCos - x * sinAngle;
    
    matrix(2, 0) = z * x * oneMinusCos - y * sinAngle;
    matrix(2, 1) = z * y * oneMinusCos + x * sinAngle;
    matrix(2, 2) = cosAngle + z * z * oneMinusCos;
    
    return matrix;
}

QMatrix4x4 TransformTool::createScaleMatrix(const QVector3D& factors) {
    QMatrix4x4 matrix;
    matrix.setToIdentity();
    matrix.scale(factors);
    return matrix;
}

QMatrix4x4 TransformTool::createTranslationMatrix(const QVector3D& translation) {
    QMatrix4x4 matrix;
    matrix.setToIdentity();
    matrix.translate(translation);
    return matrix;
}