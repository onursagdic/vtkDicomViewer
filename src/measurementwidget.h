#ifndef MEASUREMENTWIDGET_H
#define MEASUREMENTWIDGET_H

#include <vtkInteractorStyleImage.h>
#include <vtkSmartPointer.h>
#include <vtkImageViewer2.h>
#include <vtkLineSource.h>
#include <vtkPolyDataMapper.h>
#include <vtkActor.h>
#include <vtkTextActor.h>
#include <vtkRenderWindow.h>
#include <vtkRenderer.h>
#include <vtkProperty.h>
#include <vtkTextProperty.h>
#include <vtkRenderWindowInteractor.h>

class MeasurementWidget : public vtkInteractorStyleImage
{
public:
    static MeasurementWidget* New();
    vtkTypeMacro(MeasurementWidget, vtkInteractorStyleImage);

    // Metodlar
    void SetViewer(vtkSmartPointer<vtkImageViewer2> viewer);

    // VTK event handle metodları
    virtual void OnLeftButtonDown() override;
    virtual void OnMouseMove() override;
    virtual void OnLeftButtonUp() override;

protected:
    MeasurementWidget();
    ~MeasurementWidget();

private:
    // Ölçüm metodları
    void StartMeasurement(int x, int y);
    void UpdateMeasurement(int x, int y);
    void EndMeasurement();
    void UpdateDistance();
    double CalculateDistance(int x1, int y1, int x2, int y2);
    void DisplayToWorld(double x, double y, double worldPos[3]);
    void WorldToDisplay(double x, double y, double z, double displayPos[2]);

    // VTK nesneleri
    vtkSmartPointer<vtkImageViewer2> Viewer;
    vtkSmartPointer<vtkLineSource> LineSource;
    vtkSmartPointer<vtkPolyDataMapper> LineMapper;
    vtkSmartPointer<vtkActor> LineActor;
    vtkSmartPointer<vtkTextActor> DistanceActor;

    // Koordinat ve durum değişkenleri
    int StartCoords[2];
    int EndCoords[2];
    bool Measuring;
};

#endif // MEASUREMENTWIDGET_H