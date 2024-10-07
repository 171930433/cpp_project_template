#include "mylib.h"

#include <fmt/ranges.h>
#include <fstream>
// #include <glog/logging.h>
#include <gtest/gtest.h>

TEST(sensors, imu) {
  Imu i1;

  std::string str;
  struct_pb::to_proto<Imu>(str, "pb");

  std::ofstream out("sensors.proto");

  // struct_pb::to_proto_file<FramesPack>(out, "pb");

  EXPECT_TRUE(1);
}

TEST(sensors, channel_message) {

  auto imu = CreateMessage<Imu>("/imu");
  auto gnss = CreateMessage<Gnss>("/gnss");

  EXPECT_TRUE(imu);
  EXPECT_TRUE(gnss);
}

TEST(sensors, cache_message) {

  auto imu = CreateMessage<Imu>("/imu");
  auto gnss = CreateMessage<Gnss>("/gnss");

  std::vector<MessageBase::SPtr> buffer;
  buffer.push_back(imu);
  buffer.push_back(gnss);

  EXPECT_TRUE(buffer.size() == 2);
}

class MessageBufferTest : public testing::Test {
protected:
  MessageBufferTest() {
    imu = CreateMessage<Imu>("/imu");
    imu->t1_ = 0.01;

    imu2 = CreateMessage<Imu>("/imu");
    imu2->msg_.t0_ = 0.01;
    imu2->t1_ = 0.02;

    gnss = CreateMessage<Gnss>("/gnss");
    gnss->t1_ = 0.1;

    gnss2 = CreateMessage<Gnss>("/gnss");
    gnss2->msg_.t0_ = 0.2;
    gnss2->t1_ = 0.3;
  }

  void SetUp() override {
    buffer.Append(imu2);
    buffer.Append(gnss2);
    buffer.Append(gnss);
    buffer.Append(imu);
  }

  void TearDown() override {}
  TotalBuffer buffer;
  Message<Imu>::SPtr imu;
  Message<Imu>::SPtr imu2;
  Message<Gnss>::SPtr gnss;
  Message<Gnss>::SPtr gnss2;
};

TEST_F(MessageBufferTest, base) {
  EXPECT_TRUE(buffer.size() == 4);
  EXPECT_TRUE(buffer.channel_types_.size() == 2);
  EXPECT_TRUE(buffer.channel_names_.size() == 2);
}

TEST_F(MessageBufferTest, order_pushback) {

  // push back order
  std::vector<double> true_order{ imu2->t0(), gnss2->t0(), gnss->t0(), imu->t0() };
  std::vector<double> test_order;
  for (auto const& elem : buffer.get<id>()) { test_order.push_back(elem->t0()); }
  GTEST_LOG_(INFO) << "order push_bask: " << fmt::format("{:.3f}", fmt::join(test_order, ", "));
  EXPECT_EQ(true_order, test_order);
}

TEST_F(MessageBufferTest, order_t1) {

  // t1
  std::vector<double> test_order;
  std::vector<double> true_order_t1{ imu->t1_, imu2->t1_, gnss->t1_, gnss2->t1_ };
  for (auto const& elem : buffer.get<t1>()) { test_order.push_back(elem->t1_); }
  GTEST_LOG_(INFO) << "order t1: " << fmt::format("{:.3f}", fmt::join(test_order, ", "));
  EXPECT_EQ(true_order_t1, test_order);
}

TEST_F(MessageBufferTest, order_t0) {
  // t0
  std::vector<double> test_order;
  std::vector<double> true_order_t0{ imu->t0(), gnss->t0(), imu2->t0(), gnss2->t0() };
  for (auto const& elem : buffer.get<t0>()) { test_order.push_back(elem->t0()); }
  GTEST_LOG_(INFO) << "order t0: " << fmt::format("{:.3f}", fmt::join(test_order, ", "));
  EXPECT_EQ(true_order_t0, test_order);
}

TEST_F(MessageBufferTest, trajectory) {
  auto state = CreateMessage<State>("/state");
  Eigen::Vector3d origin = state->rpose_.translation();

  EXPECT_EQ(origin.x(), 0);
  EXPECT_EQ(origin.y(), 0);
  EXPECT_EQ(origin.z(), 0);
}

template <typename _T>
struct IsTraject : public std::false_type {};

template <>
struct IsTraject<Gnss> : public std::true_type {};

template <typename _T, bool = IsTraject<_T>::value>
struct InnerMessage;

template <typename _T>
struct InnerMessage<_T, false> {
  int a_ = 0;
};

template <typename _T>
struct InnerMessage<_T, true> : public InnerMessage<_T, false> {
  int b_ = 0;
};

TEST(sensors, trajectory_demo) {

  InnerMessage<Imu> i1;

  InnerMessage<Gnss> g2;

  EXPECT_EQ(i1.a_, 0);
  EXPECT_EQ(g2.b_, 0);
  EXPECT_EQ(g2.a_, 0);
}
