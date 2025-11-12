#include "MainWindow.h"
#include "OpenGLWidget.h"
#include "PointCloud.h"
#include "Mesh.h"
#include "FileImporter.h"
#include "ModelAnalyzer.h"
#include <QMenuBar>
#include <QToolBar>
#include <QDockWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QPushButton>
#include <QSlider>
#include <QLabel>
#include <QListWidget>
#include <QGroupBox>
#include <QTextEdit>
#include <QFileDialog>
#include <QMessageBox>
#include <QCheckBox>
#include <QDoubleSpinBox>
#include <QDebug>
#include <QComboBox>
#include <cmath>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent), currentModelIndex_(-1), unitScaleForImport_(1.0) {
    
    setupUI();
    createMenuBar();
    createToolBars();
    createDockWindows();
    
    // 设置定时器用于更新模型信息
    updateTimer_ = new QTimer(this);
    connect(updateTimer_, &QTimer::timeout, this, &MainWindow::updateModelInfo);
    updateTimer_->start(100); // 100ms更新一次
    
    resize(1200, 800);
    setWindowTitle("3D Data Visualization System");
}

MainWindow::~MainWindow() {
}

void MainWindow::setupUI() {
    // 创建中央OpenGL窗口
    openGLWidget_ = new OpenGLWidget(this);
    setCentralWidget(openGLWidget_);
}

void MainWindow::createMenuBar() {
    QMenuBar* menuBar = this->menuBar();
    
    // 文件菜单
    QMenu* fileMenu = menuBar->addMenu("文件(&F)");
    fileMenu->addAction("导入模型", this, &MainWindow::onImportModel);
    fileMenu->addAction("导出模型", this, &MainWindow::onExportModel);
    fileMenu->addSeparator();
    fileMenu->addAction("退出", this, &QWidget::close);
    
    // 视图菜单
    QMenu* viewMenu = menuBar->addMenu("视图(&V)");
    viewMenu->addAction("重置相机", [this]() { openGLWidget_->resetCamera(); });
    viewMenu->addAction("清除所有模型", [this]() { 
        models_.clear(); 
        openGLWidget_->clearModels(); 
        updateModelList(); 
    });
    
    // 帮助菜单
    QMenu* helpMenu = menuBar->addMenu("帮助(&H)");
    helpMenu->addAction("关于", [this]() {
        QMessageBox::about(this, "关于", "3D Data Visualization System\n\n"
                          "一个功能完整的三维数据管理与可视化系统。\n"
                          "支持点云和网格模型的导入、分析和可视化。");
    });
}

void MainWindow::createToolBars() {
    QToolBar* fileToolBar = addToolBar("文件");
    fileToolBar->addAction("导入", this, &MainWindow::onImportModel);
    fileToolBar->addAction("导出", this, &MainWindow::onExportModel);
    fileToolBar->addSeparator();
    fileToolBar->addAction("删除", this, &MainWindow::onDeleteModel);
}

