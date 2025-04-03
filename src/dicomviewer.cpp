#include "dicomviewer.h"
#include <QDebug>
#include <vtkImageData.h>
#include <vtkRenderWindow.h>
#include <vtkImageViewer2.h>

DicomViewer::DicomViewer(QWidget *parent) 
    : QWidget(parent), 
      measurementModeActive(false),
      isImageLoaded(false)
{
    // Layout oluşturma
    layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    
    // VTK widget kurulumu
    setupVtkWidget();
    
    // Ölçüm aracı oluşturma
    measurementWidget = MeasurementWidget::New();
    
    // Sinyalleri ve slotları bağlama
    connectSignalsSlots();
    
    emit statusChanged(tr("Ready. Open a DICOM file to start."));
}

void DicomViewer::connectSignalsSlots()
{
    // Eğer herhangi bir sinyal-slot bağlantısı varsa buraya ekleyebilirsiniz
    // Örneğin:
    // connect(...);
}

DicomViewer::~DicomViewer()
{
    // VTK nesnelerini manuel olarak temizleme
    if (imageViewer) {
        imageViewer->Delete();
        imageViewer = nullptr;
    }
    
    if (renderWindowInteractor) {
        renderWindowInteractor->Delete();
        renderWindowInteractor = nullptr;
    }
    
    if (imageStyle) {
        imageStyle->Delete();
        imageStyle = nullptr;
    }
    
    if (vtkWidget) {
        vtkWidget->renderWindow()->Finalize();
    }
}
void DicomViewer::setupVtkWidget()
{
    // VTK widget oluşturma
    vtkWidget = new QVTKOpenGLNativeWidget(this);
    layout->addWidget(vtkWidget);
    
    // DICOM okuyucu oluşturma
    dicomReader = vtkSmartPointer<vtkDICOMImageReader>::New();
    
    // Görüntüleyici oluşturma (statik New() metodunu kullanarak)
    imageViewer = vtkImageViewer2::New();
    
    try {
        // Render window ayarı
        imageViewer->SetRenderWindow(vtkWidget->renderWindow());
        
        // Render window interactor ayarları
        renderWindowInteractor = vtkRenderWindowInteractor::New();
        imageViewer->SetupInteractor(renderWindowInteractor);
        
        // İnteraktör stil ayarlama
        imageStyle = vtkInteractorStyleImage::New();
        renderWindowInteractor->SetInteractorStyle(imageStyle);
        
        // Başlangıç render
        imageViewer->GetRenderer()->SetBackground(0.2, 0.2, 0.2);
        imageViewer->GetRenderer()->ResetCamera();
        imageViewer->Render();
    }
    catch (const std::exception& e) {
        qDebug() << "VTK setup error:" << e.what();
    }
}

bool DicomViewer::loadDicomFile(const QString &fileName)
{
    if (fileName.isEmpty()) {
        emit statusChanged(tr("No file selected."));
        return false;
    }
    
    try {
        // DICOM dosyasını oku
        emit statusChanged(tr("Loading DICOM file: %1").arg(fileName));
        
        dicomReader->SetFileName(fileName.toUtf8().constData());
        dicomReader->Update();
        
        // Görüntüleme parametrelerini ayarla
        imageViewer->SetInputConnection(dicomReader->GetOutputPort());
        imageViewer->SetColorLevel(128);
        imageViewer->SetColorWindow(256);
        
        // Slice ayarı
        int maxSlice = dicomReader->GetOutput()->GetDimensions()[2] - 1;
        if (maxSlice < 1) maxSlice = 0;
        imageViewer->SetSliceOrientationToXY();
        imageViewer->SetSlice(maxSlice > 0 ? maxSlice / 2 : 0);
        
        // Kamerayı sıfırla ve render et
        imageViewer->GetRenderer()->ResetCamera();
        imageViewer->Render();
        
        // Ölçüm aracını görüntüleyiciye bağla
        measurementWidget->SetViewer(imageViewer);
        
        isImageLoaded = true;
        emit statusChanged(tr("DICOM file loaded: %1").arg(fileName));
        
        return true;
    }
    catch (const std::exception &e) {
        qDebug() << "DICOM loading error:" << e.what();
        emit statusChanged(tr("Error loading DICOM file: %1").arg(e.what()));
        return false;
    }
}

// Diğer metodlar önceki versiyondaki gibi kalacak
void DicomViewer::toggleMeasurementMode()
{
    if (!isImageLoaded) {
        emit statusChanged(tr("Please load a DICOM file first."));
        return;
    }
    
    measurementModeActive = !measurementModeActive;
    
    if (measurementModeActive) {
        // Ölçüm modunu aktifleştir
        renderWindowInteractor->SetInteractorStyle(measurementWidget);
        emit statusChanged(tr("Measurement mode active. Click and drag to measure."));
    }
    else {
        // Normal mod
        renderWindowInteractor->SetInteractorStyle(imageStyle);
        emit statusChanged(tr("Measurement mode deactivated."));
    }
    
    // Yeniden render et
    vtkWidget->renderWindow()->Render();
}

void DicomViewer::zoomIn()
{
    if (!isImageLoaded) {
        emit statusChanged(tr("Please load a DICOM file first."));
        return;
    }
    
    // Kamera zoom
    vtkCamera *camera = imageViewer->GetRenderer()->GetActiveCamera();
    camera->Zoom(1.1);
    vtkWidget->renderWindow()->Render();
    
    emit statusChanged(tr("Zoomed in."));
}

void DicomViewer::zoomOut()
{
    if (!isImageLoaded) {
        emit statusChanged(tr("Please load a DICOM file first."));
        return;
    }
    
    // Kamera zoom out
    vtkCamera *camera = imageViewer->GetRenderer()->GetActiveCamera();
    camera->Zoom(0.9);
    vtkWidget->renderWindow()->Render();
    
    emit statusChanged(tr("Zoomed out."));
}

void DicomViewer::resetView()
{
    if (!isImageLoaded) {
        emit statusChanged(tr("Please load a DICOM file first."));
        return;
    }
    
    // Görünümü sıfırla
    imageViewer->GetRenderer()->ResetCamera();
    vtkWidget->renderWindow()->Render();
    
    emit statusChanged(tr("View reset to default."));
}