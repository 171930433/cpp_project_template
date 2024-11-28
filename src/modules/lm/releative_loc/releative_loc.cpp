#include "releative_loc/releative_loc.h"
#include <boost/circular_buffer.hpp>
#include <common/extral_units.hpp>
#include <ylt/easylog.hpp>

namespace lm {

ReleativeLoc::ReleativeLoc() { initializer_ = std::make_unique<Initializer>(); }

Message<State>::SPtr ReleativeLoc::ProcessImpl(Message<Imu>::SCPtr frame) {
  if (!inited_) { return nullptr; }

  // ELOGD <<" ReleativeLoc::ProcessImpl imu is " << frame->to_json();

  bool is_static = StaticCheck(*frame);

  auto filter_state = eskf_.TimeUpdate(*frame);

  auto re = CreateMessage<State>("/releative_loc/pose");

  // 静止只做时间更新
  if (is_static) {
    re->msg_.t0_ = frame->t0();
    re->msg_ = buffers_.Get<State>(re->channel_name_).back()->msg_;
  } else {
    re->msg_.t0_ = filter_state->t0_;
    re->msg_.pos_.Map3d() = filter_state->x_.pos();
    re->msg_.vel_.Map3d() = filter_state->x_.vel();
    re->msg_.att_.Map3d() = Eigen::EulerAnglesZXYd(filter_state->x_.qua()).angles();
  }

  re->UpdateRelativePose(false);

  ELOGD << "ReleativeLoc::ProcessImpl " << re->to_json();

  buffers_.Append(re);

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
  re->msg_.att_.Map3d() = Eigen::EulerAnglesZXYd(re->rpose_.rotation()).angles();

  eskf_.Init(re);

  ELOGD << "ReleativeLoc::TryInit done" << frame->to_json();
  ELOGD << "inited state is" << re->to_json();

  buffers_.Append(re);

  return true;
}

// 连续1s的均值都小于0.002rad/s， [t0-1,t0]是静止状态
bool ReleativeLoc::StaticCheck(Message<Imu> const& frame) {
  using namespace units;
  using namespace units::literals;
  using namespace units::angular_velocity;
  static boost::circular_buffer<double> all_wz(125);
  all_wz.push_back(frame.msg_.gyr_.z_);

  if (!all_wz.full()) { return false; }

  Eigen::Map<Eigen::Vector<double, 125>> vz(all_wz.linearize());

  if (vz.cwiseAbs().mean() < 0.002_rad_per_s()) { return true; }

  return false;
}

bool ReleativeLoc::StaticFuse(Message<Imu> const& frame) {

  if (int(frame.t0() * 1000) % 1000 == 0) { eskf_.FuseStatic(); }

  return true;
}

}
