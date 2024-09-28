#include "mylib.h"
#include <gtest/gtest.h>
#include <units.h>

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

namespace Eigen {

template <>
struct NumTraits<units::angle::degree_t>
  : NumTraits<double> // permits to get the epsilon, dummy_precision, lowest,
                      // highest functions
{};

namespace internal {
template <>
struct scalar_cos_op<units::angle::degree_t> {
  EIGEN_EMPTY_STRUCT_CTOR(scalar_cos_op)
  EIGEN_DEVICE_FUNC inline double operator()(
    const units::angle::degree_t& a) const {
    return units::math::cos(a);
  }
  template <typename Packet>
  EIGEN_DEVICE_FUNC inline Packet packetOp(const Packet& a) const {
    return internal::pcos(a);
  }
};
}

}

TEST_F(MsfTest, base) {

  using namespace units::angle;
  using namespace units::literals;
  using namespace Eigen;
  // using namespace units::math;

  Array3<degree_t> v1{ 10.0_deg, 5_deg, 90_deg };
  Array3<degree_t> v2{ 20.0_deg, 10_deg, 90_deg };

  GTEST_LOG_(INFO) << v1.transpose();
  GTEST_LOG_(INFO) << (v1 + v2).transpose();
  GTEST_LOG_(INFO) << units::math::cos(v1[2]);
  GTEST_LOG_(INFO) << v1.transpose().cos();

  EXPECT_TRUE(1);
}