#pragma once
#include "message/message_buffer.h"
#include "message/message_callback.h"
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
  MesageIO* io() { return msf_->io(); }

  void WriteMessage(MessageBase::SCPtr frame);

  std::string name_ = "";

private:
  MultuiSensorFusion* msf_;
};

inline void AppBase::WriteMessage(MessageBase::SCPtr frame) {
  // 先给所有订阅的module一份
  auto& reader = io()->reader_;
  if (reader.contains(frame->channel_name_)) {
    for (auto& cbk : reader[frame->channel_name_]) { cbk(frame); }
  }
  // 写出到外部回调
  auto& writer = io()->writer_;
  if (writer.contains(frame->channel_name_)) {
    for (auto& cbk : writer[frame->channel_name_]) { cbk(frame); }
  }
}
