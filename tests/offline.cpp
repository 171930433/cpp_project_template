#include "common/eigen_units.h"
#include "data_mock.h"
#include "mylib.h"
#include <gtest/gtest.h>
#include <units.h>

class OfflineTest : public testing::Test {
protected:
  OfflineTest() {
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

TEST_F(OfflineTest, base) {
  using ::testing::AtLeast;
  using ::testing::Return;

  MockDataReader reader;
  EXPECT_CALL(reader, ReadFrame)
    .Times(4)
    .WillOnce(Return(std::pair<MessageBase::SPtr, IDataReader::State>{ imu, IDataReader::State::OK }))
    .WillOnce(Return(std::pair<MessageBase::SPtr, IDataReader::State>{ imu2, IDataReader::State::OK }))
    .WillOnce(Return(std::pair<MessageBase::SPtr, IDataReader::State>{ gnss, IDataReader::State::OK }))
    .WillOnce(Return(std::pair<MessageBase::SPtr, IDataReader::State>{ gnss2, IDataReader::State::END }));

  for (auto it = reader.ReadFrame(); it.second != IDataReader::State::END; it = reader.ReadFrame()) {
    msf.ProcessData(it.first);

    GTEST_LOG_(INFO) << (int)it.second << " " << it.first->to_json_str();
  }

  EXPECT_TRUE(1);
}