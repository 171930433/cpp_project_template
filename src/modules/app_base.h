#pragma once
#include "message/message_buffer.h"

class AppBase {
public:
  using SPtr = std::shared_ptr<AppBase>;

  virtual void Init() = 0;
  virtual ~AppBase() = default;
public:
  std::string name_ = "";
};
