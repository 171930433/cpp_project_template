#include "common/eigen_units.h"
#include "mock/app_mock.h"
#include "mylib.h"
#include <gtest/gtest.h>
#include <units.h>

class MsfTest : public testing::Test {
protected:
  MsfTest() {
    imu = Message<Imu>::Create("/imu");
    imu->t1_ = 0.01;
    imu2 = Message<Imu>::Create("/imu");
    imu2->msg_.t0_ = 0.01;
    imu2->t1_ = 0.02;
    gnss = Message<Gnss>::Create("/gnss");
    gnss->t1_ = 0.1;
    gnss2 = Message<Gnss>::Create("/gnss");
    gnss2->msg_.t0_ = 0.2;
    gnss2->t1_ = 0.3;
  }

  void SetUp() override {}

  void TearDown() override {}
  MultuiSensorFusion msf;
  // demo data
  Message<Imu>::SPtr imu, imu2;
  Message<Gnss>::SPtr gnss, gnss2;
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

TEST_F(MsfTest, config_manager) {
  auto* cm = msf.cm();
  auto& io_config = cm->io_;

  io_config.imu_ = { "/imu", 100, "imu name" };
  io_config.gnss_ = { "/gnss", 10, "gnss name" };
  io_config.state_ = { "/state", 100, "state name" };

  std::string json_str;
  iguana::to_json(*cm, json_str);

  std::string path = FLAGS_config_dir + "/tmp_cm.json";
  std::ofstream out(path);
  out << json_str;

  ConfigManager cm2;
  iguana::from_json(cm2, json_str);

  GTEST_LOG_(INFO) << "write to " << path << json_str;

  EXPECT_TRUE(1);

  EXPECT_EQ(cm->io_.imu_.channel_name_, cm2.io_.imu_.channel_name_);
  EXPECT_EQ(cm->io_.imu_.rate_hz_, cm2.io_.imu_.rate_hz_);
  EXPECT_EQ(cm->io_.imu_.device_name_, cm2.io_.imu_.device_name_);
}

TEST_F(MsfTest, module_base) {

  auto mock_app = msf.CreateModule<MockDemoModule>();
  EXPECT_CALL(*mock_app, ProcessImu).Times(1);
  EXPECT_CALL(*mock_app, ProcessGnss).Times(1);

  msf.ProcessData(imu);
  msf.ProcessData(gnss);

  State re_state;
  // !需要显示给定类型,否则lambda的闭包类型不支持模板参数推导
  Message<State>::CFunc func = [&re_state](Message<State>::SCPtr frame) { re_state = frame->msg_; };

  msf.dispatcher()->RegisterWriter("/state", func);

  mock_app->Write();

  EXPECT_FLOAT_EQ(re_state.t0_, 1.0);
  EXPECT_EQ(msf.modules()->size(), 1);
  EXPECT_EQ(msf.dispatcher()->reader_.size(), 2);
}

TEST_F(MsfTest, init_from_config) {
  msf.Init(FLAGS_config_dir);

  auto mock_app = msf.CreateModule<MockDemoModule>();
  EXPECT_CALL(*mock_app, ProcessImu).Times(1);
  EXPECT_CALL(*mock_app, ProcessGnss).Times(1);

  msf.ProcessData(imu);
  msf.ProcessData(gnss);

  State re_state;
  // !需要显示给定类型,否则lambda的闭包类型不支持模板参数推导
  Message<State>::CFunc func = [&re_state](Message<State>::SCPtr frame) { re_state = frame->msg_; };

  msf.dispatcher()->RegisterWriter(msf.cm()->io_.state_, func);

  mock_app->Write();

  EXPECT_FLOAT_EQ(re_state.t0_, 1.0);
  EXPECT_EQ(msf.modules()->size(), 1);
  EXPECT_EQ(msf.dispatcher()->reader_.size(), 2);
}
