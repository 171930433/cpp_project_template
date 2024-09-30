#pragma once
#include <string>

struct ChannelConfig {
  std::string channel_name_;
  int rate_hz_ = 0;
  std::string device_name_;
};

struct ChannelConfigs {
  ChannelConfig imu_;
  ChannelConfig gnss_;
  ChannelConfig state_;
};
