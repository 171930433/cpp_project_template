#include "mylib.h"
#include <gtest/gtest.h>

class MsfTest : public testing::Test {
protected:
  MsfTest() {}

  void SetUp() override {
    auto imu = ChannelMsg<Imu>::Create("/imu");
    imu->t1_ = 0.01;
    auto imu2 = ChannelMsg<Imu>::Create("/imu");
    imu2->msg_.t0_ = 0.01;
    imu2->t1_ = 0.02;
    auto gnss = ChannelMsg<Gnss>::Create("/gnss");
    gnss->t1_ = 0.1;
    auto gnss2 = ChannelMsg<Gnss>::Create("/gnss");
    gnss2->msg_.t0_ = 0.2;
    gnss2->t1_ = 0.3;

    msf.Append(imu2);
    msf.Append(gnss2);
    msf.Append(gnss);
    msf.Append(imu);
  }

  void TearDown() override {}
  MultuiSensorFusion msf;
};

TEST_F(MsfTest, base) {
  EXPECT_TRUE(1);
}