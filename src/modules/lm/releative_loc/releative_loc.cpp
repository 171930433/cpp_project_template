#include "releative_loc/releative_loc.h"
#include <ylt/easylog.hpp>

namespace lm {

ReleativeLoc::ReleativeLoc() { initializer_ = std::make_unique<Initializer>(); }

Message<State>::SPtr ReleativeLoc::ProcessImpl(Message<Imu>::SCPtr frame) {
  if (!inited_) { return nullptr; }

  auto filter_state = eskf_.TimeUpdate(*frame);

  auto re = CreateMessage<State>("/releative_loc/pose");

  re->msg_.t0_ = filter_state->t0_;
  re->msg_.pos_.Map3d() = filter_state->x_.pos();
  re->msg_.vel_.Map3d() = filter_state->x_.vel();
  re->msg_.att_.Map3d() = Eigen::EulerAnglesZXYd(filter_state->x_.qua()).angles();
  re->UpdateRelativePose(false);

  ELOGD << "ReleativeLoc::ProcessImpl " << re->to_header_str();

  return re;
}

void ReleativeLoc::ProcessImpl(std::shared_ptr<Message<Gnss> const> frame) {}

void ReleativeLoc::ProcessImpl(std::shared_ptr<Message<State> const> frame) {
  ELOGD << "get another init message " << frame->to_header_str();
}

bool ReleativeLoc::TryInit(MessageBase::SCPtr frame) {
  if (frame->channel_type_ != ylt::reflection::get_struct_name<State>()) { return false; }

  auto re = CreateMessage<State>("/releative_loc/pose");
  re->msg_ = std::dynamic_pointer_cast<Message<State> const>(frame)->msg_;
  re->UpdateRelativePose(); // 设置轨迹的圆心

  // 设置初始pose
  re->msg_.pos_.Map3d() = re->rpose_.translation();
  re->msg_.att_.Map3d() = re->rpose_.rotation().eulerAngles(2, 0, 1);

  eskf_.Init(re);

  ELOGD << "ReleativeLoc::TryInit done" << frame->to_json();
  ELOGD << "inited state is" << re->to_json();

  return true;
}

}