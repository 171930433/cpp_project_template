#include "viewer.h"
#include "lttb.h"
#include "modules/psins/psins_app.h"
#include "tree_view_helper.h"

#ifdef FF
#undef FF
#endif

#include "down_sample.h"

MyViewer::MyViewer() { reader_ = std::make_unique<PsinsReader>(); }

MyViewer::~MyViewer() {
  stop_ = false;
  exit_ = true;
  executor_.wait_for_all();
};

void MyViewer::Init() {
  reader_->Init(FLAGS_data_dir);
  msf_.Init();
  msf_.CreateModule<PsinsApp>();

  Message<State>::CFunc cbk = [this](Message<State>::SCPtr frame) {
    // buffer_[frame->channel_name_].push_back(frame);
    buffers_.Append(frame);
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

  ProjectWindow();

  TrajectoryWindow();

  Plot2dWindow();
}

void MyViewer::ProjectWindow() {
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
          buffers_.Append(it.first);
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
  ImGui::LabelText("fused_state size", "%ld", buffers_.Get<State>("/fused_state").size());

  ImGui::End();
}

template <typename _Sensor, std::enable_if_t<IsTrajectory_v<_Sensor>>* = nullptr>
void PlotTrajectory(SensorContainer<_Sensor> const& single_buffer) {
  auto const& pts = DownSample(single_buffer);
  if (pts.empty()) return;
  ImPlot::PlotScatter(single_buffer.channel_name_.data(), &pts[0].x, &pts[0].y, pts.size(), 0, 0, sizeof(ImPlotPoint));
}

void MyViewer::TrajectoryWindow() {
  ImGui::Begin("trj");

  auto plot_flag = ImPlotFlags_Equal;
  if (ImGui::Button("load pins results")) {
    auto& pins_re = buffers_.Get<State>("/psins/state");
    if (pins_re.empty()) { PsinsReader::LoadResult("/home/gsk/pro/cpp_project_template/data/ins.bin", pins_re); }
  }
  if (ImPlot::BeginPlot("##0", ImVec2(-1, -1), plot_flag)) {

    PlotTrajectory(buffers_.Get<State>("/fused_state"));
    PlotTrajectory(buffers_.Get<Gnss>("/gnss"));

    PlotTrajectory(buffers_.Get<State>("/psins/state"));

    ImPlot::EndPlot();
  }

  ImGui::End();
}

void MyViewer::Plot2dWindow() {
  ImGui::Begin("plot 2d");

  if (ImPlot::BeginPlot("##CustomRend", ImVec2(-1, -1), ImPlotFlags_Equal)) {
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
