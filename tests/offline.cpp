#include "common/eigen_units.h"
#include "mock/data_mock.h"
#include "mylib.h"
#include <gtest/gtest.h>
#include <units.h>

class OfflineTest : public testing::Test {
protected:
  OfflineTest() {
    imu = Channel<Imu>::Create("/imu");
    imu->t1_ = 0.01;
    imu2 = Channel<Imu>::Create("/imu");
    imu2->msg_.t0_ = 0.01;
    imu2->t1_ = 0.02;
    gnss = Channel<Gnss>::Create("/gnss");
    gnss->t1_ = 0.1;
    gnss2 = Channel<Gnss>::Create("/gnss");
    gnss2->msg_.t0_ = 0.2;
    gnss2->t1_ = 0.3;
  }

  void SetUp() override {}

  void TearDown() override {}
  MultuiSensorFusion msf;
  // demo data
  Channel<Imu>::SPtr imu, imu2;
  Channel<Gnss>::SPtr gnss, gnss2;
};

TEST_F(OfflineTest, base) {
  using ::testing::AtLeast;
  using ::testing::Return;

  MockDataReader reader;
  EXPECT_CALL(reader, ReadFrame)
    .Times(5)
    .WillOnce(Return(std::pair<MessageBase::SPtr, IDataReader::State>{ imu, IDataReader::State::OK }))
    .WillOnce(Return(std::pair<MessageBase::SPtr, IDataReader::State>{ imu2, IDataReader::State::OK }))
    .WillOnce(Return(std::pair<MessageBase::SPtr, IDataReader::State>{ gnss, IDataReader::State::OK }))
    .WillOnce(Return(std::pair<MessageBase::SPtr, IDataReader::State>{ gnss2, IDataReader::State::OK }))
    .WillOnce(Return(std::pair<MessageBase::SPtr, IDataReader::State>{ nullptr, IDataReader::State::END }));

  for (auto it = reader.ReadFrame(); it.second != IDataReader::State::END; it = reader.ReadFrame()) {
    msf.ProcessData(it.first);

    GTEST_LOG_(INFO) << (int)it.second << " " << it.first->to_header_str();
  }

  EXPECT_EQ(msf.get_buffer().size(), 4);
}