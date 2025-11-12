#pragma once

#include "Model.h"

class Mesh : public Model {
public:
    Mesh(const QString& name = "Mesh");
    virtual ~Mesh();
    
    // 网格特定方法
    void addVertex(const QVector3D& vertex, const QVector3D& normal = QVector3D(0,0,1), const QColor& color = Qt::white);
    void addTriangle(unsigned int i1, unsigned int i2, unsigned int i3);
    void clear();
    
    // 统计信息
    size_t getFaceCount() const { return getTriangleCount(); }
    
    // 计算表面积
    float computeSurfaceArea() const;
    
    // 重写虚函数
    void update() override;
    void render() override;
    QString getType() const override { return "Mesh"; }
    
    // 静态计数器
    static int getMeshCount() { return meshCount_; }
    
private:
    static int meshCount_;
};