void MainWindow::createDockWindows() {
    // 模型列表面板
    QDockWidget* modelDock = new QDockWidget("模型列表", this);
    QWidget* modelWidget = new QWidget();
    QVBoxLayout* modelLayout = new QVBoxLayout(modelWidget);
    
    modelListWidget_ = new QListWidget();
    connect(modelListWidget_, &QListWidget::currentRowChanged, this, &MainWindow::onModelSelectionChanged);
    
    QPushButton* addPointCloudBtn = new QPushButton("添加测试点云");
    QPushButton* addMeshBtn = new QPushButton("添加测试网格");
    
    connect(addPointCloudBtn, &QPushButton::clicked, [this]() {
        auto pointCloud = std::make_shared<PointCloud>("测试点云");
        // 添加一些测试点
        for (int i = 0; i < 100; ++i) {
            float x = (rand() % 200 - 100) / 10.0f;
            float y = (rand() % 200 - 100) / 10.0f;
            float z = (rand() % 200 - 100) / 10.0f;
            pointCloud->addPoint(QVector3D(x, y, z), QColor(rand() % 256, rand() % 256, rand() % 256));
        }
        models_.push_back(pointCloud);
        openGLWidget_->addModel(pointCloud);
        updateModelList();
    });
    
    connect(addMeshBtn, &QPushButton::clicked, [this]() {
        auto mesh = std::make_shared<Mesh>("测试网格");
        // 添加一个简单的立方体
        QVector3D vertices[] = {
            QVector3D(-1, -1, -1), QVector3D(1, -1, -1), QVector3D(1, 1, -1), QVector3D(-1, 1, -1),
            QVector3D(-1, -1, 1), QVector3D(1, -1, 1), QVector3D(1, 1, 1), QVector3D(-1, 1, 1)
        };
        
        for (int i = 0; i < 8; ++i) {
            mesh->addVertex(vertices[i], QVector3D(0, 0, 1), Qt::blue);
        }
        
        // 添加三角形面片（简化版本）
        int faces[][3] = {
            {0, 1, 2}, {2, 3, 0}, // 前面
            {4, 7, 6}, {6, 5, 4}, // 后面
            {0, 4, 5}, {5, 1, 0}, // 底面
            {2, 6, 7}, {7, 3, 2}, // 顶面
            {0, 3, 7}, {7, 4, 0}, // 左面
            {1, 5, 6}, {6, 2, 1}  // 右面
        };
        
        for (int i = 0; i < 12; ++i) {
            mesh->addTriangle(faces[i][0], faces[i][1], faces[i][2]);
        }
        
        models_.push_back(mesh);
        openGLWidget_->addModel(mesh);
        updateModelList();
    });
    
    modelLayout->addWidget(modelListWidget_);
    modelLayout->addWidget(addPointCloudBtn);
    modelLayout->addWidget(addMeshBtn);
    modelDock->setWidget(modelWidget);
    addDockWidget(Qt::LeftDockWidgetArea, modelDock);
    
    // 属性面板
    QDockWidget* propertyDock = new QDockWidget("属性面板", this);
    QWidget* propertyWidget = new QWidget();
    QVBoxLayout* propertyLayout = new QVBoxLayout(propertyWidget);
    
    // 基本信息
    analysisGroup_ = new QGroupBox("模型信息");
    QVBoxLayout* infoLayout = new QVBoxLayout(analysisGroup_);
    vertexCountLabel_ = new QLabel("顶点数: 0");
    triangleCountLabel_ = new QLabel("三角形数: 0");
    centerLabel_ = new QLabel("重心: (0, 0, 0)");
    surfaceAreaLabel_ = new QLabel("表面积: 0.0");
    infoLayout->addWidget(vertexCountLabel_);
    infoLayout->addWidget(triangleCountLabel_);
    infoLayout->addWidget(centerLabel_);
    infoLayout->addWidget(surfaceAreaLabel_);
    
    // 位置控制
    transformGroup_ = new QGroupBox("位置控制");
    QVBoxLayout* transformLayout = new QVBoxLayout(transformGroup_);
    
    QGridLayout* posGrid = new QGridLayout();
    posXSpinBox_ = new QDoubleSpinBox();
    posYSpinBox_ = new QDoubleSpinBox();
    posZSpinBox_ = new QDoubleSpinBox();
    
    // 设置范围和步长
    posXSpinBox_->setRange(-100.0, 100.0);
    posYSpinBox_->setRange(-100.0, 100.0);
    posZSpinBox_->setRange(-100.0, 100.0);
    posXSpinBox_->setSingleStep(0.1);
    posYSpinBox_->setSingleStep(0.1);
    posZSpinBox_->setSingleStep(0.1);
    posXSpinBox_->setDecimals(2);
    posYSpinBox_->setDecimals(2);
    posZSpinBox_->setDecimals(2);
    
    connect(posXSpinBox_, QOverload<double>::of(&QDoubleSpinBox::valueChanged), 
            this, &MainWindow::onPositionChanged);
    connect(posYSpinBox_, QOverload<double>::of(&QDoubleSpinBox::valueChanged), 
            this, &MainWindow::onPositionChanged);
    connect(posZSpinBox_, QOverload<double>::of(&QDoubleSpinBox::valueChanged), 
            this, &MainWindow::onPositionChanged);
    
    posGrid->addWidget(new QLabel("X:"), 0, 0);
    posGrid->addWidget(posXSpinBox_, 0, 1);
    posGrid->addWidget(new QLabel("Y:"), 1, 0);
    posGrid->addWidget(posYSpinBox_, 1, 1);
    posGrid->addWidget(new QLabel("Z:"), 2, 0);
    posGrid->addWidget(posZSpinBox_, 2, 1);
    
    resetPosButton_ = new QPushButton("重置位置");
    connect(resetPosButton_, &QPushButton::clicked, this, &MainWindow::onResetPosition);
    
    transformLayout->addLayout(posGrid);
    transformLayout->addWidget(resetPosButton_);
    
    // 颜色控制
    colorGroup_ = new QGroupBox("颜色控制");
    QVBoxLayout* colorLayout = new QVBoxLayout(colorGroup_);
    
    QHBoxLayout* rgbLayout = new QHBoxLayout();
    colorSliderR_ = new QSlider(Qt::Horizontal);
    colorSliderG_ = new QSlider(Qt::Horizontal);
    colorSliderB_ = new QSlider(Qt::Horizontal);
    colorSliderR_->setRange(0, 255);
    colorSliderG_->setRange(0, 255);
    colorSliderB_->setRange(0, 255);
    colorSliderR_->setValue(255);
    colorSliderG_->setValue(255);
    colorSliderB_->setValue(255);
    
    connect(colorSliderR_, &QSlider::valueChanged, this, &MainWindow::onColorChanged);
    connect(colorSliderG_, &QSlider::valueChanged, this, &MainWindow::onColorChanged);
    connect(colorSliderB_, &QSlider::valueChanged, this, &MainWindow::onColorChanged);
    
    rgbLayout->addWidget(new QLabel("R:"));
    rgbLayout->addWidget(colorSliderR_);
    rgbLayout->addWidget(new QLabel("G:"));
    rgbLayout->addWidget(colorSliderG_);
    rgbLayout->addWidget(new QLabel("B:"));
    rgbLayout->addWidget(colorSliderB_);
    
    QCheckBox* pseudoColorCheck = new QCheckBox("伪彩色渲染");
    connect(pseudoColorCheck, &QCheckBox::toggled, this, &MainWindow::onPseudoColorToggled);
    
    QHBoxLayout* coordLayout = new QHBoxLayout();
    coordinateSlider_ = new QSlider(Qt::Horizontal);
    coordinateSlider_->setRange(0, 2);
    coordinateSlider_->setValue(0);
    connect(coordinateSlider_, &QSlider::valueChanged, this, &MainWindow::onCoordinateChanged);
    
    coordLayout->addWidget(new QLabel("坐标轴:"));
    coordLayout->addWidget(coordinateSlider_);

    // （移除 尺度S/自动建议S 控件与逻辑）

    // 色图选择
    QHBoxLayout* cmapLayout = new QHBoxLayout();
    colorMapCombo_ = new QComboBox();
    colorMapCombo_->addItem("Rainbow", 0);
    colorMapCombo_->addItem("Viridis", 1);
    colorMapCombo_->addItem("Red-Blue", 2);
    colorMapCombo_->setCurrentIndex(0);
    connect(colorMapCombo_, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &MainWindow::onColorMapChanged);
    cmapLayout->addWidget(new QLabel("色图:"));
    cmapLayout->addWidget(colorMapCombo_);
    
    colorLayout->addLayout(rgbLayout);
    colorLayout->addWidget(pseudoColorCheck);
    colorLayout->addLayout(coordLayout);
    colorLayout->addLayout(cmapLayout);
    // 导入单位选择（影响几何缩放，内部统一为米）
    QHBoxLayout* unitLayout = new QHBoxLayout();
    unitCombo_ = new QComboBox();
    unitCombo_->addItem("米 (m)", 1.0);
    unitCombo_->addItem("厘米 (cm)", 0.01);
    unitCombo_->addItem("毫米 (mm)", 0.001);
    unitCombo_->setCurrentIndex(0);
    connect(unitCombo_, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &MainWindow::onImportUnitChanged);
    unitLayout->addWidget(new QLabel("导入单位:"));
    unitLayout->addWidget(unitCombo_);
    colorLayout->addLayout(unitLayout);
    
    propertyLayout->addWidget(analysisGroup_);
    propertyLayout->addWidget(transformGroup_);
    propertyLayout->addWidget(colorGroup_);
    propertyLayout->addStretch();
    
    propertyDock->setWidget(propertyWidget);
    addDockWidget(Qt::RightDockWidgetArea, propertyDock);
}

