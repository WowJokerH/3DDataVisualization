#pragma once

#include <QMainWindow>
#include <QTimer>
#include <memory>
#include <vector>

QT_BEGIN_NAMESPACE
class QSlider;
class QLabel;
class QListWidget;
class QPushButton;
class QGroupBox;
class QTextEdit;
class QDoubleSpinBox;
class QComboBox;
QT_END_NAMESPACE

class OpenGLWidget;
class Model;

class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void onImportModel();
    void onExportModel();
    void onDeleteModel();
    void onModelSelectionChanged();
    void onColorChanged();
    void onTransformChanged();
    void onPositionChanged();
    void onResetPosition();
    void updateModelInfo();
    void onPseudoColorToggled(bool checked);
    void onCoordinateChanged(int value);
    void onColorMapChanged(int index);
    void onImportUnitChanged(int index);

private:
    void setupUI();
    void createMenuBar();
    void createToolBars();
    void createDockWindows();
    void updateModelList();
    void updatePropertyPanel();
    
    // UI组件
    OpenGLWidget* openGLWidget_;
    QListWidget* modelListWidget_;
    QLabel* vertexCountLabel_;
    QLabel* triangleCountLabel_;
    QLabel* centerLabel_;
    QLabel* surfaceAreaLabel_;
    QTextEdit* infoTextEdit_;
    
    QSlider* colorSliderR_;
    QSlider* colorSliderG_;
    QSlider* colorSliderB_;
    QSlider* coordinateSlider_;
    QComboBox* colorMapCombo_;
    QComboBox* unitCombo_;
    
    // 位置控制
    QDoubleSpinBox* posXSpinBox_;
    QDoubleSpinBox* posYSpinBox_;
    QDoubleSpinBox* posZSpinBox_;
    QPushButton* resetPosButton_;
    
    QPushButton* importButton_;
    QPushButton* exportButton_;
    QPushButton* deleteButton_;
    
    QGroupBox* transformGroup_;
    QGroupBox* colorGroup_;
    QGroupBox* analysisGroup_;
    
    // 数据
    std::vector<std::shared_ptr<Model>> models_;
    int currentModelIndex_;
    double unitScaleForImport_; // 将导入数据转换为“米”的缩放因子（mm=0.001, cm=0.01, m=1）
    
    QTimer* updateTimer_;
};