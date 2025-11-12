#pragma once

#include "Model.h"

class PointCloud : public Model {
public:
    PointCloud(const QString& name = "Point Cloud");
    virtual ~PointCloud();
    
    // 点云特定方法
    void addPoint(const QVector3D& point, const QColor& color = Qt::white);
    void addPoint(const Vertex& vertex);
    void clear();
    
    // 统计信息
    size_t getPointCount() const { return getVertexCount(); }
    
    // 计算结果缓存
    QVector3D computeCenter() const override;
    AABB computeAABB() const override;
    
    // 变换操作
    void translate(const QVector3D& offset) override;
    void translateTo(const QVector3D& position) override;
    void rotate(const QVector3D& axis, float angle) override;
    void scale(const QVector3D& factors) override;
    
    // 重写虚函数
    void update() override;
    void render() override;
    QString getType() const override { return "PointCloud"; }
    
    // 静态计数器
    static int getPointCloudCount() { return pointCloudCount_; }
    
private:
    static int pointCloudCount_;
    
    void markDirty() const;
    void updateStatistics() const;
    
    mutable bool statsDirty_;
    mutable QVector3D cachedCenter_;
    mutable AABB cachedAABB_;
};