void MainWindow::onImportModel() {
    QString fileName = QFileDialog::getOpenFileName(this, "导入模型", "", 
                                                   "所有支持的格式 (*.ply *.obj *.xyz);;PLY文件 (*.ply);;OBJ文件 (*.obj);;XYZ文件 (*.xyz)");
    if (fileName.isEmpty()) return;
    
    // 使用文件导入器导入模型
    auto model = FileImporter::importFile(fileName);
    if (model) {
        models_.push_back(model);
        if (unitScaleForImport_ != 1.0) {
            model->scale(QVector3D(unitScaleForImport_, unitScaleForImport_, unitScaleForImport_));
        }
        openGLWidget_->addModel(model);
        updateModelList();
        // 若用户未手动修改尺度，自动按当前轴范围设置 S
        // 移除自动建议 S 逻辑：不再调整伪彩色尺度
        
        QMessageBox::information(this, "导入成功", 
                                QString("成功导入模型: %1\n类型: %2\n顶点数: %3")
                                .arg(model->getName())
                                .arg(model->getType())
                                .arg(model->getVertexCount()));
    } else {
        QMessageBox::warning(this, "导入失败", "无法导入选定的文件，请检查文件格式是否正确。");
    }
}

void MainWindow::onExportModel() {
    if (currentModelIndex_ < 0 || currentModelIndex_ >= models_.size()) {
        QMessageBox::warning(this, "警告", "请先选择一个模型");
        return;
    }
    QString fileName = QFileDialog::getSaveFileName(this, "导出模型", "", 
                                                   "PLY文件 (*.ply);;OBJ文件 (*.obj);;XYZ文件 (*.xyz)");
    if (fileName.isEmpty()) return;
    
    auto model = models_[currentModelIndex_];
    if (FileImporter::exportFile(model, fileName)) {
        QMessageBox::information(this, "导出成功", 
                                QString("模型已成功导出到: %1").arg(fileName));
    } else {
        QMessageBox::warning(this, "导出失败", "无法导出模型到指定文件。");
    }
}

