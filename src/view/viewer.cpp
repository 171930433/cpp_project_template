#include "viewer.h"
#include "lttb.h"
#include "modules/psins/psins_app.h"
#include "tree_view_helper.h"

#include "modules/lm/lm.h"

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
  msf_.CreateModule<lm::LM>();

  Message<State>::CFunc cbk = [this](Message<State>::SCPtr frame) {
    // buffer_[frame->channel_name_].push_back(frame);
    buffers_.Append(frame);
  };
  msf_.dispatcher()->RegisterWriter("/fused_state", cbk);
  msf_.dispatcher()->RegisterWriter("/releative_loc/pose", cbk);

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

template <typename _Sensor, std::enable_if_t<std::is_same_v<_Sensor, Imu>>* = nullptr>
void PlotImu(SensorContainer<_Sensor> const& single_buffer, std::string_view name) {
  auto const& pts = DownSample(single_buffer);
  if (pts.empty()) return;
  if (name == "acc_") {
    ImPlot::PlotLine((std::string(name) + "x").c_str(), &pts[0][0], &pts[0][1], pts.size(), 0, 0, sizeof(double) * 8);
    ImPlot::PlotLine((std::string(name) + "y").c_str(), &pts[0][0], &pts[0][2], pts.size(), 0, 0, sizeof(double) * 8);
    ImPlot::PlotLine((std::string(name) + "z").c_str(), &pts[0][0], &pts[0][3], pts.size(), 0, 0, sizeof(double) * 8);
  } else if (name == "gyr_") {
    ImPlot::PlotLine((std::string(name) + "x").c_str(), &pts[0][0], &pts[0][4], pts.size(), 0, 0, sizeof(double) * 8);
    ImPlot::PlotLine((std::string(name) + "y").c_str(), &pts[0][0], &pts[0][5], pts.size(), 0, 0, sizeof(double) * 8);
    ImPlot::PlotLine((std::string(name) + "z").c_str(), &pts[0][0], &pts[0][6], pts.size(), 0, 0, sizeof(double) * 8);
  }
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
    PlotTrajectory(buffers_.Get<State>("/releative_loc/pose"));
    PlotTrajectory(buffers_.Get<Gnss>("/gnss"));

    PlotTrajectory(buffers_.Get<State>("/psins/state"));

    ImPlot::EndPlot();
  }

  ImGui::End();
}

ImPlotPoint SinewaveGetter(int i, void* data) {
  float f = *(float*)data;
  return ImPlotPoint(i, sinf(f * i));
}

ImPlotPoint ImuGetter(int i, void* data) {
  auto& frame = *(reinterpret_cast<bm::SensorContainer<Imu>*>(data));

  return ImPlotPoint(frame[i]->t0(), frame[i]->msg_.acc_.x_);
}

void MyViewer::Plot2dWindow() {
  ImGui::Begin("plot 2d");

  auto const& channel_names_ = buffers_.channel_names_;
  static auto selected_it = channel_names_.begin();
  if (ImGui::BeginCombo("##channels", (selected_it == channel_names_.end() ? nullptr : selected_it->data()))) {
    for (auto it = channel_names_.begin(); it != channel_names_.end(); ++it) {
      const bool is_selected = (selected_it == it);
      if (ImGui::Selectable(it->data(), is_selected)) selected_it = it;

      if (is_selected) ImGui::SetItemDefaultFocus();
    }
    ImGui::EndCombo();
  }

  ImGui::SameLine();
  static int row_col[2] = { 1, 3 };
  ImGui::InputInt2("row*col", row_col);

  constexpr auto fields = ylt::reflection::get_member_names<Imu>();

  static ImPlotSubplotFlags flags = ImPlotSubplotFlags_ShareItems;
  static int rows = 1;
  static int cols = 2;
  static int id[] = { 0, 1, 2, 3, 4, 5 }; // 每个graph对应的序号
  static int curj = -1;
  if (ImPlot::BeginSubplots("##ItemSharing", rows, cols, ImVec2(-1, 400), flags)) {
    for (int i = 0; i < fields.size(); ++i) {
      if (ImPlot::BeginPlot("")) {
        for (int j = 0; j < fields.size(); ++j) {
          if (id[j] == i) {
            float fj = 0.01f * (j + 2);
            char const* label = std::string(fields[j]).c_str();

            auto& imu_buffer = buffers_.Get<Imu>("/imu");
            PlotImu(imu_buffer, label);

            if (ImPlot::BeginDragDropSourceItem(label)) {
              curj = j;
              ImGui::SetDragDropPayload("MY_DND", nullptr, 0);
              ImPlot::ItemIcon(ImPlot::GetLastItemColor());
              ImGui::SameLine();
              ImGui::TextUnformatted(label);
              ImPlot::EndDragDropSource();
            }
          }
        }
        if (ImPlot::BeginDragDropTargetPlot()) {
          if (ImGui::AcceptDragDropPayload("MY_DND")) id[curj] = i;
          ImPlot::EndDragDropTarget();
        }
        ImPlot::EndPlot();
      }
    }
    ImPlot::EndSubplots();
  }

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
