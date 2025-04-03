#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    
    // DICOM görüntüleyiciyi ekleyelim
    dicomViewer = new DicomViewer(this);
    setCentralWidget(dicomViewer);
    
    // Sinyal-slot bağlantıları
    connect(dicomViewer, &DicomViewer::statusChanged, this, &MainWindow::updateStatusBar);
    
    // UI bileşenlerini kur
    setupStatusBar();
    createActions();
    createMenus();
    createToolbar();
    
    // Pencere başlığı
    setWindowTitle("DICOM Viewer - Akgun");
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::setupStatusBar()
{
    statusLabel = new QLabel(this);
    statusLabel->setText(tr("Ready"));
    statusBar()->addWidget(statusLabel, 1);
}

void MainWindow::createActions()
{
    // Butonlara ikonlar atama (icons klasörü oluşturursanız)
    // ui->actionOpen->setIcon(QIcon(":/icons/open.png"));
    // ui->actionMeasurementTool->setIcon(QIcon(":/icons/measure.png"));
    // ui->actionZoomIn->setIcon(QIcon(":/icons/zoom-in.png"));
    // ui->actionZoomOut->setIcon(QIcon(":/icons/zoom-out.png"));
    // ui->actionResetView->setIcon(QIcon(":/icons/reset.png"));
}

void MainWindow::createMenus()
{
    // ui_mainwindow.ui dosyasında tanımlanmış olmalı
}

void MainWindow::createToolbar()
{
    // ui_mainwindow.ui dosyasında tanımlanmış olmalı
}

void MainWindow::on_actionOpen_triggered()
{
    QString fileName = QFileDialog::getOpenFileName(this, 
        tr("Open DICOM File"), "", tr("DICOM Files (*.dcm *.DCM);;All Files (*)"));
    
    if (!fileName.isEmpty()) {
        if (!dicomViewer->loadDicomFile(fileName)) {
            QMessageBox::warning(this, tr("Error"), tr("Failed to load DICOM file."));
        }
    }
}

void MainWindow::on_actionExit_triggered()
{
    QApplication::quit();
}

void MainWindow::on_actionMeasurementTool_triggered()
{
    dicomViewer->toggleMeasurementMode();
    
    if (ui->actionMeasurementTool->isChecked()) {
        statusLabel->setText(tr("Measurement mode: ON - Click and drag to measure"));
    } else {
        statusLabel->setText(tr("Measurement mode: OFF"));
    }
}

void MainWindow::on_actionZoomIn_triggered()
{
    dicomViewer->zoomIn();
}

void MainWindow::on_actionZoomOut_triggered()
{
    dicomViewer->zoomOut();
}

void MainWindow::on_actionResetView_triggered()
{
    dicomViewer->resetView();
}

void MainWindow::updateStatusBar(const QString &message)
{
    statusLabel->setText(message);
}