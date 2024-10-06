#include "viewer.h"
#include "modules/psins/psins_app.h"
#include "tree_view_helper.h"

MyViewer::MyViewer() {
  reader_ = std::make_shared<PsinsReader>();
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
  std::string config_str = FLAGS_config_dir;
  ImGui::InputText("ConfigDir", &config_str);
  ImGui::SameLine();
  if (ImGui::SmallButton("...##2")) {
    auto dialog = pfd::select_folder("Select a file", "/home/gsk/pro/cpp_project_template/config/demo");
    if (!dialog.result().empty()) { FLAGS_config_dir = dialog.result(); }
  }

  // 初始化
  if (!FLAGS_data_dir.empty() && !FLAGS_config_dir.empty()) {

    if (ImGui::Button("Init##")) {
      reader_->Init(FLAGS_data_dir);
      msf_.Init();
      msf_.CreateModule<PsinsApp>();

      Message<State>::CFunc cbk = [this](Message<State>::SCPtr frame) { fused_states_.push_back(frame); };
      msf_.dispatcher()->RegisterWriter("/fused_state", cbk);
    }
  }

  ImGuiTreeNodeFlags node_flags =
    ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_OpenOnDoubleClick | ImGuiTreeNodeFlags_SpanAvailWidth;

  TreeView(*msf_.cm(), node_flags);

  // 运行
  if (ImGui::Button("process")) {
    for (auto it = reader_->ReadFrame(); it.second != IDataReader::IOState::END; it = reader_->ReadFrame()) {
      msf_.ProcessData(it.first);
    }
  }

  ImGui::LabelText("fused_state size", "%ld", fused_states_.size());

  ImGui::End();
}