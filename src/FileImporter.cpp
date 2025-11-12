#include "FileImporter.h"
#include "PointCloud.h"
#include "Mesh.h"
#include <QFile>
#include <QTextStream>
#include <QFileInfo>
#include <QDebug>

std::shared_ptr<Model> FileImporter::importFile(const QString& filePath) {
    FileFormat format = detectFormat(filePath);
    
    switch (format) {
        case PLY:
            return importPLY(filePath);
        case OBJ:
            return importOBJ(filePath);
        case XYZ:
            return importXYZ(filePath);
        default:
            qDebug() << "不支持的文件格式: " << filePath;
            return nullptr;
    }
}

bool FileImporter::exportFile(std::shared_ptr<Model> model, const QString& filePath) {
    if (!model) return false;
    
    FileFormat format = detectFormat(filePath);
    
    switch (format) {
        case PLY:
            return exportPLY(model, filePath);
        case OBJ:
            return exportOBJ(model, filePath);
        case XYZ:
            return exportXYZ(model, filePath);
        default:
            qDebug() << "不支持的导出格式: " << filePath;
            return false;
    }
}

FileImporter::FileFormat FileImporter::detectFormat(const QString& filePath) {
    QFileInfo fileInfo(filePath);
    QString suffix = fileInfo.suffix().toLower();
    
    if (suffix == "ply") return PLY;
    if (suffix == "obj") return OBJ;
    if (suffix == "xyz") return XYZ;
    
    return UNKNOWN;
}

std::shared_ptr<Model> FileImporter::importPLY(const QString& filePath) {
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        qDebug() << "无法打开文件: " << filePath;
        return nullptr;
    }
    
    QTextStream in(&file);
    QString line = in.readLine();
    
    if (line != "ply") {
        qDebug() << "不是有效的PLY文件";
        return nullptr;
    }
    
    // 简化的PLY解析器
    int vertexCount = 0;
    int faceCount = 0;
    bool isAscii = true;
    
    // 读取头部信息
    while (!in.atEnd()) {
        line = in.readLine().trimmed();
        if (line == "end_header") break;
        
        if (line.startsWith("element vertex")) {
            vertexCount = line.split(" ").last().toInt();
        } else if (line.startsWith("element face")) {
            faceCount = line.split(" ").last().toInt();
        } else if (line == "format binary_little_endian 1.0") {
            isAscii = false;
        }
    }
    
    if (vertexCount == 0) {
        qDebug() << "没有找到顶点数据";
        return nullptr;
    }
    
    // 仅支持ASCII简化解析
    if (!isAscii) {
        qDebug() << "暂不支持二进制PLY，尝试按点云读取顶点";
    }

    const QString baseName = QFileInfo(filePath).baseName();
    if (faceCount > 0) {
        // 读取为Mesh
        auto mesh = std::make_shared<Mesh>(baseName);
        // 读取顶点
        for (int i = 0; i < vertexCount && !in.atEnd(); ++i) {
            line = in.readLine();
            QStringList coords = line.split(" ", Qt::SkipEmptyParts);
            if (coords.size() >= 3) {
                float x = coords[0].toFloat();
                float y = coords[1].toFloat();
                float z = coords[2].toFloat();
                QColor color = Qt::white;
                if (coords.size() >= 6) {
                    int r = qBound(0, coords[3].toInt(), 255);
                    int g = qBound(0, coords[4].toInt(), 255);
                    int b = qBound(0, coords[5].toInt(), 255);
                    color = QColor(r, g, b);
                }
                mesh->addVertex(QVector3D(x, y, z), QVector3D(0,0,1), color);
            }
        }
        // 读取面（每行: n i0 i1 i2 ...）
        for (int f = 0; f < faceCount && !in.atEnd(); ++f) {
            line = in.readLine().trimmed();
            if (line.isEmpty()) { --f; continue; }
            QStringList parts = line.split(" ", Qt::SkipEmptyParts);
            if (parts.size() < 4) continue; // 至少: n i j k
            bool ok=false;
            int n = parts[0].toInt(&ok);
            if (!ok || n < 3 || parts.size() < n+1) continue;
            std::vector<unsigned int> idx;
            idx.reserve(n);
            for (int k = 0; k < n; ++k) {
                unsigned int vi = parts[1+k].toUInt();
                idx.push_back(vi);
            }
            // 扇形三角化
            for (int k = 1; k < n-1; ++k) {
                mesh->addTriangle(idx[0], idx[k], idx[k+1]);
            }
        }
        file.close();
        return mesh;
    } else {
        // 无面时作为点云
        auto pointCloud = std::make_shared<PointCloud>(baseName);
        for (int i = 0; i < vertexCount && !in.atEnd(); ++i) {
            line = in.readLine();
            QStringList coords = line.split(" ", Qt::SkipEmptyParts);
            if (coords.size() >= 3) {
                float x = coords[0].toFloat();
                float y = coords[1].toFloat();
                float z = coords[2].toFloat();
                QColor color = Qt::white;
                if (coords.size() >= 6) {
                    int r = qBound(0, coords[3].toInt(), 255);
                    int g = qBound(0, coords[4].toInt(), 255);
                    int b = qBound(0, coords[5].toInt(), 255);
                    color = QColor(r, g, b);
                }
                pointCloud->addPoint(QVector3D(x, y, z), color);
            }
        }
        file.close();
        return pointCloud;
    }
}

