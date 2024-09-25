#include "sensor_frames.h"
#include <gtest/gtest.h>
#include <fmt/core.h>
#include <fmt/ostream.h>
#include <fstream>

TEST(sensor_frame, imu)
{
    ImuFrame i1;

    std::string str;
    struct_pb::to_proto<ImuFrame>(str, "pb");

    fmt::println("{}", str);

    std::ofstream out("sensor_frame.proto");

    struct_pb::to_proto_file<FramesPack>(out, "pb");

    EXPECT_TRUE(1);
}