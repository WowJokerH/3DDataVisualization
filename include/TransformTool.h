#pragma once

#include <QVector3D>
#include <QMatrix4x4>

class TransformTool {
public:
    // 平移变换
    static QVector3D translate(const QVector3D& point, const QVector3D& offset);
    static std::vector<QVector3D> translate(const std::vector<QVector3D>& points, const QVector3D& offset);
    
    // 旋转变换
    static QVector3D rotate(const QVector3D& point, const QVector3D& axis, float angle);
    static std::vector<QVector3D> rotate(const std::vector<QVector3D>& points, const QVector3D& axis, float angle);
    
    // 缩放变换
    static QVector3D scale(const QVector3D& point, const QVector3D& factors);
    static std::vector<QVector3D> scale(const std::vector<QVector3D>& points, const QVector3D& factors);
    
    // 组合变换
    static QMatrix4x4 createTransformMatrix(const QVector3D& translation, const QVector3D& rotation, const QVector3D& scale);
    
private:
    static QMatrix4x4 createRotationMatrix(const QVector3D& axis, float angle);
    static QMatrix4x4 createScaleMatrix(const QVector3D& factors);
    static QMatrix4x4 createTranslationMatrix(const QVector3D& translation);
};