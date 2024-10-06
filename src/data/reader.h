#pragma once
#include "message/message_buffer.h"

class IDataReader {
public:
  enum class IOState { OK, END };
  virtual std::pair<MessageBase::SPtr, IOState> ReadFrame() = 0;
  virtual void Init(std::string const& path) = 0;
};