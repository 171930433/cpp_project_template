#include <gtest/gtest.h>

#include <boost/timer/timer.hpp>
#include <cmath>

TEST(timer, base) {
  boost::timer::auto_cpu_timer t;

  for (long i = 0; i < 1e7; ++i); // burn some time

  GTEST_LOG_(INFO) << t.format();

  EXPECT_TRUE(1);
}