void MainWindow::onDeleteModel() {
    if (currentModelIndex_ < 0 || currentModelIndex_ >= models_.size()) {
        QMessageBox::warning(this, "警告", "请先选择一个模型");
        return;
    }
    
    auto model = models_[currentModelIndex_];
    models_.erase(models_.begin() + currentModelIndex_);
    openGLWidget_->removeModel(model);
    updateModelList();
    
    currentModelIndex_ = -1;
    updatePropertyPanel();
}

void MainWindow::onModelSelectionChanged() {
    currentModelIndex_ = modelListWidget_->currentRow();
    openGLWidget_->setSelectedModelIndex(currentModelIndex_);
    updatePropertyPanel();
}

void MainWindow::onColorChanged() {
    if (currentModelIndex_ < 0 || currentModelIndex_ >= models_.size()) return;
    
    int r = colorSliderR_->value();
    int g = colorSliderG_->value();
    int b = colorSliderB_->value();
    
    auto model = models_[currentModelIndex_];
    model->setColor(QColor(r, g, b));
    
    openGLWidget_->update();
}

void MainWindow::onPseudoColorToggled(bool checked) {
    openGLWidget_->setPseudoColorEnabled(checked);
}

void MainWindow::onCoordinateChanged(int value) {
    openGLWidget_->setCoordinateAxis(value);
}

// （移除 onScaleChanged / onAutoSuggestScale 槽函数）

void MainWindow::onColorMapChanged(int index) {
    int mode = colorMapCombo_->itemData(index).toInt();
    openGLWidget_->setColorMapMode(mode);
}

void MainWindow::onImportUnitChanged(int index) {
    if (!unitCombo_) return;
    unitScaleForImport_ = unitCombo_->itemData(index).toDouble();
}

void MainWindow::updateModelInfo() {
    if (currentModelIndex_ >= 0 && currentModelIndex_ < models_.size()) {
        auto model = models_[currentModelIndex_];
        
        vertexCountLabel_->setText(QString("顶点数: %1").arg(model->getVertexCount()));
        triangleCountLabel_->setText(QString("三角形数: %1").arg(model->getTriangleCount()));
        
    QVector3D center = model->computeCenter();
    centerLabel_->setText(QString("重心(cm): (%1, %2, %3)")
                 .arg(center.x(), 0, 'f', 2)
                 .arg(center.y(), 0, 'f', 2)
                 .arg(center.z(), 0, 'f', 2));
        
        if (model->getType() == "Mesh") {
            auto mesh = std::dynamic_pointer_cast<Mesh>(model);
            float areaCm2 = mesh->computeSurfaceArea();
            surfaceAreaLabel_->setText(QString("表面积: %1 cm^2").arg(areaCm2, 0, 'f', 2));
        } else {
            surfaceAreaLabel_->setText("表面积: N/A");
        }
    }
}

