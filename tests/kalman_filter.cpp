#include <eigen3/Eigen/Dense>
#include <gtest/gtest.h>

#include "modules/lm/kalman.h"
#include <common/eigen_units.h>
#include <ylt/easylog.hpp>
using namespace lm;
using namespace lm::filter;
using namespace Eigen;

using namespace units;
using namespace units::angle;
using namespace units::angular_velocity;
using namespace units::literals;

TEST(ESKF15, units) {
  EXPECT_DOUBLE_EQ(SI(1.0_deg), M_PI / 180);
  EXPECT_DOUBLE_EQ(SI(1.0_dph), M_PI / 180 / 3600);
  EXPECT_DOUBLE_EQ(SI(1.0_rad), 1);
  EXPECT_DOUBLE_EQ(SI(1.0_dpss), M_PI / 180);
  EXPECT_DOUBLE_EQ(SI(1.0_dpsh), M_PI / 180 / 60);
  EXPECT_DOUBLE_EQ(SI(1.0_gpshz), 1e0 * 980665 / 1e5);
  EXPECT_DOUBLE_EQ(SI(1.0_mgpshz), 1e-3 * 980665 / 1e5);
  EXPECT_DOUBLE_EQ(SI(1.0_ugpshz), 1e-6 * 980665 / 1e5);
}

TEST(ESKF15, euler_angle) {
  EulerAnglesZXYd euler1{ SI(10.0_deg), SI(20.0_deg), SI(30.0_deg) };
  ELOGD << "euler1 = " << euler1.angles().transpose() * convert<radian, degree>(1.0);

  Quaterniond q1 = euler1;
  Vector3d euler2 = q1.toRotationMatrix().eulerAngles(2, 0, 1);
  ELOGD << "euler2 = " << euler2.transpose() * convert<radian, degree>(1.0);

  // euler角向量的顺序为yaw pitch roll
  EXPECT_TRUE(euler1.angles().isApprox(euler2));
}

TEST(ESKF15, base) {
  ESKF15 eskf;
  auto state0 = CreateMessage<State>("/init_pose");
  state0->msg_.t0_ = 1;
  state0->msg_.pos_.Map3d() << 1, 1, 1;
  state0->msg_.vel_.Map3d() << 1, 1, 1;
  state0->msg_.att_.Map3d() = Eigen::Quaterniond::Identity().toRotationMatrix().eulerAngles(2, 0, 1);

  eskf.Init(state0);

  EXPECT_TRUE(eskf.states_.x_.pos().isApprox(state0->msg_.pos_.Map3d()));
  EXPECT_TRUE(eskf.states_.x_.vel().isApprox(state0->msg_.vel_.Map3d()));
  EXPECT_TRUE(eskf.states_.x_.qua().isApprox(Eigen::Quaterniond::Identity()));
  EXPECT_TRUE(eskf.states_.dx_.isZero());
  EXPECT_TRUE(!eskf.states_.cov_.isZero());
  EXPECT_TRUE(!eskf.q_.isZero());
}

TEST(ESKF15, time_update) {

  ESKF15 eskf;

  Eigen::Vector3d v0 = Eigen::Vector3d{ 1, 1, 0 }.normalized() * 100;

  auto state0 = CreateMessage<State>("/init_pose");
  state0->msg_.t0_ = 0;
  state0->msg_.pos_.Map3d() << 0, 0, 0;
  state0->msg_.vel_.Map3d() = v0;
  state0->msg_.att_.Map3d() = Eigen::Quaterniond::Identity().toRotationMatrix().eulerAngles(2, 0, 1);

  eskf.Init(state0);

  // 构造imu数据
  auto imu0 = CreateMessage<Imu>("/imu");
  imu0->msg_.t0_ = 0.01;
  imu0->msg_.gyr_.Map3d().setZero();
  imu0->msg_.acc_.Map3d() << 0, 0, gl_g0;

  eskf.TimeUpdate(*imu0);

  ELOGD << "eskf.states_.x_.pos().transpose() = " << eskf.states_.x_.pos().transpose();
  EXPECT_TRUE(eskf.states_.x_.vel().isApprox(v0));
  EXPECT_TRUE(eskf.states_.x_.pos().isApprox(v0 * 0.01));
  EXPECT_TRUE(eskf.states_.x_.qua().isApprox(Eigen::Quaterniond::Identity()));

  // get some angular velocity
  Eigen::Vector3d w0 = Eigen::Vector3d{ 0, 0, 100 * (45_deg_per_s).convert<radians_per_second>()() };

  auto imu1 = CreateMessage<Imu>("/imu");
  imu1->msg_.t0_ = 0.02;
  imu1->msg_.gyr_.Map3d() = w0;
  imu1->msg_.acc_.Map3d() << 0, 0, gl_g0;

  eskf.TimeUpdate(*imu1);
  EXPECT_TRUE(eskf.states_.x_.vel().isApprox(v0));
  EXPECT_TRUE(eskf.states_.x_.pos().isApprox(v0 * 0.02));
  ELOGD << "eskf.states_.x_.qua() is = "
        << eskf.states_.x_.qua().toRotationMatrix().eulerAngles(2, 0, 1) * convert<radian, degree>(1.0);
  EXPECT_TRUE(eskf.states_.x_.qua().isApprox(Quaterniond(EulerAnglesZXYd((45_deg).convert<radian>()(), 0, 0))));
}