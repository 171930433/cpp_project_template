// #include "injector/vtkDearImGuiInjector.h"

#include <vtkRenderWindow.h>
#include <vtkRenderWindowInteractor.h>
#include <vtkRenderer.h>

#include "viewer.h"

int main(int argc, char** argv) {

  vtkNew<vtkRenderer> renderer;
  vtkNew<vtkRenderWindow> renderWindow;
  vtkNew<vtkRenderWindowInteractor> iren;

  renderWindow->AddRenderer(renderer);
  iren->SetRenderWindow(renderWindow);

  // Start rendering app
  renderWindow->Render();

  MyViewer viewer;
  viewer.Inject(iren);

  // Start event loop
  renderWindow->SetSize(640 * 2, 480 * 2);
  iren->EnableRenderOff();
  iren->Start();

  return 0;
}
