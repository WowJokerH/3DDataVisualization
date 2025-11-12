#pragma once

#include <QVector3D>
#include <QColor>

struct Vertex {
    QVector3D position;
    QVector3D normal;
    QColor color;
    
    Vertex() : position(0, 0, 0), normal(0, 0, 1), color(Qt::white) {}
    Vertex(const QVector3D& pos, const QColor& col = Qt::white) 
        : position(pos), normal(0, 0, 1), color(col) {}
    Vertex(const QVector3D& pos, const QVector3D& norm, const QColor& col = Qt::white)
        : position(pos), normal(norm), color(col) {}
};