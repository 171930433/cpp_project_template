#include "releative_loc/releative_loc.h"
#include <ylt/easylog.hpp>

namespace lm {

ReleativeLoc::ReleativeLoc() { initializer_ = std::make_unique<Initializer>(); }

Message<State>::SPtr ReleativeLoc::ProcessImpl(Message<Imu>::SCPtr frame) {
  if (!inited_) { return nullptr; }

  auto filter_state = eskf_.TimeUpdate(*frame);

  auto re = CreateMessage<State>("/releative_loc/pose");

  re->rpose_ = Eigen::Translation3d(filter_state->x_.pos()) * filter_state->x_.qua();
  re->msg_.pos_.Map3d().setZero();
  re->msg_.vel_.Map3d().setZero();
  re->msg_.att_.Map3d().setZero();

  return re;
}

void ReleativeLoc::ProcessImpl(std::shared_ptr<Message<Gnss> const> frame) {}

void ReleativeLoc::ProcessImpl(std::shared_ptr<Message<State> const> frame) {
  ELOGD << "get another init message " << frame->to_header_str();
}

bool ReleativeLoc::TryInit(MessageBase::SCPtr frame) {
  if (frame->channel_type_ != ylt::reflection::get_struct_name<State>()) { return false; }

  auto re = CreateMessage<State>("/releative_loc/pose");
  *re = *std::dynamic_pointer_cast<Message<State> const>(frame);
  re->UpdateRelativePose();

  eskf_.Init(re);

  return true;
}

}