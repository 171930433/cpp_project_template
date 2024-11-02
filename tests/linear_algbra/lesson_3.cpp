#include <eigen3/Eigen/Dense>
#include <gtest/gtest.h>
// 矩阵消元  Elimination of matrix

using namespace Eigen;

class Lesson3 : public testing::Test {

public:
  Lesson3() {
    A_ = Matrix3d::Random();
    B_ = Matrix3d ::Random();
    C_ = A_ * B_;
  }
  void SetUp() override {}

  void TearDown() override {}

protected:
  Matrix3d A_;
  Matrix3d B_;
  Matrix3d C_;
};

TEST_F(Lesson3, product_basic1) {
  Matrix3d C0;
  C0.setZero();
  for (int i = 0; i < 3; ++i) {
    for (int j = 0; j < 3; ++j) {
      for (int k = 0; k < 3; ++k) { C0(i, j) += A_(i, k) * B_(k, j); }
    }
  }

  EXPECT_TRUE(C0.isApprox(C_));
}

TEST_F(Lesson3, product_basic2) {
  Matrix3d C0;
  for (int i = 0; i < 3; ++i) {
    for (int j = 0; j < 3; ++j) { C0(i, j) = A_.row(i) * B_.col(j); }
  }

  EXPECT_TRUE(C0.isApprox(C_));
}

TEST_F(Lesson3, product_row_picture) {
  Matrix3d C0;

  // C0的每一行
  for (int i = 0; i < 3; ++i) {
    C0.row(i) = A_(i, 0) * B_.row(0) + A_(i, 1) * B_.row(1) + A_(i, 2) * B_.row(2); // B_行的线性组合
  }
  EXPECT_TRUE(C0.isApprox(C_));
}

TEST_F(Lesson3, product_col_picture) {
  Matrix3d C0;

  for (int i = 0; i < 3; ++i) { C0.col(i) = B_(0, i) * A_.col(0) + B_(1, i) * A_.col(1) + B_(2, i) * A_.col(2); }

  EXPECT_TRUE(C0.isApprox(C_));
}

TEST_F(Lesson3, product_block_picture) {
  Matrix3d C0;
  C0.setZero();
  for (int i = 0; i < 3; i++) { C0 += A_.col(i) * B_.row(i); }

  EXPECT_TRUE(C0.isApprox(C_));
}

TEST_F(Lesson3, product_block) {
  // blocks of A
  auto A00 = A_.topLeftCorner<2, 2>();
  auto A01 = A_.topRightCorner<2, 1>();
  auto A10 = A_.bottomLeftCorner<1, 2>();
  auto A11 = A_.bottomRightCorner<1, 1>();
  // blocks of B
  auto B00 = B_.topLeftCorner<2, 2>();
  auto B01 = B_.topRightCorner<2, 1>();
  auto B10 = B_.bottomLeftCorner<1, 2>();
  auto B11 = B_.bottomRightCorner<1, 1>();

  Matrix3d C0;
  C0.topLeftCorner<2, 2>() = A00 * B00 + A01 * B10;
  C0.topRightCorner<2, 1>() = A00 * B01 + A01 * B11;
  C0.bottomLeftCorner<1, 2>() = A10 * B00 + A11 * B10;
  C0.bottomRightCorner<1, 1>() = A10 * B01 + A11 * B11;

  EXPECT_TRUE(C0.isApprox(C_));
}