#pragma once

#include "common/extral_units.hpp"
#include "common/eigen_type.hpp"
#include "message/message_buffer.h"
#include <eigen3/Eigen/Dense>


namespace lm::filter {

inline Eigen::Matrix3d askew(Eigen::Vector3d const& v) {
  return (Eigen::Matrix3d() << 0, -v(2), v(1), v(2), 0.0, -v(0), -v(1), v(0), 0).finished();
}

struct State16 : public Eigen::Vector<double, 16> {
  State16() {
    this->setZero();
    this->qua().setIdentity();
  }

  State16(State const& s)
    : State16() {
    this->qua() = Eigen::Quaterniond(Eigen::EulerAnglesZXYd(s.att_.Map3d()));
    this->pos() = s.pos_.Map3d();
    this->vel() = s.vel_.Map3d();
  }
  enum class Idx { qua = 0, dv = 4, dp = 7, dbg = 10, dba = 13 };
  Eigen::QuaternionMapd qua() { return Eigen::QuaternionMapd(this->data() + (int)Idx::qua); }
  Eigen::VectorMap3d vel() { return Eigen::VectorMap3d(this->data() + (int)Idx::dv); }
  Eigen::VectorMap3d pos() { return Eigen::VectorMap3d(this->data() + (int)Idx::dp); }
  Eigen::VectorMap3d bg() { return Eigen::VectorMap3d(this->data() + (int)Idx::dbg); }
  Eigen::VectorMap3d ba() { return Eigen::VectorMap3d(this->data() + (int)Idx::dba); }
  //
  Eigen::CQuaternionMapd qua() const { return Eigen::CQuaternionMapd(this->data() + (int)Idx::qua); }
  Eigen::CVectorMap3d vel() const { return Eigen::CVectorMap3d(this->data() + (int)Idx::dv); }
  Eigen::CVectorMap3d pos() const { return Eigen::CVectorMap3d(this->data() + (int)Idx::dp); }
  Eigen::CVectorMap3d bg() const { return Eigen::CVectorMap3d(this->data() + (int)Idx::dbg); }
  Eigen::CVectorMap3d ba() const { return Eigen::CVectorMap3d(this->data() + (int)Idx::dba); }

  template <typename OtherDerived>
  State16& operator=(const Eigen::MatrixBase<OtherDerived>& other) {
    this->Base::operator=(other);
    return *this;
  }

  State16 Update(Message<Imu> const& frame, double dt) {
    using namespace Eigen;
    State16 re = *this;
    Vector3d const vel_inc = frame.msg_.acc_.Map3d() * dt;
    Vector3d const ang_inc = frame.msg_.gyr_.Map3d() * dt;
    Vector3d const vel_rot = 1.0 / 2.0 * ang_inc.cross(vel_inc);
    // pos vel att
    re.vel() += qua() * (vel_inc + vel_rot) + Vector3d{ 0, 0, -gl_g0 } * dt;
    re.pos() += 0.5 * (re.vel() + this->vel()) * dt;
    re.qua() = this->qua() * AngleAxisd{ ang_inc.norm(), ang_inc.normalized() };
    return re;
  }
};

struct ErrorState15 : public Eigen::Vector<double, 15> {
  using Base = Eigen::Matrix<double, 15, 1>;
  constexpr static unsigned klocal = 15;
  ErrorState15() { this->setZero(); }
  enum class Idx { fai = 0, dv = 3, dp = 6, dbg = 9, dba = 12 };
  Eigen::VectorMap3d fai() { return Eigen::VectorMap3d(this->data() + (int)Idx::fai); }
  Eigen::VectorMap3d dv() { return Eigen::VectorMap3d(this->data() + (int)Idx::dv); }
  Eigen::VectorMap3d dp() { return Eigen::VectorMap3d(this->data() + (int)Idx::dp); }
  Eigen::VectorMap3d dbg() { return Eigen::VectorMap3d(this->data() + (int)Idx::dbg); }
  Eigen::VectorMap3d dba() { return Eigen::VectorMap3d(this->data() + (int)Idx::dba); }
  //
  Eigen::CVectorMap3d fai() const { return Eigen::CVectorMap3d(this->data() + (int)Idx::fai); }
  Eigen::CVectorMap3d dv() const { return Eigen::CVectorMap3d(this->data() + (int)Idx::dv); }
  Eigen::CVectorMap3d dp() const { return Eigen::CVectorMap3d(this->data() + (int)Idx::dp); }
  Eigen::CVectorMap3d dbg() const { return Eigen::CVectorMap3d(this->data() + (int)Idx::dbg); }
  Eigen::CVectorMap3d dba() const { return Eigen::CVectorMap3d(this->data() + (int)Idx::dba); }
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
  re.qua() = Eigen::AngleAxisd{ dx.fai().norm(), dx.fai().normalized() } * x.qua();
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
    using namespace Eigen;
    using namespace units;
    using namespace units::literals;
    using namespace units::angular_velocity;
    using namespace units::angle;
    using namespace units::velocity;
    using namespace units::length;
    using namespace units::acceleration;
    using namespace units::angle_random_walk;
    states_.t0_ = frame->t0();
    states_.x_ = { frame->msg_ };
    states_.dx_.setZero();

    // init cov
    Eigen::Vector3d v3_ones = Vector3d::Ones();
    QType std0 = QType::Zero();
    std0 << Vector3d::Constant(SI(5.0_deg)), Vector3d::Constant(SI(1.0_mps)), Vector3d::Constant(SI(1.0_m)),
      Vector3d::Constant(SI(100.0_dph)), Vector3d::Constant(SI(0.005_SG));

    states_.cov_ = std0.cwiseAbs2().asDiagonal();
    // q
    q_.setZero();
    q_ << Vector3d::Constant(SI(0.1_dpsh)), Vector3d::Constant(SI(10.0_ugpshz)), Vector<double, 15 - 6>::Zero();
  }

  std::shared_ptr<FStates> TimeUpdate(Message<Imu> const& frame) {
    auto predicted_states = std::make_shared<FStates>(states_);
    double const dt = frame.t0() - states_.t0_;
    predicted_states->t0_ = frame.t0();

    // 0. 误差状态更新
    FaiType const& Phi = Fai(states_, frame, dt);
    if (!states_.dx_.isZero()) { predicted_states->dx_ = Phi * states_.dx_; }

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

public:
  FStates states_;
  QType q_ = QType::Zero();
};

using ESKF15 = ErrorStateKalmanFilter<100, State16, ErrorState15>;

}
