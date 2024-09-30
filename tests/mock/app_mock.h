#pragma once
#include <gmock/gmock.h>

#include "modules/app_base.h"

class DemoModule : public AppBase {
public:
  using SPtr = std::shared_ptr<DemoModule>;
  DemoModule() { name_ = "DemoModule"; }

public:
  void Init() override {
    GTEST_LOG_(INFO) << "DemoModule init done";

    dispatcher()->RegisterReader("/imu", &DemoModule::ProcessImu, this);
    // dispatcher()->RegisterReader("/gnss", &DemoModule::ProcessGnss, this);
    dispatcher()->RegisterReader("/gnss", &DemoModule::ProcessImu, this);
  }
  virtual void ProcessImu(std::shared_ptr<const Message<Imu>> frame) = 0;
  virtual void ProcessGnss(std::shared_ptr<const Message<Gnss>> frame) = 0;
  void Write() {
    auto state = Message<State>::Create("/state");
    state->msg_.t0_ = 1;
    WriteMessage(state);
  }
};

class MockDemoModule : public DemoModule {
public:
  using SPtr = std::shared_ptr<MockDemoModule>;
  MOCK_METHOD(void, ProcessImu, (std::shared_ptr<const Message<Imu>>), ());
  MOCK_METHOD(void, ProcessGnss, (std::shared_ptr<const Message<Gnss>>), ());
};