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
  if (ImPlot::BeginPlot("##0", ImVec2(-1, -1), plot_flag)) {
    auto get_data = [](int idx, void* data) {
      auto* buffer = static_cast<MessageBuffer*>(data);
      Eigen::Isometry3d const* rpose = nullptr;
      if (buffer->at(idx)->channel_type_ == ylt::reflection::type_string<Gnss>()) {
        auto frame = std::dynamic_pointer_cast<Message<Gnss> const>(buffer->at(idx));
        rpose = &frame->rpose_;
      } else if (buffer->at(idx)->channel_type_ == ylt::reflection::type_string<State>()) {
        auto frame = std::dynamic_pointer_cast<Message<State> const>(buffer->at(idx));
        rpose = &frame->rpose_;
      }
      return ImPlotPoint(rpose->translation().x(), rpose->translation().y());
    };

    // 1 获得视口范围
    // 获取视口范围
    // auto& pts = buffer_["/gnss"];
    auto& pts = buffer_["/fused_state"];

    std::vector<ImPlotPoint> pts_all;
    pts_all.reserve(pts.size());

    for (auto const& pt : pts) {
      Eigen::Isometry3d const* rpose = nullptr;
      if (pt->channel_type_ == ylt::reflection::type_string<Gnss>()) {
        auto frame = std::dynamic_pointer_cast<Message<Gnss> const>(pt);
        rpose = &frame->rpose_;
      } else if (pt->channel_type_ == ylt::reflection::type_string<State>()) {
        auto frame = std::dynamic_pointer_cast<Message<State> const>(pt);
        rpose = &frame->rpose_;
      }
      pts_all.emplace_back(ImPlotPoint{ rpose->translation().x(), rpose->translation().y() });
    }

    // 降采样
    std::vector<ImPlotPoint> pts_downsample;
    int const size_10hz = pts_all.size() / 20;
    pts_downsample.reserve(size_10hz);
    PointLttb::Downsample(pts_all.begin(), pts_all.size(), std::back_inserter(pts_downsample), size_10hz);

    // 视口范围内
    std::vector<ImPlotPoint> pts_viewport;
    pts_viewport.reserve(1000);

    auto range = ImPlot::GetPlotLimits();
    for (auto const& pt : pts_downsample) {
      if (pt.x < range.X.Min || pt.x > range.X.Max) continue;
      if (pt.y < range.Y.Min || pt.y > range.Y.Max) continue;
      pts_viewport.emplace_back(pt);
    }

    ImPlot::PlotLine("line", &pts_viewport[0].x, &pts_viewport[0].y, pts_viewport.size(), 0, 0, sizeof(ImPlotPoint));
    ImPlot::PlotScatter(
      "scatter", &pts_viewport[0].x, &pts_viewport[0].y, pts_viewport.size(), 0, 0, sizeof(ImPlotPoint));

    // ImPlot::PlotLineG("gnss_line", get_data, &buffer_["/gnss"], buffer_["/gnss"].size());
    // ImPlot::PlotScatterG("gnss_scatter", get_data, &buffer_["/gnss"], buffer_["/gnss"].size());

    // ImPlot::PlotLineG("fused_state_line", get_data, &buffer_["/fused_state"], buffer_["/fused_state"].size());
    // ImPlot::PlotScatterG("fused_state_scatter", get_data, &buffer_["/fused_state"], buffer_["/fused_state"].size());

    ImPlot::EndPlot();
  }

  ImGui::End();
}

void MyViewer::DrawTrajectory(MessageBuffer const& single_buffer) {
  static std::unordered_map<std::string_view, std::vector<ImPlotPoint>> down_pts;
}
