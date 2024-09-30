#pragma once

#include <ylt/struct_json/json_reader.h>
#include <ylt/struct_json/json_writer.h>

#include "config/channel_config.h"
#include "config/sensor_setting.h"

struct ConfigManager {
  ChannelConfigs io_;
  //   SensorSetting ss_;
  void LoadFromDir(std::string const& path) { iguana::from_json_file(*this, path); }
};
