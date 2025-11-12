#include "ModelAnalyzer.h"
#include "PointCloud.h"
#include "Mesh.h"
#include "AABB.h"

QString ModelAnalyzer::analyzePointCloud(std::shared_ptr<PointCloud> pointCloud) {
    if (!pointCloud) return "无效的点云对象";
    
    QString result;
    result += QString("点云名称: %1\n").arg(pointCloud->getName());
    result += QString("点数量: %1\n").arg(pointCloud->getPointCount());
    
    QVector3D centerCm = pointCloud->computeCenter(); // 现为厘米
    result += QString("几何重心(cm): %1\n").arg(formatVector3D(centerCm));
    
    AABB aabb = pointCloud->computeAABB();
    QVector3D minCm = aabb.min * 100.0f;
    QVector3D maxCm = aabb.max * 100.0f;
    QVector3D sizeCm = (aabb.size()) * 100.0f;
    result += QString("AABB包围盒(单位cm):\n");
    result += QString("  最小点: %1\n").arg(formatVector3D(minCm));
    result += QString("  最大点: %1\n").arg(formatVector3D(maxCm));
    result += QString("  尺寸: %1\n").arg(formatVector3D(sizeCm));
    
    return result;
}

QString ModelAnalyzer::formatVector3D(const QVector3D& vec) {
    return QString("(%1, %2, %3)")
        .arg(vec.x(), 0, 'f', 3)
        .arg(vec.y(), 0, 'f', 3)
        .arg(vec.z(), 0, 'f', 3);
}

QString ModelAnalyzer::analyzeMesh(std::shared_ptr<Mesh> mesh) {
    if (!mesh) return "无效的网格对象";
    
    QString result;
    result += QString("网格名称: %1\n").arg(mesh->getName());
    result += QString("顶点数: %1\n").arg(mesh->getVertexCount());
    result += QString("面片数: %1\n").arg(mesh->getFaceCount());
    
    QVector3D centerCm = mesh->computeCenter();
    result += QString("几何重心(cm): %1\n").arg(formatVector3D(centerCm));
    
    AABB aabb = mesh->computeAABB();
    QVector3D minCm2 = aabb.min * 100.0f;
    QVector3D maxCm2 = aabb.max * 100.0f;
    QVector3D sizeCm2 = (aabb.size()) * 100.0f;
    result += QString("AABB包围盒(单位cm):\n");
    result += QString("  最小点: %1\n").arg(formatVector3D(minCm2));
    result += QString("  最大点: %1\n").arg(formatVector3D(maxCm2));
    result += QString("  尺寸: %1\n").arg(formatVector3D(sizeCm2));
    
    float surfaceAreaCm2 = mesh->computeSurfaceArea(); // 已切换为cm²
    result += QString("表面积(cm^2): %1\n").arg(surfaceAreaCm2, 0, 'f', 3);
    
    return result;
}
