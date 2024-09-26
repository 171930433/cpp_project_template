#include "mylib.h"

#include <fstream>
#include <glog/logging.h>
#include <gtest/gtest.h>

TEST(sensors, imu) {
  Imu i1;

  std::string str;
  struct_pb::to_proto<Imu>(str, "pb");

  // fmt::println("{}", str);

  std::ofstream out("sensors.proto");

  struct_pb::to_proto_file<FramesPack>(out, "pb");

  EXPECT_TRUE(1);
}

TEST(sensors, channel_message) {

  auto imu = ChannelMsg<Imu>::Create("/imu");
  auto gnss = ChannelMsg<Gnss>::Create("/gnss");

  // LOG(INFO) << imu->to_json_str();
  // LOG(INFO) << gnss->to_json_str();

  EXPECT_TRUE(imu);
  EXPECT_TRUE(gnss);
}

TEST(sensors, cache_message) {

  auto imu = ChannelMsg<Imu>::Create("/imu");
  auto imu2 = ChannelMsg<Imu>::Create("/imu");
  imu2->msg_.t0_ = 0.01;
  auto gnss = ChannelMsg<Gnss>::Create("/gnss");
  auto gnss2 = ChannelMsg<Gnss>::Create("/gnss");
  gnss2->msg_.t0_ = 0.2;

  std::vector<MessageBase::SPtr> buffer;
  buffer.push_back(imu);
  buffer.push_back(imu2);
  buffer.push_back(gnss);
  buffer.push_back(gnss2);

  for (auto& elem : buffer) {
    LOG(INFO) << elem->channel_name_ << fmt::format(" t0 = {:.3f}", elem->t0());
  }

  EXPECT_TRUE(1);
}

TEST(sensors, sort_message) {
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

  TotalBuffer buffer;
  buffer.Append(imu2);
  buffer.Append(gnss2);
  buffer.Append(gnss);
  buffer.Append(imu);

  // push back order
  for (auto const& elem : buffer.get<id>()) {
    LOG(INFO) << elem->to_json_str();
  }
  // t1
  for (auto const& elem : buffer.get<t0>()) {
    LOG(INFO) << elem->to_json_str();
  }
  // t0
  for (auto const& elem : buffer.get<t1>()) {
    LOG(INFO) << elem->to_json_str();
  }

  EXPECT_TRUE(1);
}