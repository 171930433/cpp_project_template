#include "releative_loc/releative_loc.h"
#include <ylt/easylog.hpp>

namespace lm {

ReleativeLoc::ReleativeLoc() { initializer_ = std::make_unique<Initializer>(); }

Message<State>::SPtr ReleativeLoc::ProcessImpl(Message<Imu>::SCPtr frame) {
  if (!inited_) { return nullptr; }

  auto re = CreateMessage<State>("/releative_loc/pose");
  return re;
}

void ReleativeLoc::ProcessImpl(std::shared_ptr<const Message<Gnss>> frame) {}

void ReleativeLoc::ProcessImpl(std::shared_ptr<const Message<State>> frame) {

  ELOGD << "get another init message " << frame->to_header_str();
}

}