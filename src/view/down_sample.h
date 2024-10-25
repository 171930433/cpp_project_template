#pragma once

#include <boost/geometry.hpp>
#include <boost/geometry/geometries/box.hpp>
#include <boost/geometry/geometries/point.hpp>
#include <boost/geometry/geometries/register/point.hpp>
#include <boost/geometry/geometries/segment.hpp>
#include <implot.h>

BOOST_GEOMETRY_REGISTER_POINT_2D(ImPlotPoint, double, cs::cartesian, x, y);
namespace bg = boost::geometry;
namespace bgm = boost::geometry::model;

template <typename _Sensor, std::enable_if_t<IsTrajectory_v<_Sensor>>* = nullptr>
std::vector<ImPlotPoint> DownSample(SensorContainer<_Sensor> const& single_buffer) {
  std::vector<ImPlotPoint> pts_downsample;

  if (single_buffer.empty()) return {};

  std::string_view channel_name = single_buffer.channel_name_;
  static std::unordered_map<std::string_view, std::vector<ImPlotPoint>> down_pts;

  auto& raw_pts = down_pts[channel_name];

  int raw_pts_size = raw_pts.size();
  {
    // size的获取也需要枷锁，因为size的计算依赖迭代器，而去size的过程中，有添加的话会导致迭代器失效
    std::shared_lock<std::shared_mutex> lg(single_buffer.mtx_);
    int single_buffer_size = single_buffer.size();

    if (raw_pts_size < single_buffer.size()) {
      raw_pts.resize(single_buffer.size());

      for (int i = raw_pts_size; i < single_buffer.size(); ++i) {
        auto const& pose = single_buffer[i]->rpose_;
        raw_pts[i] = { pose(0, 3), pose(1, 3) };
      }
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

  return pts_downsample;
}
