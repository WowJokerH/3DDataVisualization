#pragma once

#include <QString>
#include <QVector3D>
#include <vector>
#include <memory>

class Model;
class PointCloud;
class Mesh;

class ModelAnalyzer {
public:
    static QString analyzePointCloud(std::shared_ptr<PointCloud> pointCloud);
    static QString analyzeMesh(std::shared_ptr<Mesh> mesh);
    
private:
    static QString formatVector3D(const QVector3D& vec);
};
