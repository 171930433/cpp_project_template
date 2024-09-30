#pragma once
#include <gmock/gmock.h>

#include "modules/app_base.h"

class DemoModule : public AppBase {
public:
  using SPtr = std::shared_ptr<DemoModule>;
  DemoModule() { name_ = "DemoModule"; }

public:
  void Init() override {
    LOG(INFO) << "DemoModule init done";

    io()->RegisterReader("/imu", &DemoModule::ProcessImu, this);
    io()->RegisterReader("/gnss", &DemoModule::ProcessGnss, this);
  }
  virtual void ProcessImu(std::shared_ptr<const Channel<Imu>> frame) = 0;
  virtual void ProcessGnss(std::shared_ptr<const Channel<Gnss>> frame) = 0;
  void Write() {
    auto state = Channel<State>::Create("/state");
    state->msg_.t0_ = 1;
    WriteMessage(state);
  }
};

class MockDemoModule : public DemoModule {
public:
  using SPtr = std::shared_ptr<MockDemoModule>;
  MOCK_METHOD(void, ProcessImu, (std::shared_ptr<const Channel<Imu>>), ());
  MOCK_METHOD(void, ProcessGnss, (std::shared_ptr<const Channel<Gnss>>), ());
};