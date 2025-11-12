#pragma once

#include <QString>
#include <vector>
#include <memory>

class Model;
class PointCloud;
class Mesh;

class FileImporter {
public:
    // 支持的文件格式
    enum FileFormat {
        PLY,
        OBJ,
        XYZ,
        UNKNOWN
    };
    
    // 导入文件
    static std::shared_ptr<Model> importFile(const QString& filePath);
    
    // 导出文件
    static bool exportFile(std::shared_ptr<Model> model, const QString& filePath);
    
private:
    // 文件格式检测
    static FileFormat detectFormat(const QString& filePath);
    
    // 具体格式的导入器
    static std::shared_ptr<Model> importPLY(const QString& filePath);
    static std::shared_ptr<Model> importOBJ(const QString& filePath);
    static std::shared_ptr<Model> importXYZ(const QString& filePath);
    
    // 具体格式的导出器
    static bool exportPLY(std::shared_ptr<Model> model, const QString& filePath);
    static bool exportOBJ(std::shared_ptr<Model> model, const QString& filePath);
    static bool exportXYZ(std::shared_ptr<Model> model, const QString& filePath);
};