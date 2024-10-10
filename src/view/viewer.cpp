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

    std::vector<ImPlotPoint> downsampled_pts;

    DownSampleTrajectory(buffer_["/fused_state"], downsampled_pts);

    ImPlot::PlotScatter(
      "scatter", &downsampled_pts[0].x, &downsampled_pts[0].y, downsampled_pts.size(), 0, 0, sizeof(ImPlotPoint));

    ImPlot::EndPlot();
  }

  ImGui::End();
}

void MyViewer::DownSampleTrajectory(MessageBuffer const& single_buffer, std::vector<ImPlotPoint>& pts_downsample) {
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
      raw_pts[i] = { trans.x(), trans.y() };
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
  // std::vector<ImPlotPoint> pts_downsample;
  pts_downsample.reserve(raw_pts.size());

  int const rate = 200;
  int last_index = 0; // 上一个被采样索引
  pts_downsample.push_back(raw_pts[0]);
  for (int i = 0; i < raw_pts.size(); i += rate) {
    auto& pt = raw_pts[i];

    // 如果未发生覆盖,从 [last_index,i)
    double const dx = std::abs(pt.x - pts_downsample.back().x);
    double const dy = std::abs(pt.y - pts_downsample.back().y);

    // 视口外，忽略
    if (!range.Contains(pt.x, pt.y)) {

      // 上一个点在范围内
      if (last_index == i - 1) {
        double pixel_n = std::hypotf(dx, dy) * scale_ppm; // 需要这么些个点
        pixel_n = std::min(rate, (int)pixel_n);
        PointLttb::Downsample(&raw_pts[i - rate], rate, std::back_inserter(pts_downsample), pixel_n);
      }

      continue;
    }

    if (dx >= scale_mpp || dy >= scale_mpp) {

      double pixel_n = std::hypotf(dx, dy) * scale_ppm; // 需要这么些个点
      pixel_n = std::min(rate, (int)pixel_n);
      PointLttb::Downsample(&raw_pts[i - rate], rate, std::back_inserter(pts_downsample), pixel_n);
      last_index = i;
    }
  }

  ELOGD << "raw raw_pts = " << single_buffer.size() << " downsample pts = " << pts_downsample.size();
}
