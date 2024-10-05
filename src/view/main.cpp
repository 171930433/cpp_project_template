#include "injector/vtkDearImGuiInjector.h"

#include <vtkRenderer.h>
#include <vtkRenderWindow.h>
#include <vtkRenderWindowInteractor.h>


//------------------------------------------------------------------------------
int main(int argc, char** argv) {

  vtkNew<vtkRenderer> renderer;
  vtkNew<vtkRenderWindow> renderWindow;
  vtkNew<vtkRenderWindowInteractor> iren;

  renderWindow->SetMultiSamples(8);
  renderWindow->AddRenderer(renderer);
  iren->SetRenderWindow(renderWindow);

  // Start rendering app
  renderWindow->Render();

  // Initialize an overlay with DearImgui elements.
  vtkNew<vtkDearImGuiInjector> dearImGuiOverlay;
  // dearImGuiOverlay->DebugOn();
  // dearImGuiOverlay->Inject(iren);

  // dearImGuiOverlay->AddObserver(vtkDearImGuiInjector::ImGuiSetupEvent, &scene, &SimpleScene::setup);
  // dearImGuiOverlay->AddObserver(vtkDearImGuiInjector::ImGuiDrawEvent, &scene, &SimpleScene::draw);

  // Start event loop
  renderWindow->SetSize(640*2, 480*2);
  iren->EnableRenderOff();
  iren->Start();

  return 0;
}
