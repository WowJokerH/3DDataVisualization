#pragma once

#include <QVector3D>
#include <cfloat>
#include <algorithm>

class AABB {
public:
    QVector3D min;
    QVector3D max;
    
    AABB() : min(FLT_MAX, FLT_MAX, FLT_MAX), max(-FLT_MAX, -FLT_MAX, -FLT_MAX) {}
    AABB(const QVector3D& minimum, const QVector3D& maximum) : min(minimum), max(maximum) {}
    
    void expand(const QVector3D& point) {
        min.setX(std::min(min.x(), point.x()));
        min.setY(std::min(min.y(), point.y()));
        min.setZ(std::min(min.z(), point.z()));
        max.setX(std::max(max.x(), point.x()));
        max.setY(std::max(max.y(), point.y()));
        max.setZ(std::max(max.z(), point.z()));
    }
    
    QVector3D center() const {
        return (min + max) * 0.5f;
    }
    
    QVector3D size() const {
        return max - min;
    }
    
    bool isValid() const {
        return min.x() <= max.x() && min.y() <= max.y() && min.z() <= max.z();
    }
    
    void reset() {
        min = QVector3D(FLT_MAX, FLT_MAX, FLT_MAX);
        max = QVector3D(-FLT_MAX, -FLT_MAX, -FLT_MAX);
    }
};
