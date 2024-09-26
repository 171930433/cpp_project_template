#include "channel_message.h"
#include "channel_message_buffer.h"
#include "sensor_frames.h"

#include <fstream>
#include <glog/logging.h>
#include <gtest/gtest.h>

TEST(sensor_frame, imu) {
  ImuFrame i1;

  std::string str;
  struct_pb::to_proto<ImuFrame>(str, "pb");

  // fmt::println("{}", str);

  std::ofstream out("sensor_frame.proto");

  struct_pb::to_proto_file<FramesPack>(out, "pb");

  EXPECT_TRUE(1);
}

TEST(sensor_frame, channel_message) {

  auto channel_imu = ChannelMsg<ImuFrame>::Create("/imu");
  auto channel_gnss = ChannelMsg<GnssFrame>::Create("/gnss");

  LOG(INFO) << channel_imu->to_json_str();
  LOG(INFO) << channel_gnss->to_json_str();

  EXPECT_TRUE(1);
}

TEST(sensor_frame, cache_message) {

  auto channel_imu = ChannelMsg<ImuFrame>::Create("/imu");
  auto channel_imu2 = ChannelMsg<ImuFrame>::Create("/imu");
  channel_imu2->msg_.t0_ = 0.01;
  auto channel_gnss = ChannelMsg<GnssFrame>::Create("/gnss");
  auto channel_gnss2 = ChannelMsg<GnssFrame>::Create("/gnss");
  channel_gnss2->msg_.t0_ = 0.2;

  std::vector<MessageBase::SPtr> buffer;
  buffer.push_back(channel_imu);
  buffer.push_back(channel_imu2);
  buffer.push_back(channel_gnss);
  buffer.push_back(channel_gnss2);

  for (auto& elem : buffer) {
    LOG(INFO) << elem->channel_name_ << fmt::format(" t0 = {:.3f}", elem->t0());
  }

  EXPECT_TRUE(1);
}

TEST(sensor_frame, sort_message) {
  auto channel_imu = ChannelMsg<ImuFrame>::Create("/imu");
  channel_imu->t1_ = 0.01;
  auto channel_imu2 = ChannelMsg<ImuFrame>::Create("/imu");
  channel_imu2->msg_.t0_ = 0.01;
  channel_imu2->t1_ = 0.02;
  auto channel_gnss = ChannelMsg<GnssFrame>::Create("/gnss");
  channel_gnss->t1_ = 0.1;
  auto channel_gnss2 = ChannelMsg<GnssFrame>::Create("/gnss");
  channel_gnss2->msg_.t0_ = 0.2;
  channel_gnss2->t1_ = 0.3;

  TotalBuffer buffer;
  buffer.Append(channel_imu2);
  buffer.Append(channel_gnss2);
  buffer.Append(channel_gnss);
  buffer.Append(channel_imu);

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