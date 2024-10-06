#pragma once

// #include "data/reader.h"
#include "injector/scene.h"
#include "mylib.h"

class IDataReader;

class MyViewer : public SimpleScene {
public:
  MyViewer();
  void draw(vtkObject* caller, unsigned long eventId, void* callData) override;

protected:
  MultuiSensorFusion msf_;
  std::shared_ptr<IDataReader> reader_;

protected:
  bool imgui_demo_ = false;
  bool implot_demo_ = false;
  std::atomic_bool stop_{ false };
  std::deque<Message<State>::SCPtr> fused_states_;
};
