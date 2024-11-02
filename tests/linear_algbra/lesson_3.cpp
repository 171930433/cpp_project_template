#include <eigen3/Eigen/Dense>
#include <gtest/gtest.h>
// 矩阵消元  Elimination of matrix

using namespace Eigen;

class Lesson3 : public testing::Test {

public:
  Lesson3() {
    m_ = 3;
    n_ = 5;
    p_ = 4;
  }
  void SetUp() override {
    A_ = MatrixXd::Random(m_, n_);
    B_ = MatrixXd::Random(n_, p_);
    C_ = A_ * B_;
  }

  void TearDown() override {}

protected:
  MatrixXd A_;
  MatrixXd B_;
  MatrixXd C_;
  int m_;
  int n_;
  int p_;
};

TEST_F(Lesson3, product_basic1) {
  MatrixXd C0(m_, p_);
  C0.setZero();
  for (int i = 0; i < m_; ++i) {
    for (int j = 0; j < p_; ++j) {
      for (int k = 0; k < n_; ++k) { C0(i, j) += A_(i, k) * B_(k, j); }
    }
  }

  EXPECT_TRUE(C0.isApprox(C_));
}

TEST_F(Lesson3, product_basic2) {
  MatrixXd C0(m_, p_);
  for (int i = 0; i < m_; ++i) {
    for (int j = 0; j < p_; ++j) { C0(i, j) = A_.row(i) * B_.col(j); }
  }

  EXPECT_TRUE(C0.isApprox(C_));
}

TEST_F(Lesson3, product_row_picture) {
  MatrixXd C0(m_, p_);
  C0.setZero();

  // C0的每一行
  for (int i = 0; i < m_; ++i) {
    for (int j = 0; j < n_; j++) { C0.row(i) += A_(i, j) * B_.row(j); }
  }
  EXPECT_TRUE(C0.isApprox(C_));
}

TEST_F(Lesson3, product_col_picture) {
  MatrixXd C0(m_, p_);
  C0.setZero();

  for (int i = 0; i < p_; ++i) {
    for (int j = 0; j < n_; j++) { C0.col(i) += B_(j, i) * A_.col(j); }
  }

  EXPECT_TRUE(C0.isApprox(C_));
}

TEST_F(Lesson3, product_block_picture) {
  MatrixXd C0(m_, p_);
  C0.setZero();
  for (int i = 0; i < n_; i++) { C0 += A_.col(i) * B_.row(i); }

  EXPECT_TRUE(C0.isApprox(C_));
}

TEST_F(Lesson3, product_block) {
  // blocks of A
  auto A00 = A_.block(0, 0, 2, 2);
  auto A01 = A_.block(0, 2, 2, n_ - 2);
  auto A10 = A_.block(2, 0, m_ - 2, 2);
  auto A11 = A_.block(2, 2, m_ - 2, n_ - 2);
  // blocks of B
  auto B00 = B_.block(0, 0, 2, 2);
  auto B01 = B_.block(0, 2, 2, p_ - 2);
  auto B10 = B_.block(2, 0, n_ - 2, 2);
  auto B11 = B_.block(2, 2, n_ - 2, p_ - 2);

  MatrixXd C0(m_, p_);
  C0.block(0, 0, 2, 2) = A00 * B00 + A01 * B10;
  C0.block(0, 2, 2, p_ - 2) = A00 * B01 + A01 * B11;
  C0.block(2, 0, m_ - 2, 2) = A10 * B00 + A11 * B10;
  C0.block(2, 2, m_ - 2, p_ - 2) = A10 * B01 + A11 * B11;

  EXPECT_TRUE(C0.isApprox(C_));
}