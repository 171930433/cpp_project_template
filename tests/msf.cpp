#include "common/eigen_units.h"
#include "data_mock.h"
#include "mylib.h"
#include <gtest/gtest.h>
#include <units.h>

class MsfTest : public testing::Test {
protected:
  MsfTest() {
    imu = ChannelMsg<Imu>::Create("/imu");
    imu->t1_ = 0.01;
    imu2 = ChannelMsg<Imu>::Create("/imu");
    imu2->msg_.t0_ = 0.01;
    imu2->t1_ = 0.02;
    gnss = ChannelMsg<Gnss>::Create("/gnss");
    gnss->t1_ = 0.1;
    gnss2 = ChannelMsg<Gnss>::Create("/gnss");
    gnss2->msg_.t0_ = 0.2;
    gnss2->t1_ = 0.3;
  }

  void SetUp() override {}

  void TearDown() override {}
  MultuiSensorFusion msf;
  // demo data
  ChannelMsg<Imu>::SPtr imu, imu2;
  ChannelMsg<Gnss>::SPtr gnss, gnss2;
};

TEST_F(MsfTest, units) {

  using namespace units::angle;
  using namespace units::literals;
  using namespace Eigen;

  Vector3<degree_t> v1{ 10.0_deg, 5_deg, 90_deg };
  Vector3<degree_t> v2{ 20.0_deg, 10_deg, 90_deg };

  GTEST_LOG_(INFO) << v1;
  GTEST_LOG_(INFO) << v1[0] * 2;
  GTEST_LOG_(INFO) << (v1 + v2).transpose();
  GTEST_LOG_(INFO) << units::math::cos(v1[2]);

  Vector3<double> v3 = v1.cast<radian_t>().cast<double>();
  GTEST_LOG_(INFO) << v3.transpose();

  EXPECT_TRUE(1);
}
