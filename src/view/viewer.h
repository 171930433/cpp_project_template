#pragma once

#include "injector/scene.h"

class MyViewer : public SimpleScene {
public:
  void draw(vtkObject* caller, unsigned long eventId, void* callData) override;

protected:
  bool imgui_demo_ = false;
  bool implot_demo_ = false;
};

void MyViewer::draw(vtkObject* caller, unsigned long eventId, void* callData) {

  // Menu Bar
  if (ImGui::BeginMainMenuBar()) {
    if (ImGui::BeginMenu("demos")) {
      ImGui::MenuItem("ImGui", NULL, &this->imgui_demo_);
      ImGui::MenuItem("implot", NULL, &this->implot_demo_);
      ImGui::EndMenu();
    }
    ImGui::EndMainMenuBar();
  }

  if (this->imgui_demo_) {
    ImGui::ShowDemoWindow(&this->imgui_demo_);
  }
  if (this->implot_demo_) {
    ImPlot::ShowDemoWindow(&this->implot_demo_);
  }
}