#include "viewer.h"
#include "lttb.h"
#include "modules/psins/psins_app.h"
#include "tree_view_helper.h"

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
    buffer_[frame->channel_name_].push_back(frame);
    // fused_states_.push_back(frame);
    // ELOGD << frame->rpose_.translation().transpose();
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
          buffer_[it.first->channel_name_].push_back(it.first);
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

  ImGui::LabelText("fused_state size", "%ld", buffer_["/fused_state"].size());

  ImGui::End();

  ImGui::Begin("trj");

  auto plot_flag = ImPlotFlags_Equal;
  if (stop_ && ImPlot::BeginPlot("##0", ImVec2(-1, -1), plot_flag)) {
    // auto get_data = [](int idx, void* data) {
    //   auto* buffer = static_cast<std::vector<MessageBase::SCPtr>*>(data);
    //   Eigen::Isometry3d const* rpose = nullptr;
    //   if (buffer->at(idx)->channel_type_ == ylt::reflection::type_string<Gnss>()) {
    //     auto frame = std::dynamic_pointer_cast<Message<Gnss> const>(buffer->at(idx));
    //     rpose = &frame->rpose_;
    //   } else if (buffer->at(idx)->channel_type_ == ylt::reflection::type_string<State>()) {
    //     auto frame = std::dynamic_pointer_cast<Message<State> const>(buffer->at(idx));
    //     rpose = &frame->rpose_;
    //   }
    //   return ImPlotPoint(rpose->translation().x(), rpose->translation().y());
    // };

    // // 1 获得视口范围
    // auto range = ImPlot::GetPlotLimits();
    // // 获取绘图区域的起始位置和大小（像素单位）
    // ImVec2 plot_pos = ImPlot::GetPlotPos();          // 左上角的位置（像素单位）
    // ImVec2 plot_size = ImPlot::GetPlotSize();        // 绘图区的大小（像素单位）
    // double scale_ppm = plot_size.x / range.X.Size(); // pixel per meter
    // double scale_mpp = range.X.Size() / plot_size.x; // meter per pixel
    // // 获取视口范围
    // // auto& pts = buffer_["/gnss"];
    // auto& pts = buffer_["/fused_state"];

    // // std::vector<ImPlotPoint> pts_viewport;
    // std::vector<MessageBase::SCPtr> pts_downsample;

    // // pts_viewport.reserve(pts.size());
    // pts_downsample.reserve(pts.size());

    // int last_index = 0;
    // ImPlotPoint last_pt = { 0, 0 };

    // int const rate = 200;
    // for (auto i = 0; i < pts.size(); i += rate) {
    //   auto& pt = pts[i];
    //   Eigen::Isometry3d const* rpose = nullptr;
    //   if (pt->channel_type_ == ylt::reflection::type_string<Gnss>()) {
    //     auto frame = std::dynamic_pointer_cast<Message<Gnss> const>(pt);
    //     rpose = &frame->rpose_;
    //   } else if (pt->channel_type_ == ylt::reflection::type_string<State>()) {
    //     auto frame = std::dynamic_pointer_cast<Message<State> const>(pt);
    //     rpose = &frame->rpose_;
    //   }
    //   auto& trans = rpose->translation();
    //   if (!range.Contains(trans.x(), trans.y())) {
    //     last_pt = { trans.x(), trans.y() };
    //     last_index = 0;
    //     continue;
    //   }

    //   if (i == 0) {
    //     // pts_downsample.emplace_back(ImPlotPoint{ trans.x(), trans.y() });
    //     pts_downsample.push_back(pts[i]);
    //     last_pt = { trans.x(), trans.y() };
    //     last_index = 0;
    //     continue;
    //   }
    //   // 如果未发生覆盖,从lasti到i开始采样
    //   double const dx = std::abs(trans.x() - last_pt.x);
    //   double const dy = std::abs(trans.y() - last_pt.y);
    //   if (dx >= scale_mpp || dy >= scale_mpp) {
    //     // 需要这么些个点
    //     double pixel_n = std::hypotf(dx, dy) * scale_ppm;
    //     pixel_n = std::min((i - last_index), (int)pixel_n);
    //     // 获得这个区间的点
    //     // PointLttb::Downsample(&pts[last_index], (i - last_index) * rate, std::back_inserter(pts_downsample),
    //     // pixel_n);
    //     // std::vector<MessageBase::SCPtr> temp(pts[last_index], &pts[i]);pts.it
    //     StateLttb::Downsample(pts.begin() += last_index, (i - last_index), std::back_inserter(pts_downsample),
    //     pixel_n);

    //     last_pt = { trans.x(), trans.y() };
    //     last_index = i;
    //   } else {
    //     last_pt = { trans.x(), trans.y() };
    //     last_index = i;
    //   }
    // }

    // ImPlot::PlotLine(
    //   "line", &pts_downsample[0].x, &pts_downsample[0].y, pts_downsample.size(), 0, 0, sizeof(ImPlotPoint));
    // ImPlot::PlotScatter(
    //   "scatter", &pts_downsample[0].x, &pts_downsample[0].y, pts_downsample.size(), 0, 0, sizeof(ImPlotPoint));

    // ImPlot::PlotLineG("gnss_line", get_data, &buffer_["/gnss"], buffer_["/gnss"].size());
    // ImPlot::PlotScatterG("gnss_scatter", get_data, &buffer_["/gnss"], buffer_["/gnss"].size());

    // ImPlot::PlotLineG("fused_state_line", get_data, &buffer_["/fused_state"], buffer_["/fused_state"].size());
    // ImPlot::PlotScatterG("fused_state_scatter", get_data, &pts_downsample, pts_downsample.size());
    // ELOGD << "raw pts = " << pts.size() << " downsample pts = " << pts_downsample.size();

    DownSampleTrajectory(buffer_["/fused_state"]);

    ImPlot::EndPlot();
  }

  ImGui::End();
}

