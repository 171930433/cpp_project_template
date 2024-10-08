#pragma once

#include "font/Karla-Regular.hpp"
#include "injector/vtkDearImGuiInjector.h"
#include <imgui.h>
#include <imgui_stdlib.h>
#include <implot.h>
#include <vtkObject.h>
#include <vtkSmartPointer.h>
#include <portable-file-dialogs.h>

class SimpleScene {
public:
  SimpleScene();
  virtual ~SimpleScene() = default;

  virtual void setup(vtkObject* caller, unsigned long eventId, void* callData);
  virtual void draw(vtkObject* caller, unsigned long eventId, void* callData) = 0;
  void Inject(vtkRenderWindowInteractor* iren) { dearImGuiOverlay_->Inject(iren); }

protected:
  vtkSmartPointer<vtkDearImGuiInjector> dearImGuiOverlay_;
  // vtkRenderer* render_;
};

inline SimpleScene::SimpleScene() {
  dearImGuiOverlay_ = vtkSmartPointer<vtkDearImGuiInjector>::New();
  //   dearImGuiOverlay_->DebugOn();
  dearImGuiOverlay_->AddObserver(vtkDearImGuiInjector::ImGuiSetupEvent, this, &SimpleScene::setup);
  dearImGuiOverlay_->AddObserver(vtkDearImGuiInjector::ImGuiDrawEvent, this, &SimpleScene::draw);
}

inline void SimpleScene::setup(vtkObject* caller, unsigned long eventId, void* callData) {

  vtkDearImGuiInjector* overlay_ = reinterpret_cast<vtkDearImGuiInjector*>(caller);
  if (!callData) { return; }
  bool imguiInitStatus = *(reinterpret_cast<bool*>(callData));
  if (imguiInitStatus) {
    auto& io = ImGui::GetIO();
    float const scale = 180.0f / 96.0f;
    // float const scale = 96.0f / 96.0f;
    io.FontGlobalScale *= scale;
    io.DisplayFramebufferScale = { scale, scale };
    io.Fonts->AddFontFromMemoryCompressedBase85TTF(Karla_Regular_compressed_data_base85, 16);
    io.Fonts->AddFontDefault();

    auto& style = ImGui::GetStyle();
    style.ChildRounding = 8;
    style.FrameRounding = 8;
    style.GrabRounding = 8;
    style.PopupRounding = 8;
    style.ScrollbarRounding = 8;
    style.TabRounding = 8;
    style.WindowRounding = 8;
    style.FrameBorderSize = 1.f;
  } else {
    vtkErrorWithObjectMacro(overlay_, "Failed to setup overlay UI because ImGUI failed to initialize!");
  }

  // enable docking
  ImGui::GetIO().ConfigFlags |= ImGuiConfigFlags_DockingEnable;
};
