#pragma once
#include "message/message_buffer.h"

class IDataReader {
public:
  enum class State { OK, END };
  virtual std::pair<MessageBase::SPtr, State> ReadFrame() = 0;
};