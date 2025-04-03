#include "mainwindow.h"
#include <QApplication>
#include <QSurfaceFormat>
#include <QVTKOpenGLNativeWidget.h>

int main(int argc, char *argv[])
{
    // Qt High DPI ayarları
    QCoreApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
    
    // QVTKOpenGLNativeWidget için yüzey formatını ayarla
    QSurfaceFormat::setDefaultFormat(QVTKOpenGLNativeWidget::defaultFormat());
    
    QApplication app(argc, argv);
    app.setApplicationName("DICOM Viewer");
    app.setOrganizationName("Akgun");
    
    MainWindow mainWindow;
    mainWindow.resize(1024, 768);
    mainWindow.show();
    
    return app.exec();
}