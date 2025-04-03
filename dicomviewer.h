#ifndef DICOMVIEWER_H
#define DICOMVIEWER_H
#include <vtkImageData.h>
#include <cstring> // memset için
#include <QWidget>
#include <QVBoxLayout>
#include <QVTKOpenGLNativeWidget.h>

#include <vtkSmartPointer.h>
#include <vtkDICOMImageReader.h>
#include <vtkImageViewer2.h>
#include <vtkRenderWindowInteractor.h>
#include <vtkInteractorStyleImage.h>
#include <vtkRenderer.h>
#include <vtkCamera.h>

// İleri (forward) tanımlama
class MeasurementWidget;

class DicomViewer : public QWidget
{
    Q_OBJECT

public:
    explicit DicomViewer(QWidget *parent = nullptr);
    ~DicomViewer();

    bool loadDicomFile(const QString &fileName);

public slots:
    void toggleMeasurementMode();
    void zoomIn();
    void zoomOut();
    void resetView();

signals:
    void statusChanged(const QString &message);

private:
    void setupVtkWidget();
    void connectSignalsSlots();

    // Qt widgets
    QVBoxLayout *layout;
    QVTKOpenGLNativeWidget *vtkWidget;

    // VTK objects
    vtkSmartPointer<vtkDICOMImageReader> dicomReader;
    vtkImageViewer2* imageViewer;
    vtkRenderWindowInteractor* renderWindowInteractor;
    vtkInteractorStyleImage* imageStyle;

    // Measurement widget
    MeasurementWidget* measurementWidget;

    // State flags
    bool measurementModeActive;
    bool isImageLoaded;
};

#endif // DICOMVIEWER_H