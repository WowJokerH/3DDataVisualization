#include "ColorMapper.h"
#include <cmath>

QColor ColorMapper::mapByCoordinate(const QVector3D& position, int axis) {
    float coord = 0.0f;
    switch (axis) {
        case 0: coord = position.x(); break;
        case 1: coord = position.y(); break;
        case 2: coord = position.z(); break;
        default: coord = position.z(); break;
    }
    
    // 将坐标值归一化到0-1范围
    float normalized = (coord + 5.0f) / 10.0f; // 假设范围在-5到5之间
    normalized = std::max(0.0f, std::min(1.0f, normalized));
    
    // 使用HSV颜色空间：蓝色到红色
    float hue = normalized * 240.0f; // 蓝色(240°)到红色(0°)
    return hsvToRgb(hue, 1.0f, 1.0f);
}

QColor ColorMapper::mapByHeight(float height, float minHeight, float maxHeight) {
    if (maxHeight <= minHeight) return Qt::blue;
    
    float normalized = (height - minHeight) / (maxHeight - minHeight);
    normalized = std::max(0.0f, std::min(1.0f, normalized));
    
    // 从绿色到红色
    float hue = (1.0f - normalized) * 120.0f; // 绿色(120°)到红色(0°)
    return hsvToRgb(hue, 1.0f, 1.0f);
}

QColor ColorMapper::mapByDistance(const QVector3D& position, const QVector3D& reference) {
    float distance = (position - reference).length();
    
    // 将距离归一化到0-1范围
    float normalized = distance / 10.0f; // 假设最大距离为10
    normalized = std::max(0.0f, std::min(1.0f, normalized));
    
    // 使用彩虹色谱
    float hue = normalized * 360.0f;
    return hsvToRgb(hue, 1.0f, 1.0f);
}

QColor ColorMapper::hsvToRgb(float hue, float saturation, float value) {
    // 确保色相在0-360范围内
    hue = std::fmod(hue, 360.0f);
    if (hue < 0) hue += 360.0f;
    
    float c = value * saturation;
    float x = c * (1 - std::abs(std::fmod(hue / 60.0f, 2) - 1));
    float m = value - c;
    
    float r, g, b;
    if (hue < 60) {
        r = c; g = x; b = 0;
    } else if (hue < 120) {
        r = x; g = c; b = 0;
    } else if (hue < 180) {
        r = 0; g = c; b = x;
    } else if (hue < 240) {
        r = 0; g = x; b = c;
    } else if (hue < 300) {
        r = x; g = 0; b = c;
    } else {
        r = c; g = 0; b = x;
    }
    
    return QColor(static_cast<int>((r + m) * 255), 
                  static_cast<int>((g + m) * 255), 
                  static_cast<int>((b + m) * 255));
}

float ColorMapper::interpolate(float value, float min1, float max1, float min2, float max2) {
    return min2 + (value - min1) * (max2 - min2) / (max1 - min1);
}