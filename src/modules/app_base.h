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

  std::string name_ = "";

private:
  MultuiSensorFusion* msf_;
};
