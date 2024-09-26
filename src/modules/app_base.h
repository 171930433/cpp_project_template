#pragma once
#include "message/message_buffer.h"

class AppBase {
public:
  virtual void Init() = 0;
  virtual ~AppBase() = default;
};
