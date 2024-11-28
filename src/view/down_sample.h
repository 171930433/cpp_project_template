#pragma once

#include <boost/geometry.hpp>
#include <boost/geometry/geometries/box.hpp>
#include <boost/geometry/geometries/point.hpp>
#include <boost/geometry/geometries/register/point.hpp>
#include <boost/geometry/geometries/segment.hpp>
#include <implot.h>
#include <message/message.h>

inline bool operator==(ImPlotRect const& r1, ImPlotRect const& r2) {
  return r1.Contains(r2.Min()) && r1.Contains(r2.Max()) && r2.Contains(r1.Min()) && r2.Contains(r1.Max());
}

inline bool operator==(ImPlotRange const& r1, ImPlotRange const& r2) {
  return fabs(r1.Min - r2.Min) < 1e-6 && fabs(r1.Max - r2.Max) < 1e-6;
}

BOOST_GEOMETRY_REGISTER_POINT_2D(ImPlotPoint, double, cs::cartesian, x, y);
namespace bg = boost::geometry;
namespace bgm = boost::geometry::model;

template <typename _Sensor, std::enable_if_t<IsTrajectory_v<_Sensor>>* = nullptr>
inline std::vector<ImPlotPoint> DownSample(SensorContainer<_Sensor> const& single_buffer) {
  std::vector<ImPlotPoint> pts_downsample;

  if (single_buffer.empty()) return {};

  std::string_view channel_name = single_buffer.channel_name_;
  static std::unordered_map<std::string_view, std::vector<ImPlotPoint>> all_pts;
  static std::unordered_map<std::string_view, std::vector<ImPlotPoint>> down_pts;

  // 判断待显示点是否发生改变
  bool raw_pts_changed = false;
  auto& raw_pts = all_pts[channel_name];

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

      raw_pts_changed = true;
    }
  }

  // 判断plot显示区域是否改变
  bool viewport_changed = false;
  static ImPlotRect last_viewpor{};
  auto range = ImPlot::GetPlotLimits(); // 1 获得视口范围,比例尺

  if (last_viewpor != range) {
    viewport_changed = true;
    last_viewpor = range;
  }

  if (!raw_pts_changed && !viewport_changed) { return down_pts[channel_name]; }

  // bgm::box<ImPlotPoint> box{ range.Min(), range.Max() };
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

    // if (!bg::within(pt, box)) { continue; }
    if (!range.Contains(pt)) { continue; }

    double const dx = std::abs(pt.x - last_pt.x);
    double const dy = std::abs(pt.y - last_pt.y);

    if (dx < scale_mpp && dy < scale_mpp) { continue; }

    pts_downsample.push_back(pt);
  }

  down_pts[channel_name] = pts_downsample;

  return pts_downsample;
}

template <typename _Sensor, std::enable_if_t<std::is_same_v<_Sensor, Imu>>* = nullptr>
inline std::vector<std::array<double, 8>> DownSample(SensorContainer<_Sensor> const& single_buffer) {
  std::vector<std::array<double, 8>> pts_downsample;

  if (single_buffer.empty()) return {};

  std::string_view channel_name = single_buffer.channel_name_;
  static std::unordered_map<std::string_view, std::vector<std::array<double, 8>>> all_pts;
  static std::unordered_map<std::string_view, std::vector<std::array<double, 8>>> down_pts;

  // 判断待显示点是否发生改变
  bool raw_pts_changed = false;
  auto& raw_pts = all_pts[channel_name];

  int raw_pts_size = raw_pts.size();
  {
    // size的获取也需要枷锁，因为size的计算依赖迭代器，而去size的过程中，有添加的话会导致迭代器失效
    std::shared_lock<std::shared_mutex> lg(single_buffer.mtx_);
    int single_buffer_size = single_buffer.size();

    if (raw_pts_size < single_buffer.size()) {
      raw_pts.resize(single_buffer.size());

      for (int i = raw_pts_size; i < single_buffer.size(); ++i) {
        raw_pts[i][0] = single_buffer[i]->t0();
        raw_pts[i][1] = single_buffer[i]->msg_.acc_.x_;
        raw_pts[i][2] = single_buffer[i]->msg_.acc_.y_;
        raw_pts[i][3] = single_buffer[i]->msg_.acc_.z_;
        raw_pts[i][4] = single_buffer[i]->msg_.gyr_.x_;
        raw_pts[i][5] = single_buffer[i]->msg_.gyr_.y_;
        raw_pts[i][6] = single_buffer[i]->msg_.gyr_.z_;
        raw_pts[i][7] = single_buffer[i]->msg_.temperature_;
      }

      raw_pts_changed = true;
    }
  }

  // 判断plot显示区域是否改变
  bool viewport_changed = false;
  static ImPlotRect last_viewpor{};
  auto range = ImPlot::GetPlotLimits(); // 1 获得视口范围,比例尺

  if (last_viewpor != range) {
    viewport_changed = true;
    last_viewpor = range;
  }

  if (!raw_pts_changed && !viewport_changed) { return down_pts[channel_name]; }

  ImVec2 plot_pos = ImPlot::GetPlotPos();            // 左上角的位置（像素单位）
  ImVec2 plot_size = ImPlot::GetPlotSize();          // 绘图区的大小（像素单位）
  double scale_mpp_x = range.X.Size() / plot_size.x; // meter per pixel

  // 最终结果
  pts_downsample.reserve(raw_pts.size());
  pts_downsample.push_back(raw_pts[0]);
  int last_index = 0;

  // 获得视口范围内的点, 根据采样间隔遍历
  for (int i = 0; i < raw_pts.size(); ++i) {
    auto& pt = raw_pts[i];
    auto& last_pt = raw_pts[last_index];

    // 根据所有点的时间间隔进行采样，在平移时就不会抖动
    if (fabs(pt[0] - last_pt[0]) < scale_mpp_x) { continue; }

    last_index = i;

    if (!range.X.Contains(pt[0])) { continue; }

    pts_downsample.push_back(pt);
  }

  down_pts[channel_name] = pts_downsample;
  // ELOGD << fmt::format("raw={}, downsample to {}", raw_pts.size(), pts_downsample.size());

  return pts_downsample;
}