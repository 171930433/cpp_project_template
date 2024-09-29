#pragma once
#include "message/message_buffer.h"
#include "modules/app_base.h"

class DemoModule : public AppBase {
public:
  using SPtr = std::shared_ptr<DemoModule>;

public:
  void Init() override {
    name_ = "DemoModule";
    LOG(INFO) << "DemoModule init done";
  }
  void ProcessImu(ChannelMsg<Imu>::SPtr frame) { LOG(INFO) << "got an imu " << fmt::format("{:.3f}", frame->t0()); }
};
