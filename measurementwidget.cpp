#include "measurementwidget.h"
#include <vtkObjectFactory.h>
#include <vtkRendererCollection.h>
#include <vtkCamera.h>
#include <vtkMath.h>
#include <vtkRenderWindow.h>
#include <vtkImageData.h>
#include <vtkTextActor.h>
#include <vtkLineSource.h>
#include <vtkPolyDataMapper.h>
#include <vtkActor.h>
#include <vtkRenderer.h>
#include <vtkProperty.h>
#include <vtkTextProperty.h>
#include <vtkRenderWindowInteractor.h>

#include <sstream>
#include <iomanip>
#include <cmath>

vtkStandardNewMacro(MeasurementWidget);

MeasurementWidget::MeasurementWidget() : Measuring(false)
{
    // Hat kaynağı oluşturma
    LineSource = vtkSmartPointer<vtkLineSource>::New();
    LineSource->SetPoint1(0, 0, 0);
    LineSource->SetPoint2(0, 0, 0);
    
    // Hat görselleştirme için mapper ve actor
    LineMapper = vtkSmartPointer<vtkPolyDataMapper>::New();
    LineMapper->SetInputConnection(LineSource->GetOutputPort());
    
    LineActor = vtkSmartPointer<vtkActor>::New();
    LineActor->SetMapper(LineMapper);
    LineActor->GetProperty()->SetColor(1, 1, 0); // Sarı renk
    LineActor->GetProperty()->SetLineWidth(2);   // Hat kalınlığı
    
    // Mesafe metni için text actor
    DistanceActor = vtkSmartPointer<vtkTextActor>::New();
    DistanceActor->GetTextProperty()->SetFontSize(14);
    DistanceActor->GetTextProperty()->SetColor(1, 1, 0); // Sarı renk
    DistanceActor->GetTextProperty()->SetBackgroundColor(0, 0, 0);
    DistanceActor->GetTextProperty()->SetBackgroundOpacity(0.5);
    DistanceActor->SetPosition(10, 10);
    DistanceActor->SetInput("Distance: 0.00 mm");
    
    // Başlangıç koordinatları
    StartCoords[0] = StartCoords[1] = 0;
    EndCoords[0] = EndCoords[1] = 0;
}

MeasurementWidget::~MeasurementWidget()
{
    // Temizleme işlemleri gerekirse
}

void MeasurementWidget::SetViewer(vtkSmartPointer<vtkImageViewer2> viewer)
{
    Viewer = viewer;
    
    // Renderer'a aktörleri ekleyelim
    Viewer->GetRenderer()->AddActor(LineActor);
    Viewer->GetRenderer()->AddActor(DistanceActor);
    
    // Aktörleri gizle (ölçüm başlayana kadar)
    LineActor->VisibilityOff();
    DistanceActor->VisibilityOff();
}

void MeasurementWidget::OnLeftButtonDown()
{
    if (!Viewer) {
        vtkInteractorStyleImage::OnLeftButtonDown();
        return;
    }
    
    // Fare pozisyonunu al
    int x, y;
    this->GetInteractor()->GetEventPosition(x, y);
    
    // Ölçüm başlat
    StartMeasurement(x, y);
    
    // Tıklama işlemini ilet
    vtkInteractorStyleImage::OnLeftButtonDown();
}

void MeasurementWidget::OnMouseMove()
{
    // Fare hareketi sırasında ölçümü güncelle
    if (Measuring && Viewer) {
        int x, y;
        this->GetInteractor()->GetEventPosition(x, y);
        UpdateMeasurement(x, y);
    }
    
    // Fare hareketini ilet
    vtkInteractorStyleImage::OnMouseMove();
}

void MeasurementWidget::OnLeftButtonUp()
{
    // Ölçüm aktifse tamamla
    if (Measuring && Viewer) {
        int x, y;
        this->GetInteractor()->GetEventPosition(x, y);
        UpdateMeasurement(x, y);
        EndMeasurement();
    }
    
    // Tıklama bırakma işlemini ilet
    vtkInteractorStyleImage::OnLeftButtonUp();
}

