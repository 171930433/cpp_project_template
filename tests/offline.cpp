#include "common/eigen_units.h"
#include "mock/data_mock.h"
#include "mylib.h"
#include <gtest/gtest.h>
#include <units.h>

#include "mock/app_mock.h"
#include "modules/psins/psins_app.h"

class OfflineTest : public testing::Test {
protected:
  OfflineTest() {
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

TEST_F(OfflineTest, base) {
  using ::testing::AtLeast;
  using ::testing::Return;

  MockDataReader reader;
  EXPECT_CALL(reader, ReadFrame)
    .Times(5)
    .WillOnce(Return(std::pair<MessageBase::SPtr, IDataReader::IOState>{ imu, IDataReader::IOState::OK }))
    .WillOnce(Return(std::pair<MessageBase::SPtr, IDataReader::IOState>{ imu2, IDataReader::IOState::OK }))
    .WillOnce(Return(std::pair<MessageBase::SPtr, IDataReader::IOState>{ gnss, IDataReader::IOState::OK }))
    .WillOnce(Return(std::pair<MessageBase::SPtr, IDataReader::IOState>{ gnss2, IDataReader::IOState::OK }))
    .WillOnce(Return(std::pair<MessageBase::SPtr, IDataReader::IOState>{ nullptr, IDataReader::IOState::END }));

  auto mock_app = msf.CreateModule<MockDemoModule>();
  EXPECT_CALL(*mock_app, ProcessImu).Times(2);
  EXPECT_CALL(*mock_app, ProcessGnss).Times(2);

  for (auto it = reader.ReadFrame(); it.second != IDataReader::IOState::END; it = reader.ReadFrame()) {
    msf.ProcessData(it.first);

    GTEST_LOG_(INFO) << (int)it.second << " " << it.first->to_header_str();
  }

  EXPECT_EQ(msf.get_buffer()->size(), 4);
}

TEST_F(OfflineTest, psins) {
  msf.Init(FLAGS_config_dir);

  PsinsReader reader;
  reader.Init(FLAGS_data_dir + "/mimuattgps.bin");

  msf.CreateModule<PsinsApp>();

  // 构造输出
  std::deque<Message<State>::SCPtr> fused_states;
  Message<State>::CFunc cbk = [&fused_states](Message<State>::SCPtr frame) { fused_states.push_back(frame); };

  msf.dispatcher()->RegisterWriter("/fused_state", cbk);

  int count = 100;
  for (int i = 0; i < count; ++i) {
    auto it = reader.ReadFrame();
    msf.ProcessData(it.first);
  }


  EXPECT_TRUE(1);
  EXPECT_EQ(fused_states.size(), count - 1);
}