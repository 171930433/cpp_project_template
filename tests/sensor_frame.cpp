#include "sensor_frames.h"
#include "channel_message.h"
#include <gtest/gtest.h>
#include <glog/logging.h>
#include <fstream>

TEST(sensor_frame, imu)
{
    ImuFrame i1;

    std::string str;
    struct_pb::to_proto<ImuFrame>(str, "pb");

    // fmt::println("{}", str);

    std::ofstream out("sensor_frame.proto");

    struct_pb::to_proto_file<FramesPack>(out, "pb");

    EXPECT_TRUE(1);
}

TEST(sensor_frame, channel_message)
{

    auto channel_imu = ChannelMsg<ImuFrame>::Create("/imu");
    auto channel_gnss = ChannelMsg<GnssFrame>::Create("/gnss");

    LOG(INFO) << *channel_imu;
    LOG(INFO) << *channel_gnss;

    EXPECT_TRUE(1);
}

TEST(sensor_frame, cache_message)
{

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

    for (auto &elem : buffer)
    {
        LOG(INFO) << elem->channel_name_ << fmt::format(" t0 = {:.3f}", elem->t0());
    }

    EXPECT_TRUE(1);
}