void MainWindow::updateModelList() {
    modelListWidget_->clear();
    
    for (size_t i = 0; i < models_.size(); ++i) {
        const auto& model = models_[i];
        QString itemText = QString("%1: %2 (顶点:%3)")
                          .arg(i + 1)
                          .arg(model->getName())
                          .arg(model->getVertexCount());
        modelListWidget_->addItem(itemText);
    }
}

void MainWindow::updatePropertyPanel() {
    if (currentModelIndex_ >= 0 && currentModelIndex_ < models_.size()) {
        auto model = models_[currentModelIndex_];
        
        // 更新颜色
        QColor color = model->getColor();
        colorSliderR_->setValue(color.red());
        colorSliderG_->setValue(color.green());
        colorSliderB_->setValue(color.blue());
        
        // 更新位置
        QVector3D center = model->computeCenter();
        posXSpinBox_->blockSignals(true);
        posYSpinBox_->blockSignals(true);
        posZSpinBox_->blockSignals(true);
        
        posXSpinBox_->setValue(center.x());
        posYSpinBox_->setValue(center.y());
        posZSpinBox_->setValue(center.z());
        
        posXSpinBox_->blockSignals(false);
        posYSpinBox_->blockSignals(false);
        posZSpinBox_->blockSignals(false);
        
        analysisGroup_->setEnabled(true);
        transformGroup_->setEnabled(true);
        colorGroup_->setEnabled(true);
        if (model->getType() == "Mesh") {
            triangleCountLabel_->setText(QString("三角形数: %1").arg(model->getTriangleCount()));
        } else {
            triangleCountLabel_->setText("三角形数: N/A");
        }

        // 同步尺度显示（不触发信号）
        // 尺度控件逻辑废弃，不再更新
    } else {
        analysisGroup_->setEnabled(false);
        transformGroup_->setEnabled(false);
        colorGroup_->setEnabled(false);
        
    vertexCountLabel_->setText("顶点数: 0");
    triangleCountLabel_->setText("三角形数: N/A");
        centerLabel_->setText("重心(cm): (0, 0, 0)");
        surfaceAreaLabel_->setText("表面积: 0.0 cm^2");
        // 尺度控件逻辑废弃，不再更新
    }
}

void MainWindow::onTransformChanged() {
    // 变换改变的处理
    openGLWidget_->update();
}

void MainWindow::onPositionChanged() {
    if (currentModelIndex_ < 0 || currentModelIndex_ >= models_.size()) return;
    
    auto model = models_[currentModelIndex_];
    QVector3D newPosition(posXSpinBox_->value(), 
                         posYSpinBox_->value(), 
                         posZSpinBox_->value());
    
    // 计算当前位置和新位置的差值
    QVector3D currentCenter = model->computeCenter(); // cm
    QVector3D offset = newPosition - currentCenter;   // cm
    
    // 应用厘米制平移
    model->translateCm(offset);
    
    openGLWidget_->update();
}

void MainWindow::onResetPosition() {
    if (currentModelIndex_ < 0 || currentModelIndex_ >= models_.size()) return;
    
    auto model = models_[currentModelIndex_];
    QVector3D currentCenter = model->computeCenter(); // cm
    
    // 移动到原点（厘米）
    model->translateCm(-currentCenter);
    
    // 更新UI
    posXSpinBox_->blockSignals(true);
    posYSpinBox_->blockSignals(true);
    posZSpinBox_->blockSignals(true);
    
    posXSpinBox_->setValue(0.0);
    posYSpinBox_->setValue(0.0);
    posZSpinBox_->setValue(0.0);
    
    posXSpinBox_->blockSignals(false);
    posYSpinBox_->blockSignals(false);
    posZSpinBox_->blockSignals(false);
    
    openGLWidget_->update();
}