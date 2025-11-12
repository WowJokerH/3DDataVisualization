#include <QApplication>
#include "MainWindow.h"

int main(int argc, char *argv[]) {
    QApplication app(argc, argv);
    
    // 设置应用程序信息
    app.setApplicationName("3D Data Visualization System");
    app.setOrganizationName("3D Visualization Lab");
    
    // 创建并显示主窗口
    MainWindow window;
    window.show();
    
    return app.exec();
}