std::shared_ptr<Model> FileImporter::importOBJ(const QString& filePath) {
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        qDebug() << "无法打开文件: " << filePath;
        return nullptr;
    }
    
    auto mesh = std::make_shared<Mesh>(QFileInfo(filePath).baseName());
    QTextStream in(&file);
    
    while (!in.atEnd()) {
        QString line = in.readLine().trimmed();
        if (line.isEmpty() || line.startsWith("#")) continue;
        
        QStringList parts = line.split(" ", Qt::SkipEmptyParts);
        if (parts.isEmpty()) continue;
        
        QString type = parts[0];
        
        if (type == "v" && parts.size() >= 4) {
            // 顶点坐标
            float x = parts[1].toFloat();
            float y = parts[2].toFloat();
            float z = parts[3].toFloat();
            mesh->addVertex(QVector3D(x, y, z));
        } else if (type == "vn" && parts.size() >= 4) {
            // 顶点法线（暂时存储，后续处理）
            float nx = parts[1].toFloat();
            float ny = parts[2].toFloat();
            float nz = parts[3].toFloat();
            // 在实际应用中，需要将法线与顶点关联
        } else if (type == "f" && parts.size() >= 4) {
            // 面片
            std::vector<unsigned int> indices;
            for (int i = 1; i < parts.size(); ++i) {
                QStringList vertexData = parts[i].split("/");
                if (!vertexData.isEmpty()) {
                    unsigned int index = vertexData[0].toUInt() - 1; // OBJ索引从1开始
                    indices.push_back(index);
                }
            }
            
            // 将多边形三角化（简化处理）
            for (size_t i = 1; i < indices.size() - 1; ++i) {
                mesh->addTriangle(indices[0], indices[i], indices[i + 1]);
            }
        }
    }
    
    file.close();
    return mesh;
}

std::shared_ptr<Model> FileImporter::importXYZ(const QString& filePath) {
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        qDebug() << "无法打开文件: " << filePath;
        return nullptr;
    }
    
    auto pointCloud = std::make_shared<PointCloud>(QFileInfo(filePath).baseName());
    QTextStream in(&file);
    
    while (!in.atEnd()) {
        QString line = in.readLine().trimmed();
        if (line.isEmpty() || line.startsWith("#")) continue;
        
        QStringList coords = line.split(" ", Qt::SkipEmptyParts);
        if (coords.size() >= 3) {
            float x = coords[0].toFloat();
            float y = coords[1].toFloat();
            float z = coords[2].toFloat();
            
            // XYZ格式通常只包含坐标，使用默认颜色
            pointCloud->addPoint(QVector3D(x, y, z));
        }
    }
    
    file.close();
    return pointCloud;
}

bool FileImporter::exportPLY(std::shared_ptr<Model> model, const QString& filePath) {
    QFile file(filePath);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        qDebug() << "无法创建文件: " << filePath;
        return false;
    }
    
    QTextStream out(&file);
    
    // 写入PLY头部
    out << "ply\n";
    out << "format ascii 1.0\n";
    out << "element vertex " << model->getVertexCount() << "\n";
    out << "property float x\n";
    out << "property float y\n";
    out << "property float z\n";
    out << "property uchar red\n";
    out << "property uchar green\n";
    out << "property uchar blue\n";
    out << "end_header\n";
    
    // 写入顶点数据
    const auto& vertices = model->getVertices();
    for (const auto& vertex : vertices) {
        out << vertex.position.x() << " " 
            << vertex.position.y() << " " 
            << vertex.position.z() << " "
            << vertex.color.red() << " "
            << vertex.color.green() << " "
            << vertex.color.blue() << "\n";
    }
    
    file.close();
    return true;
}

bool FileImporter::exportOBJ(std::shared_ptr<Model> model, const QString& filePath) {
    QFile file(filePath);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        qDebug() << "无法创建文件: " << filePath;
        return false;
    }
    
    QTextStream out(&file);
    
    // 写入顶点
    const auto& vertices = model->getVertices();
    for (const auto& vertex : vertices) {
        out << "v " << vertex.position.x() << " " 
            << vertex.position.y() << " " 
            << vertex.position.z() << "\n";
    }
    
    // 写入法线
    for (const auto& vertex : vertices) {
        out << "vn " << vertex.normal.x() << " " 
            << vertex.normal.y() << " " 
            << vertex.normal.z() << "\n";
    }
    
    // 写入面片
    const auto& triangles = model->getTriangles();
    for (size_t i = 0; i < triangles.size(); i += 3) {
        if (i + 2 < triangles.size()) {
            out << "f " << (triangles[i] + 1) << " " 
                << (triangles[i + 1] + 1) << " " 
                << (triangles[i + 2] + 1) << "\n";
        }
    }
    
    file.close();
    return true;
}

bool FileImporter::exportXYZ(std::shared_ptr<Model> model, const QString& filePath) {
    QFile file(filePath);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        qDebug() << "无法创建文件: " << filePath;
        return false;
    }
    
    QTextStream out(&file);
    
    // 写入顶点坐标（XYZ格式只包含坐标）
    const auto& vertices = model->getVertices();
    for (const auto& vertex : vertices) {
        out << vertex.position.x() << " " 
            << vertex.position.y() << " " 
            << vertex.position.z() << "\n";
    }
    
    file.close();
    return true;
}