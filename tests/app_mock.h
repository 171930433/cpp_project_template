#pragma once
#include <gmock/gmock.h>

#include "message/message_buffer.h"
#include "modules/app_base.h"

class DemoModule : public AppBase {
public:
  using SPtr = std::shared_ptr<DemoModule>;
  DemoModule() { name_ = "DemoModule"; }

public:
  void Init() override {
    LOG(INFO) << "DemoModule init done";

    RegisterReader("/imu", &DemoModule::ProcessImu, this);
    RegisterReader("/gnss", &DemoModule::ProcessGnss, this);
  }
  virtual void ProcessImu(std::shared_ptr<const ChannelMsg<Imu>> frame) = 0;
  virtual void ProcessGnss(std::shared_ptr<const ChannelMsg<Gnss>> frame) = 0;
};


class MockDemoModule : public DemoModule {
public:
  using SPtr = std::shared_ptr<MockDemoModule>;
  MOCK_METHOD(void, ProcessImu, (std::shared_ptr<const ChannelMsg<Imu>>), ());
  MOCK_METHOD(void, ProcessGnss, (std::shared_ptr<const ChannelMsg<Gnss>>), ());
};