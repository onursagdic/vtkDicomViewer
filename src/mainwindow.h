#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QFileDialog>
#include <QMessageBox>
#include <QLabel>
#include "dicomviewer.h"

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void on_actionOpen_triggered();
    void on_actionExit_triggered();
    void on_actionMeasurementTool_triggered();
    void on_actionZoomIn_triggered();
    void on_actionZoomOut_triggered();
    void on_actionResetView_triggered();
    void updateStatusBar(const QString &message);

private:
    void setupStatusBar();
    void createActions();
    void createMenus();
    void createToolbar();

    Ui::MainWindow *ui;
    DicomViewer *dicomViewer;
    QLabel *statusLabel;
};
#endif // MAINWINDOW_H