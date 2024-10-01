#include "mylib.h"

DEFINE_string(config_dir, "", "config dir");

MultuiSensorFusion::MultuiSensorFusion(std::string const& path) {
  Init(path);
}

void MultuiSensorFusion::Init(std::string const& path = FLAGS_config_dir) {
  CHECK(!path.empty()) << "when called MultuiSensorFusion:Init(), you should pass --config_dir=/path/to/cm.json";

  std::string cm_path = path + "/cm.json";
  cm_.LoadFromDir(cm_path);
}

int add(int const x, int const y) {
  return x + y;
}

int minus(int const x, int const y) {
  return x - y;
}
