#pragma once
#include "message/message_buffer.h"
namespace lm {

class Initializer {
public:
  bool TryInit(MessageBase::SCPtr frame);
  void InitByState(std::shared_ptr<const Message<State>> frame);
  Message<State>::SCPtr InitedState() { return inited_state_; }

private:
  TotalBuffer<Imu, Gnss, State> buffers_;
  Message<State>::SPtr inited_state_;
};

}