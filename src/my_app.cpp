#include <hello_imgui/hello_imgui.h>
#include <implot.h>

struct MyRunnerParams : public HelloImGui::RunnerParams {
  MyRunnerParams();

protected:
  bool show_imgui_demo_ = true;
  bool show_implot_demo_ = true;
};

int main(int, char*[]) {

  MyRunnerParams params;
  HelloImGui::Run(params);
}

MyRunnerParams::MyRunnerParams() {
  auto& para = *this;
  auto& callbacks = para.callbacks;
  callbacks.PostInit = [] { ImPlot::CreateContext(); };
  callbacks.BeforeExit_PostCleanup = [] {
    if (ImPlot::GetCurrentContext()) { ImPlot::DestroyContext(); }
  };

  callbacks.ShowAppMenuItems = [this] {
    ImGui::MenuItem("Imgui::DemoWindow", nullptr, &show_imgui_demo_);
    ImGui::MenuItem("Implot::DemoWindow", nullptr, &show_implot_demo_);
  };

  callbacks.BeforeImGuiRender = [this] {
    if (show_imgui_demo_) { ImGui::ShowDemoWindow(&show_imgui_demo_); }
    if (show_implot_demo_) { ImPlot::ShowDemoWindow(&show_implot_demo_); }
  };

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
  HelloImGui::DockableWindow dw1("dw window1", "name1");
  HelloImGui::DockableWindow dw2("dw window2", "name2");
  dock.dockableWindows.push_back(dw1);
  dock.dockableWindows.push_back(dw2);
}
