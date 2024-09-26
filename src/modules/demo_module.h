#pragma once
#include "message/message_buffer.h"
#include "modules/app_base.h"

class DemoModule : public AppBase {
public:
  void Init() override { LOG(INFO) << "DemoModule init done"; }
  void ProcessImu(ChannelMsg<Imu>::SPtr frame) {
    LOG(INFO) << "got an imu " << fmt::format("{:.3f}", frame->t0());
  }
};
