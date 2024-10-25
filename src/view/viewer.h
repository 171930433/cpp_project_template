#pragma once

// #include "data/reader.h"
#include "injector/scene.h"
#include "mylib.h"

#include <boost/geometry.hpp>
#include <boost/geometry/geometries/register/point.hpp>

class IDataReader;

class MyViewer : public SimpleScene {
public:
  MyViewer();
  ~MyViewer();
  void Init();
  void draw(vtkObject* caller, unsigned long eventId, void* callData) override;

protected:
  MultuiSensorFusion msf_;
  std::shared_ptr<IDataReader> reader_;

protected:

protected:
  bool imgui_demo_ = false;
  bool implot_demo_ = false;
  std::atomic_bool stop_{ false };
  std::atomic_bool exit_{ false };
  std::atomic_bool inited_{ false };
  TotalBuffer<Gnss, Imu, State> buffer3_;
  tf::Executor executor_;
};
