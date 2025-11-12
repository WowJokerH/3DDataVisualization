#pragma once

#include <QVector3D>
#include <QColor>

class ColorMapper {
public:
    // 基于坐标值的伪彩色映射
    static QColor mapByCoordinate(const QVector3D& position, int axis);
    static QColor mapByHeight(float height, float minHeight, float maxHeight);
    static QColor mapByDistance(const QVector3D& position, const QVector3D& reference);
    
    // HSV颜色空间转换
    static QColor hsvToRgb(float hue, float saturation, float value);
    
private:
    static float interpolate(float value, float min1, float max1, float min2, float max2);
};