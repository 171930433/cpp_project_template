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
  // void DownSampleTrajectory(MessageBuffer const& single_buffer, std::vector<ImPlotPoint>& pts_downsample);
  template <typename _Sensor>
  void DownSampleTrajectory2(bm::SensorContainer<_Sensor> const& single_buffer, std::vector<ImPlotPoint>& pts_downsample);

protected:
  bool imgui_demo_ = false;
  bool implot_demo_ = false;
  std::atomic_bool stop_{ false };
  std::atomic_bool exit_{ false };
  std::atomic_bool inited_{ false };
  std::deque<Message<State>::SCPtr> fused_states_;
  tf::Executor executor_;
  TotalBuffer<Gnss, Imu, State> buffer3_;
};

namespace bg = boost::geometry;
namespace bgm = boost::geometry::model;
BOOST_GEOMETRY_REGISTER_POINT_2D(ImPlotPoint, double, cs::cartesian, x, y);

template <typename _Sensor>
void MyViewer::DownSampleTrajectory2(
  bm::SensorContainer<_Sensor> const& single_buffer, std::vector<ImPlotPoint>& pts_downsample) {

  //
  if (single_buffer.empty()) return;
  std::string_view channel_name = single_buffer.front()->channel_name_;
  static std::unordered_map<std::string_view, std::vector<ImPlotPoint>> down_pts;

  auto& raw_pts = down_pts[channel_name];

  int raw_pts_size = raw_pts.size();
  if (raw_pts_size < single_buffer.size()) {
    raw_pts.resize(single_buffer.size());

    // 拷贝出所有点
    for (int i = raw_pts_size; i < single_buffer.size(); ++i) {
      auto const& pose = single_buffer[i]->rpose_;
      raw_pts[i] = { pose(0, 3), pose(1, 3) };
    }
  }

  // 1 获得视口范围,比例尺
  auto range = ImPlot::GetPlotLimits();
  bgm::box<ImPlotPoint> box{ range.Min(), range.Max() };
  // 获取绘图区域的起始位置和大小（像素单位）
  ImVec2 plot_pos = ImPlot::GetPlotPos();          // 左上角的位置（像素单位）
  ImVec2 plot_size = ImPlot::GetPlotSize();        // 绘图区的大小（像素单位）
  double scale_ppm = plot_size.x / range.X.Size(); // pixel per meter
  double scale_mpp = range.X.Size() / plot_size.x; // meter per pixel

  // 最终结果
  pts_downsample.reserve(raw_pts.size());

  pts_downsample.push_back(raw_pts[0]);

  for (int i = 0; i < raw_pts.size(); ++i) {
    auto& pt = raw_pts[i];
    auto& last_pt = pts_downsample.back();

    if (!bg::within(pt, box)) { continue; }

    double const dx = std::abs(pt.x - last_pt.x);
    double const dy = std::abs(pt.y - last_pt.y);

    if (dx < scale_mpp && dy < scale_mpp) { continue; }

    pts_downsample.push_back(pt);
  }
}