void MyViewer::DownSampleTrajectory(MessageBuffer const& single_buffer) {
  //
  if (single_buffer.empty()) return;
  std::string_view channel_name = single_buffer.front()->channel_name_;
  static std::unordered_map<std::string_view, std::vector<ImPlotPoint>> down_pts;

  auto& pts = down_pts[channel_name];

  if (pts.size() < single_buffer.size()) {
    pts.resize(single_buffer.size());

    // 拷贝出所有点
    for (int i = pts.size(); i < single_buffer.size(); ++i) {
      auto& pt = single_buffer[i];
      Eigen::Isometry3d const* rpose = nullptr;
      if (pt->channel_type_ == ylt::reflection::type_string<Gnss>()) {
        auto frame = std::dynamic_pointer_cast<Message<Gnss> const>(pt);
        rpose = &frame->rpose_;
      } else if (pt->channel_type_ == ylt::reflection::type_string<State>()) {
        auto frame = std::dynamic_pointer_cast<Message<State> const>(pt);
        rpose = &frame->rpose_;
      }
      auto& trans = rpose->translation();
      pts[i] = { trans.x(), trans.y() };
    }
  }

  // 1 获得视口范围,比例尺
  auto range = ImPlot::GetPlotLimits();
  // 获取绘图区域的起始位置和大小（像素单位）
  ImVec2 plot_pos = ImPlot::GetPlotPos();          // 左上角的位置（像素单位）
  ImVec2 plot_size = ImPlot::GetPlotSize();        // 绘图区的大小（像素单位）
  double scale_ppm = plot_size.x / range.X.Size(); // pixel per meter
  double scale_mpp = range.X.Size() / plot_size.x; // meter per pixel

  // 最终结果
  std::vector<ImPlotPoint> pts_downsample;
  int const rate = 200;

  int last_index = 0;
  pts_downsample.push_back(pts[0]);
  for (int i = 1; i < pts.size(); i += rate) {
    auto& pt = pts[i];

    // 视口外，忽略
    if (!range.Contains(pt.x, pt.y)) {
      last_index = i;
      continue;
    }

    // 如果未发生覆盖,从 [last_index,i)
    double const dx = std::abs(pt.x - pts_downsample.back().x);
    double const dy = std::abs(pt.y - pts_downsample.back().y);

    if (dx >= scale_mpp || dy >= scale_mpp) {
      // 需要这么些个点
      double pixel_n = std::hypotf(dx, dy) * scale_ppm;
      pixel_n = std::min((i - last_index), (int)pixel_n);
      // 获得这个区间的点
      PointLttb::Downsample(&pts[last_index], (i - last_index), std::back_inserter(pts_downsample), pixel_n);
    }
    last_index = i;
  }

  ImPlot::PlotScatter(
    "scatter", &pts_downsample[0].x, &pts_downsample[0].y, pts_downsample.size(), 0, 0, sizeof(ImPlotPoint));
  ELOGD << "raw pts = " << single_buffer.size() << " downsample pts = " << pts_downsample.size();
}
