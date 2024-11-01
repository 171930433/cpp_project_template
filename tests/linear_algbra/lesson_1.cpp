#include <eigen3/Eigen/Dense>
#include <gtest/gtest.h>
// 线性方程组的几何解释  The Geometry of Linear Equations

using namespace Eigen;

class Lesson1 : public testing::Test {

public:
  Lesson1() {
    a_ << 2, 3, 1, -1;
    b_ << 7, 1;
    x_ << 2, 1;
  }
  void SetUp() override {}

  void TearDown() override {}

protected:
  Matrix2d a_;
  Vector2d b_;
  Vector2d x_;
};

TEST_F(Lesson1, row_picture) {
  Vector2d b0;
  b0.row(0) = a_.row(0) * x_;
  b0.row(1) = a_.row(1) * x_;

  EXPECT_EQ(b0, b_);
}

TEST_F(Lesson1, col_picture) {
  Vector2d b0;
  b0 = x_[0] * a_.col(0) + x_[1] * a_.col(1);

  EXPECT_EQ(b0, b_);
}