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

  State16 Update(Message<Imu> const& frame, double dt) {
    using namespace Eigen;
    State16 re = *this;
    Vector3d const vel_inc = frame.msg_.acc_.Map3d() * dt;
    Vector3d const ang_inc = frame.msg_.gyr_.Map3d() * dt;
    Vector3d const vel_rot = 1.0 / 2.0 * ang_inc.cross(vel_inc);
    // pos vel att
    re.vel() += qua() * (vel_inc + vel_rot) + Vector3d{ 0, 0, gl_g0 } * dt;
    re.pos() += 0.5 * (re.vel() + this->vel());
    re.qua() = AngleAxisd{ ang_inc.norm(), ang_inc / ang_inc.norm() } * this->qua();
    return re;
  }
};

struct ErrorState15 : public Eigen::Matrix<double, 15, 1> {
  using Base = Eigen::Matrix<double, 15, 1>;
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
  // This method allows you to assign Eigen expressions to MyVectorType
  // https://eigen.tuxfamily.org/dox/TopicCustomizing_InheritingMatrix.html
  template <typename OtherDerived>
  ErrorState15& operator=(const Eigen::MatrixBase<OtherDerived>& other) {
    this->Base::operator=(other);
    return *this;
  }
};

inline State16 Compensate(State16 const& x, ErrorState15 const& dx) {
  State16 re;

  re.qua() = Eigen::AngleAxisd{ dx.fai().norm(), dx.fai() / dx.fai().norm() } * x.qua();
  re.tail(12) = x.tail(12) + dx.tail(12);

  return re;
}

template <typename _State, typename _ErrorState>
struct FilterState {
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
  using QType = Eigen::Matrix<double, klocal, 1>;
  using FStates = FilterState<_State, _ErrorState>;

  ErrorStateKalmanFilter() = default;

  void Init(std::shared_ptr<Message<State> const> frame) {
    states_.t0_ = frame->t0();
    states_.x_.pos() = Eigen::Vector3d::Zero();
    states_.x_.qua() = Eigen::Quaterniond::Identity();
    states_.x_.pos() = Eigen::Vector3d{ 0, frame->msg_.vel_.Map3d().norm(), 0 };
  }

  std::shared_ptr<FStates> TimeUpdate(Message<Imu> const& frame) {
    auto predicted_states = std::make_shared<FStates>(states_);
    double const dt = frame.t0() - states_.t0_;
    predicted_states->t0_ = frame.t0();

    // 0. 误差状态更新
    FaiType const& Phi = Fai(states_, frame, dt);
    if (!(states_.dx_.all() == 0)) { predicted_states->dx_ = Phi * states_.dx_; }

    // 1. 方差更新
    FaiType const cov = Phi.transpose() * states_.cov_ * Phi + FaiType(q_.asDiagonal() * dt);
    predicted_states->cov_ = (cov.transpose() + cov) * 0.5;

    // 2. 全状态更新
    predicted_states->x_ = states_.x_.Update(frame, dt);

    // 更新当前滤波状态
    states_ = *predicted_states;

    return predicted_states;
  }

protected:
  FaiType Fai(FStates const& states, Message<Imu> const& frame, double dt) {
    static FaiType const Inn = FaiType::Identity();
    auto& x = states.x_;

    FaiType F = FaiType::Zero();

    Eigen::Vector3d const fn = x.qua() * frame.msg_.acc_.Map3d();
    Eigen::Matrix3d const Cnb = x.qua().toRotationMatrix();

    using Idx = _ErrorState::Idx;
    F.template block<3, 3>((int)Idx::fai, (int)Idx::dbg) = -Cnb;
    F.template block<3, 3>((int)Idx::dv, (int)Idx::fai) = askew(fn);
    F.template block<3, 3>((int)Idx::dv, (int)Idx::dba) = Cnb;
    F.template block<3, 3>((int)Idx::dp, (int)Idx::dv) = Eigen::Matrix3d::Identity();

    //   相关时间暂时不管
    FaiType Phi = Inn + F * dt;

    return Phi;
  };

protected:
  FStates states_;
  QType q_ = QType::Zero();
};

using ESKF15 = ErrorStateKalmanFilter<100, State16, ErrorState15>;

}
