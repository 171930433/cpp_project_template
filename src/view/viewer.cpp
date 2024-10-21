#include "viewer.h"
#include "lttb.h"
#include "modules/psins/psins_app.h"
#include "tree_view_helper.h"

#ifdef FF
#undef FF
#endif

#include <boost/geometry.hpp>
#include <boost/geometry/geometries/box.hpp>
#include <boost/geometry/geometries/point.hpp>
#include <boost/geometry/geometries/register/point.hpp>
#include <boost/geometry/geometries/segment.hpp>
BOOST_GEOMETRY_REGISTER_POINT_2D(ImPlotPoint, double, cs::cartesian, x, y);
namespace bg = boost::geometry;
namespace bgm = boost::geometry::model;

template <typename _Sensor, typename = std::enable_if_t<IsTrajectory_v<_Sensor>>>
void DownSample(SensorContainer<_Sensor> const& single_buffer, std::vector<ImPlotPoint>& pts_downsample);

MyViewer::MyViewer() {
  reader_ = std::make_shared<PsinsReader>();
}

MyViewer::~MyViewer() {
  stop_ = true;
  exit_ = true;
};

void MyViewer::Init() {
  reader_->Init(FLAGS_data_dir);
  msf_.Init();
  msf_.CreateModule<PsinsApp>();

  Message<State>::CFunc cbk = [this](Message<State>::SCPtr frame) {
    // buffer_[frame->channel_name_].push_back(frame);
    buffer3_.Append(frame);
  };
  msf_.dispatcher()->RegisterWriter("/fused_state", cbk);

  inited_ = true;
}

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

  if (this->imgui_demo_) { ImGui::ShowDemoWindow(&this->imgui_demo_); }
  if (this->implot_demo_) { ImPlot::ShowDemoWindow(&this->implot_demo_); }

  // project window
  ImGui::Begin("Project");
  if (FLAGS_data_dir.empty()) { FLAGS_data_dir = "/home/gsk/pro/cpp_project_template/data/mimuattgps.bin"; }
  std::string data_str = FLAGS_data_dir;
  ImGui::InputText("DataDir", &data_str);
  ImGui::SameLine();
  if (ImGui::SmallButton("...##1")) {
    auto dialog = pfd::open_file("Select a file", "/home/gsk/pro/cpp_project_template/data/mimuattgps.bin",
      { "All Files", "*", "Image Files", "*.png *.jpg *.jpeg *.bmp", "Audio Files", "*.wav *.mp3" },
      pfd::opt::multiselect);
    for (auto const& filename : dialog.result()) std::cout << "Selected file: " << filename << "\n";
    if (!dialog.result().empty()) { FLAGS_data_dir = dialog.result().front(); }
  }

  // config
  if (FLAGS_config_dir.empty()) { FLAGS_config_dir = "/home/gsk/pro/cpp_project_template/config/demo"; }
  std::string config_str = FLAGS_config_dir;
  ImGui::InputText("ConfigDir", &config_str);
  ImGui::SameLine();
  if (ImGui::SmallButton("...##2")) {
    auto dialog = pfd::select_folder("Select a file", "/home/gsk/pro/cpp_project_template/config/demo");
    if (!dialog.result().empty()) { FLAGS_config_dir = dialog.result(); }
  }

  ImGuiTreeNodeFlags node_flags =
    ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_OpenOnDoubleClick | ImGuiTreeNodeFlags_SpanAvailWidth;

  TreeView(*msf_.cm(), node_flags);

  // 初始化

  if (ImGui::Button("Init##")) {
    if (!FLAGS_data_dir.empty() && !FLAGS_config_dir.empty()) { Init(); }
  }

  ImGui::SameLine();

  // 运行
  if (ImGui::Button("process")) {
    if (inited_) {
      executor_.silent_async([this] {
        for (auto it = reader_->ReadFrame(); it.second != IDataReader::IOState::END && !exit_;
             it = reader_->ReadFrame()) {
          buffer3_.Append(it.first);
          msf_.ProcessData(it.first);
          while (stop_) { std::this_thread::sleep_for(std::chrono::duration(std::chrono::milliseconds(100))); }
        }
      });
    }
  }

  ImGui::SameLine();

  if (ImGui::Button("stop")) {
    stop_.store(!stop_);
    ELOGD << "stop_ = " << stop_;
  }

  // ImGui::LabelText("fused_state size", "%ld", buffer_["/fused_state"].size());
  ImGui::LabelText("fused_state size", "%ld", buffer3_.Get<State>("/fused_state").size());

  ImGui::End();

  ImGui::Begin("trj");

  auto plot_flag = ImPlotFlags_Equal;
  if (ImGui::Button("load pins results")) {
    auto& pins_re = buffer3_.Get<State>("/psins/state");
    if (pins_re.empty()) { PsinsReader::LoadResult("/home/gsk/pro/cpp_project_template/data/ins.bin", pins_re); }
  }
  if (ImPlot::BeginPlot("##0", ImVec2(-1, -1), plot_flag)) {

    if (stop_) {
      std::vector<ImPlotPoint> pts1;
      DownSample(buffer3_.Get<State>("/fused_state"), pts1);
      ImPlot::PlotScatter("scatter", &pts1[0].x, &pts1[0].y, pts1.size(), 0, 0, sizeof(ImPlotPoint));

      std::vector<ImPlotPoint> pts2;
      DownSample(buffer3_.Get<Gnss>("/gnss"), pts2);
      ImPlot::PlotScatter("gnss_scatter", &pts2[0].x, &pts2[0].y, pts2.size(), 0, 0, sizeof(ImPlotPoint));
    }

    std::vector<ImPlotPoint> pts2;
    DownSample(buffer3_.Get<State>("/psins/state"), pts2);
    ImPlot::PlotScatter("psins_scatter", &pts2[0].x, &pts2[0].y, pts2.size(), 0, 0, sizeof(ImPlotPoint));

    ImPlot::EndPlot();
  }

  ImGui::End();

  ImGui::Begin("plot 2d");

  if (ImPlot::BeginPlot("##CustomRend", ImVec2(-1, -1), plot_flag)) {
    ImVec2 cntr = ImPlot::PlotToPixels(ImPlotPoint(0.5f, 0.5f));
    ImVec2 rmin = ImPlot::PlotToPixels(ImPlotPoint(0.25f, 0.75f));
    ImVec2 rmax = ImPlot::PlotToPixels(ImPlotPoint(0.75f, 0.25f));
    ImPlot::PushPlotClipRect();
    ImPlot::GetPlotDrawList()->AddCircleFilled(cntr, 20, IM_COL32(255, 255, 0, 255), 20);
    ImPlot::GetPlotDrawList()->AddRect(rmin, rmax, IM_COL32(128, 0, 255, 255));
    ImPlot::PopPlotClipRect();
    ImPlot::EndPlot();
  }

  ImGui::End();
}

template <typename _Sensor, typename = std::enable_if_t<IsTrajectory_v<_Sensor>>>
void DownSample(SensorContainer<_Sensor> const& single_buffer, std::vector<ImPlotPoint>& pts_downsample) {
  if (single_buffer.empty()) return;

  std::string_view channel_name = single_buffer.channel_name_;
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
