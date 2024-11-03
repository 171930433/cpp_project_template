#pragma once
#include "common/earth.hpp"
#include "message/message_buffer.h"
#include "message/message_dispatcher.h"
#include "mylib.h"

class AppBase {
public:
  using SPtr = std::shared_ptr<AppBase>;

  void InitModule(MultuiSensorFusion* msf) {
    msf_ = msf;
    Init();
  }
  virtual void Init() = 0;
  virtual ~AppBase() = default;
  Dispatcher* dispatcher() { return &msf_->dispatcher_; }
  ChannelConfigs* io_cfg() { return &msf_->cm_.io_; }
  tf::Executor* executor() { return &msf_->executor_; }
  void WriteMessage(MessageBase::SCPtr frame);

  std::string name_ = "";

private:
  MultuiSensorFusion* msf_;
};

inline void AppBase::WriteMessage(MessageBase::SCPtr frame) {
  // 先给所有订阅的module一份
  dispatcher()->ForeachReaders(frame);

  // 写出到外部回调
  dispatcher()->ForeachWriters(frame);
}
