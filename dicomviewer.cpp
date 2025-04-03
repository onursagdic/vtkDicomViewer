#include "dicomviewer.h"
#include "measurementwidget.h" // MeasurementWidget sınıfı için gerekli

#include <QDebug>
#include <vtkImageData.h>
#include <vtkRenderWindow.h>
#include <vtkImageViewer2.h>
#include <vtkRenderer.h>
#include <vtkCamera.h>

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
    measurementWidget = MeasurementWidget::New(); // vtkNew veya vtkSmartPointer kullanmak daha güvenli olabilir
    
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
    
    if (measurementWidget) {
        measurementWidget->Delete();
        measurementWidget = nullptr;
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
    
    try {
        // Geçici bir görüntü verisi oluştur - başlangıç düzgün bir pipeline için
        vtkSmartPointer<vtkImageData> emptyImage = vtkSmartPointer<vtkImageData>::New();
        emptyImage->SetDimensions(10, 10, 1);
        emptyImage->AllocateScalars(VTK_UNSIGNED_CHAR, 1);
        
        // Tüm pikselleri siyah yap
        unsigned char* ptr = static_cast<unsigned char*>(emptyImage->GetScalarPointer());
        memset(ptr, 0, 10 * 10 * sizeof(unsigned char));
        
        // Görüntüleyici oluşturma
        imageViewer = vtkImageViewer2::New();
        imageViewer->SetInputData(emptyImage);  // Geçici veri ile başlat
        
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
        
        // Dosya okuma ve hata denetimi
        dicomReader->SetFileName(fileName.toUtf8().constData());
        dicomReader->Update();
        
        if (!dicomReader->GetOutput()) {
            emit statusChanged(tr("Failed to read DICOM file or empty data."));
            return false;
        }
        
        // Görüntüleme parametrelerini ayarla
        imageViewer->SetInputConnection(dicomReader->GetOutputPort());
        
        // Renk penceresi ve seviyesini ayarla
        double range[2];
        dicomReader->GetOutput()->GetScalarRange(range);
        double window = range[1] - range[0];
        double level = (range[1] + range[0]) / 2.0;
        
        imageViewer->SetColorWindow(window);
        imageViewer->SetColorLevel(level);
        
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