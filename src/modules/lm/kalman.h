#pragma once

#include "message/message_buffer.h"
#include <eigen3/Eigen/Dense>

namespace lm::filter {

inline Eigen::Matrix3d askew(Eigen::Vector3d const& v) {
  return (Eigen::Matrix3d() << 0, -v(2), v(1), v(2), 0.0, -v(0), -v(1), v(0), 0).finished();
}

struct State16 : public Eigen::Matrix<double, 16, 1> {
  enum class Idx { qua = 0, dv = 4, dp = 7, dbg = 10, dba = 13 };
  auto qua() { return Eigen::Map<Eigen::Quaterniond>(this->data()); }
  auto vel() { return Eigen::Map<Eigen::Vector3d>(this->data() + 4); }
  auto pos() { return Eigen::Map<Eigen::Vector3d>(this->data() + 7); }
  auto bg() { return Eigen::Map<Eigen::Vector3d>(this->data() + 10); }
  auto ba() { return Eigen::Map<Eigen::Vector3d>(this->data() + 13); }
  //
  auto qua() const { return Eigen::Map<Eigen::Quaterniond const>(this->data()); }
  auto vel() const { return Eigen::Map<Eigen::Vector3d const>(this->data() + 4); }
  auto pos() const { return Eigen::Map<Eigen::Vector3d const>(this->data() + 7); }
  auto bg() const { return Eigen::Map<Eigen::Vector3d const>(this->data() + 10); }
  auto ba() const { return Eigen::Map<Eigen::Vector3d const>(this->data() + 13); }
};

template <typename _State>
std::shared_ptr<_State> StateUpdate(_State const& x, Message<Imu> const& frame) {
  return nullptr;
}

struct ErrorState15 : public Eigen::Matrix<double, 15, 1> {
  constexpr static unsigned klocal = 15;
  enum class Idx { fai = 0, dv = 3, dp = 6, dbg = 9, dba = 12 };
  auto fai() { return Eigen::Map<Eigen::Vector3d>(this->data() + (int)Idx::fai); }
  auto dv() { return Eigen::Map<Eigen::Vector3d>(this->data() + (int)Idx::dv); }
  auto dp() { return Eigen::Map<Eigen::Vector3d>(this->data() + (int)Idx::dp); }
  auto dbg() { return Eigen::Map<Eigen::Vector3d>(this->data() + (int)Idx::dbg); }
  auto dba() { return Eigen::Map<Eigen::Vector3d>(this->data() + (int)Idx::dba); }
  //
  auto fai() const { return Eigen::Map<Eigen::Vector3d const>(this->data() + (int)Idx::fai); }
  auto dv() const { return Eigen::Map<Eigen::Vector3d const>(this->data() + (int)Idx::dv); }
  auto dp() const { return Eigen::Map<Eigen::Vector3d const>(this->data() + (int)Idx::dp); }
  auto dbg() const { return Eigen::Map<Eigen::Vector3d const>(this->data() + (int)Idx::dbg); }
  auto dba() const { return Eigen::Map<Eigen::Vector3d const>(this->data() + (int)Idx::dba); }
};

template <typename _State, typename _ErrorState>
struct StatePair {
  using CovType = Eigen::Matrix<double, _ErrorState::klocal, _ErrorState::klocal>;
  double t0_;
  _State x_;
  _ErrorState dx_;
  CovType cov_ = CovType::Zero();
};

template <unsigned _rate_hz, typename _State, typename _ErrorState>
class ErrorStateKalmanFilter {
public:
  constexpr static unsigned klocal = _ErrorState::klocal;
  using FaiType = Eigen::Matrix<double, klocal, klocal>;
  using States = StatePair<_State, _ErrorState>;

  ErrorStateKalmanFilter() = default;

  std::shared_ptr<States> TimeUpdate(Message<Imu> const& frame) {
    auto predicted_states = std::make_shared<States>(*states_);
    predicted_states->t0_ = frame.t0();

    auto Phi = Fai(states_, frame);
    if (!(states_.dx_.all() == 0)) { predicted_states->dx_ = Phi * states_.dx_; }
    predicted_states->cov_ = Phi.transpose() * states_.cov_ * Phi /*过程噪声部分先忽略*/;
    predicted_states->x_ = StateUpdate(states_.x_, frame);

    return predicted_states;
  }

protected:
  FaiType Fai(States const& states, Message<Imu> const& frame) {
    static FaiType const Inn = FaiType::Identity();
    auto& x = states.x_;
    double const dt = frame.t0() - states.t0_;

    FaiType F = FaiType::Zero();

    Eigen::Vector3d const fn = x.qua() * ToVector3d(frame.msg_.acc_);
    Eigen::Matrix3d const Cnb = x.qua().toRotationMatrix();

    F.block<3, 3>(_State::Idx::fai, _State::Idx::dbg) = -Cnb;
    F.block<3, 3>(_State::Idx::dv, _State::Idx::fai) = askew(fn);
    F.block<3, 3>(_State::Idx::dv, _State::Idx::dba) = Cnb;
    F.block<3, 3>(_State::Idx::dp, _State::Idx::dv) = Eigen::Matrix3d::Identity();

    //   相关时间暂时不管
    FaiType Phi = Inn + F * dt;

    return Phi;
  };

protected:
  States states_;
};

using ESKF15 = ErrorStateKalmanFilter<100, State16, ErrorState15>;

}
