#pragma once

#include <QString>
#include <QVector3D>
#include <QColor>
#include <vector>
#include <memory>
#include "Vertex.h"
#include "AABB.h"

class Model {
public:
    Model(const QString& name = "Unnamed Model");
    virtual ~Model();
    
    // 基本属性
    QString getName() const { return name_; }
    void setName(const QString& name) { name_ = name; }
    
    size_t getVertexCount() const { return vertices_.size(); }
    size_t getTriangleCount() const { return triangles_.size() / 3; }
    
    // 颜色属性
    QColor getColor() const { return color_; }
    void setColor(const QColor& color);
    void updateVertexColors(const QColor& color);
    
    // 变换操作
    virtual void translate(const QVector3D& offset);
    virtual void translateTo(const QVector3D& position);
    // 新增：厘米制的位移接口（传入cm，内部转米）
    virtual void translateCm(const QVector3D& offsetCm);
    virtual void translateToCm(const QVector3D& positionCm);
    virtual void rotate(const QVector3D& axis, float angle);
    virtual void scale(const QVector3D& factors);
    
    // 核心计算
    // 注意：以下返回值单位调整为厘米（cm）
    virtual QVector3D computeCenter() const;
    virtual AABB computeAABB() const;
    
    // 获取数据
    const std::vector<Vertex>& getVertices() const { return vertices_; }
    const std::vector<unsigned int>& getTriangles() const { return triangles_; }
    
    // 虚函数 - 子类必须实现
    virtual void update() = 0;
    virtual void render() = 0;
    virtual QString getType() const = 0;
    
    // 静态计数器
    static int getTotalModelCount() { return totalModelCount_; }
    
protected:
    QString name_;
    QColor color_;
    std::vector<Vertex> vertices_;
    std::vector<unsigned int> triangles_;
    QVector3D position_;
    QVector3D rotation_;
    QVector3D scale_;
    
    static int totalModelCount_;
};