void MeasurementWidget::StartMeasurement(int x, int y)
{
    // Başlangıç koordinatlarını kaydet
    StartCoords[0] = x;
    StartCoords[1] = y;
    
    // Bitiş koordinatları başlangıçta aynı
    EndCoords[0] = x;
    EndCoords[1] = y;
    
    // Dünya koordinatlarına dönüştür
    double p1[3], p2[3];
    DisplayToWorld(StartCoords[0], StartCoords[1], p1);
    DisplayToWorld(EndCoords[0], EndCoords[1], p2);
    
    // Hat kaynağını güncelle
    LineSource->SetPoint1(p1);
    LineSource->SetPoint2(p2);
    LineSource->Update();
    
    // Aktörleri görünür yap
    LineActor->VisibilityOn();
    DistanceActor->VisibilityOn();
    
    // Ölçüm durumunu aktifleştir
    Measuring = true;
    
    // Mesafe metnini güncelle
    UpdateDistance();
    
    // Render
    this->GetInteractor()->GetRenderWindow()->Render();
}

void MeasurementWidget::UpdateMeasurement(int x, int y)
{
    if (!Measuring) return;
    
    // Bitiş koordinatlarını güncelle
    EndCoords[0] = x;
    EndCoords[1] = y;
    
    // Dünya koordinatlarına dönüştür
    double p2[3];
    DisplayToWorld(EndCoords[0], EndCoords[1], p2);
    
    // Hat kaynağını güncelle
    LineSource->SetPoint2(p2);
    LineSource->Update();
    
    // Mesafe metnini güncelle
    UpdateDistance();
    
    // Render
    this->GetInteractor()->GetRenderWindow()->Render();
}

void MeasurementWidget::EndMeasurement()
{
    Measuring = false;
}

void MeasurementWidget::UpdateDistance()
{
    if (!Viewer) return;
    
    // Piksel cinsinden mesafeyi hesapla
    double distance = CalculateDistance(StartCoords[0], StartCoords[1], EndCoords[0], EndCoords[1]);
    
    // Dünya koordinatlarındaki mesafeyi hesapla
    double p1[3], p2[3];
    DisplayToWorld(StartCoords[0], StartCoords[1], p1);
    DisplayToWorld(EndCoords[0], EndCoords[1], p2);
    
    double worldDistance = sqrt(vtkMath::Distance2BetweenPoints(p1, p2));
    
    // DICOM veri piksel boyutuna göre gerçek mesafeyi hesaplayabilirsiniz
    // Burada basit bir yaklaşımla mm cinsinden ifade ediyoruz
    double spacing[3];
    Viewer->GetInput()->GetSpacing(spacing);
    double realDistance = worldDistance * spacing[0]; // mm cinsinden
    
    // Mesafe metnini güncelle
    std::ostringstream distanceText;
    distanceText << "Distance: " << std::fixed << std::setprecision(2) << realDistance << " mm";
    DistanceActor->SetInput(distanceText.str().c_str());
    
    // Metin pozisyonunu güncelle - ortaya yakın bir yer
    int midX = (StartCoords[0] + EndCoords[0]) / 2;
    int midY = (StartCoords[1] + EndCoords[1]) / 2 + 15; // Çizginin biraz üstünde
    DistanceActor->SetPosition(midX, midY);
}

double MeasurementWidget::CalculateDistance(int x1, int y1, int x2, int y2)
{
    return sqrt(pow(static_cast<double>(x2 - x1), 2) + pow(static_cast<double>(y2 - y1), 2));
}

void MeasurementWidget::DisplayToWorld(double x, double y, double worldPos[3])
{
    if (!Viewer) return;
    
    vtkRenderer* renderer = Viewer->GetRenderer();
    if (!renderer) return;
    
    // Derinlik değerini al (Z-buffer)
    renderer->SetDisplayPoint(x, y, 0);
    renderer->DisplayToWorld();
    double* world = renderer->GetWorldPoint();
    
    // Z koordinatını hesaplamak için ışın izleme kullanılabilir
    // Basit bir yaklaşım olarak, görüntü düzleminde olduğunu varsayıyoruz
    worldPos[0] = world[0];
    worldPos[1] = world[1];
    worldPos[2] = 0;
}

void MeasurementWidget::WorldToDisplay(double x, double y, double z, double displayPos[2])
{
    if (!Viewer) return;
    
    vtkRenderer* renderer = Viewer->GetRenderer();
    if (!renderer) return;
    
    renderer->SetWorldPoint(x, y, z, 1.0);
    renderer->WorldToDisplay();
    double* display = renderer->GetDisplayPoint();
    
    displayPos[0] = display[0];
    displayPos[1] = display[1];
}