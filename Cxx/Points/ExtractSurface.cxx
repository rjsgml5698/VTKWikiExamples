#include <vtkSmartPointer.h>
#include <vtkPLYReader.h>
#include <vtkXMLPolyDataReader.h>
#include <vtkOBJReader.h>
#include <vtkSTLReader.h>
#include <vtkPointSource.h>

#include <vtkPCANormalEstimation.h>
#include <vtkSignedDistance.h>
#include <vtkExtractSurface.h>
#include <vtkPointData.h>

#include <vtkPolyDataMapper.h>
#include <vtkProperty.h>
#include <vtkRenderWindow.h>
#include <vtkRenderWindowInteractor.h>
#include <vtkRenderer.h>
#include <vtkCamera.h>

#include <vtksys/SystemTools.hxx>

static vtkSmartPointer<vtkPolyData> ReadPolyData(const char *fileName);

int main (int argc, char *argv[])
{
  vtkSmartPointer<vtkPolyData> polyData = ReadPolyData(argc > 1 ? argv[1] : "");;

  double bounds[6];
  polyData->GetBounds(bounds);
  double range[3];
  for (int i = 0; i < 3; ++i)
  {
    range[i] = bounds[2*i + 1] - bounds[2*i];
  }

  int sampleSize = polyData->GetNumberOfPoints() * .00005;
  if (sampleSize < 10)
    {
    sampleSize = 10;
    }
  std::cout << "Sample size is: " << sampleSize << std::endl;
  // Do we need to estimate normals?
  vtkSmartPointer<vtkSignedDistance> distance =
    vtkSmartPointer<vtkSignedDistance>::New();
  if (polyData->GetPointData()->GetNormals())
    {
    std::cout << "Using normals from input file" << std::endl;
    distance->SetInputData (polyData);
    }
  else
    {
    std::cout << "Estimating normals using PCANormalEstimation" << std::endl;
    vtkSmartPointer<vtkPCANormalEstimation> normals =
      vtkSmartPointer<vtkPCANormalEstimation>::New();
    normals->SetInputData (polyData);
    normals->SetSampleSize(sampleSize);
    normals->SetNormalOrientationToGraphTraversal();
    normals->FlipNormalsOn();
    distance->SetInputConnection (normals->GetOutputPort());
    }
  std::cout << "Range: "
            << range[0] << ", "
            << range[1] << ", "
            << range[2] << std::endl;
  int dimension = 256;
  double radius = range[0] * .02;
  radius = range[0] / static_cast<double>(dimension) * 3; // ~3 voxels
  std::cout << "Radius: " << radius << std::endl;

  distance->SetRadius(radius);
  distance->SetDimensions(dimension, dimension, dimension);
  distance->SetBounds(
    bounds[0] - range[0] * .1,
    bounds[1] + range[0] * .1,
    bounds[2] - range[1] * .1,
    bounds[3] + range[1] * .1,
    bounds[4] - range[2] * .1,
    bounds[5] + range[2] * .1);

  vtkSmartPointer<vtkExtractSurface> surface =
    vtkSmartPointer<vtkExtractSurface>::New();
  surface->SetInputConnection (distance->GetOutputPort());
  surface->SetRadius(radius * .99);
  surface->Update();

  vtkSmartPointer<vtkPolyDataMapper> surfaceMapper =
    vtkSmartPointer<vtkPolyDataMapper>::New();
  surfaceMapper->SetInputConnection(surface->GetOutputPort());

  vtkSmartPointer<vtkActor> surfaceActor =
    vtkSmartPointer<vtkActor>::New();
  surfaceActor->SetMapper(surfaceMapper);
  surfaceActor->GetProperty()->SetColor(1.0000,0.3882,0.2784);
  
  // Create graphics stuff
  //
  vtkSmartPointer<vtkRenderer> ren1 =
    vtkSmartPointer<vtkRenderer>::New();
  ren1->SetBackground(.3, .4, .6);

  vtkSmartPointer<vtkRenderWindow> renWin =
    vtkSmartPointer<vtkRenderWindow>::New();
  renWin->AddRenderer(ren1);
  renWin->SetSize(512,512);

  vtkSmartPointer<vtkRenderWindowInteractor> iren =
    vtkSmartPointer<vtkRenderWindowInteractor>::New();
  iren->SetRenderWindow(renWin);
  
  // Add the actors to the renderer, set the background and size
  //
  ren1->AddActor(surfaceActor);

  // Generate an interesting view
  //
  ren1->ResetCamera();
  ren1->GetActiveCamera()->Azimuth(120);
  ren1->GetActiveCamera()->Elevation(30);
  ren1->GetActiveCamera()->Dolly(1.0);
  ren1->ResetCameraClippingRange();

  iren->Initialize();
  iren->Start();

  return EXIT_SUCCESS;
}

static vtkSmartPointer<vtkPolyData> ReadPolyData(const char *fileName)
{
  vtkSmartPointer<vtkPolyData> polyData;
  std::string extension = vtksys::SystemTools::GetFilenameExtension(std::string(fileName));
  if (extension == ".ply")
  {
    vtkSmartPointer<vtkPLYReader> reader =
      vtkSmartPointer<vtkPLYReader>::New();
    reader->SetFileName (fileName);
    reader->Update();
    polyData = reader->GetOutput();
  }
  else if (extension == ".vtp")
  {
    vtkSmartPointer<vtkXMLPolyDataReader> reader =
      vtkSmartPointer<vtkXMLPolyDataReader>::New();
    reader->SetFileName (fileName);
    reader->Update();
    polyData = reader->GetOutput();
  }
  else if (extension == ".obj")
  {
    vtkSmartPointer<vtkOBJReader> reader =
      vtkSmartPointer<vtkOBJReader>::New();
    reader->SetFileName (fileName);
    reader->Update();
    polyData = reader->GetOutput();
  }
  else if (extension == ".stl")
  {
    vtkSmartPointer<vtkSTLReader> reader =
      vtkSmartPointer<vtkSTLReader>::New();
    reader->SetFileName (fileName);
    reader->Update();
    polyData = reader->GetOutput();
  }
  else
  {
    vtkSmartPointer<vtkPointSource> points =
      vtkSmartPointer<vtkPointSource>::New();
    points->SetNumberOfPoints(100000);
    points->SetRadius(10.0);
    points->SetCenter(vtkMath::Random(-100, 100),
                      vtkMath::Random(-100, 100),
                      vtkMath::Random(-100, 100));
    points->SetDistributionToShell();
    points->Update();
    polyData = points->GetOutput();
  }
  return polyData;
}
