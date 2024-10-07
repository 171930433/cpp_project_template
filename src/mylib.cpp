#include "mylib.h"
#include "modules/app_base.h"

DEFINE_string(config_dir, "", "config dir");
DEFINE_string(data_dir, "", "data dir");

MultuiSensorFusion::MultuiSensorFusion(std::string const& path) {
  Init(path);
}

void MultuiSensorFusion::Init(std::string const& path) {
  // CHECK(!path.empty()) << "when called MultuiSensorFusion:Init(), you should pass --config_dir=/path/to/cm.json";
  // LOG_IF(INFO, path.empty()) << "when called MultuiSensorFusion:Init(), you should pass
  // --config_dir=/path/to/cm.json";

  if (path.empty()) {
    ELOGE<<" FLAGS_config_dir is empty. "<< "when called MultuiSensorFusion:Init(), you should pass --config_dir=/path/to/cm.json";
  }

  std::string cm_path = path + "/cm.json";
  cm_.LoadFromDir(cm_path);

  dispatcher_.Init(&executor_);
}

int add(int const x, int const y) {
  return x + y;
}

int minus(int const x, int const y) {
  return x - y;
}
