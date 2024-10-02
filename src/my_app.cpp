#include "hello_imgui/hello_imgui.h"

HelloImGui::RunnerParams InitRunnerParams();


int main(int, char*[]) {

  auto params = InitRunnerParams();
  HelloImGui::Run(params);
}

HelloImGui::RunnerParams InitRunnerParams() {
  HelloImGui::RunnerParams para;

  auto& callbacks = para.callbacks;

  // appWindowParams
  para.appWindowParams.windowTitle = "my Example";
  para.appWindowParams.windowGeometry.size = { 640, 480 };
  para.appWindowParams.windowGeometry.sizeAuto = true;
  para.appWindowParams.restorePreviousGeometry = true;

  // imgui window
  para.imGuiWindowParams.showMenuBar = true;
  para.imGuiWindowParams.showStatusBar = true;
  para.imGuiWindowParams.defaultImGuiWindowType = HelloImGui::DefaultImGuiWindowType::ProvideFullScreenDockSpace;

  // fps
  auto& fps = para.fpsIdling;
  fps.fpsIdle = 9;

  //   dpi
  auto& dpi = para.dpiAwareParams;
  dpi.dpiWindowSizeFactor = 180.0f / 96.0f; // 例如，针对约180 DPI的4K屏幕
  dpi.fontRenderingScale = 1.0f;            // 或根据需要调整

  //   dock
  auto& dock = para.dockingParams;
  HelloImGui::DockableWindow dw1("dw window1","name1");
  HelloImGui::DockableWindow dw2("dw window2","name2");
  dock.dockableWindows.push_back(dw1);
  dock.dockableWindows.push_back(dw2);

  return para;
}
