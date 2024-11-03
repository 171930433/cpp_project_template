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

// 如果一个矩阵A可逆,则
// 1. A的行列式不等于0
// 2. A的所有列向量线性无关
// 3. AX = 0, 仅有X=0 时成立
class Lesson3Inverse : public testing::Test {
protected:
  Lesson3Inverse() {
    a_ << 1, 2, 3, 7;
    a_inverse_ = a_.inverse();
  }
  void SetUp() override {}
  void TearDown() override {}
  Matrix2d a_;
  Matrix2d a_inverse_;
};

TEST_F(Lesson3Inverse, gauss_jordan_2d) {
  Matrix<double, 2, 4> A_argumented;
  A_argumented << a_, Matrix2d::Identity();

  Matrix2d Elementry1;
  Elementry1 << RowVector2d(1, 0), RowVector2d(-3, 1);

  Matrix2d A1;
  A1 << 1, 2, 0, 1;

  EXPECT_TRUE((A1.isApprox((Elementry1 * A_argumented).leftCols(2))));

  // guass到主元下方元素都是0就结束了
  // jordan 是在guass完成的基础上,将U矩阵分解成 U = D * U2,  即对角阵和另外一个上三角阵
  Matrix2d Elementry2;
  Elementry2 << RowVector2d(1, -2), RowVector2d(0, 1);

  EXPECT_TRUE((Matrix2d::Identity().isApprox((Elementry2 * Elementry1 * A_argumented).leftCols(2))));
  EXPECT_TRUE((a_inverse_.isApprox((Elementry2 * Elementry1 * A_argumented).rightCols(2))));
}