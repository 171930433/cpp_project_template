#include "rref.h"
#include <eigen3/Eigen/Dense>
#include <gtest/gtest.h>

// 4.3 最小二乘估计
//

using namespace Eigen;

class Chapter4_3 : public testing::Test {

public:
  Chapter4_3() {}
  void SetUp() override {}

  void TearDown() override {}

protected:
};

TEST_F(Chapter4_3, worked_example_A) {
  Vector<double, 10> A = Vector<double, 10>::Ones();
  Vector<double, 10> b = Vector<double, 10>::Zero();
  b(9) = 40;

  double C = (A.transpose() * A).inverse() * A.transpose() * b;

  // 1. 最小二乘解
  EXPECT_EQ(C, b.mean());

  // 2. 最小最大误差解, 以为观测值只有0 和 40, 所以最大误差只能是均值20, 即C = 40-20 = 20

  // 3. 最小误差的绝对值, 即对 |e| = |b-Ax| 进行求和, 9*fabs(0-C) + fabs(40-C)=y
  // if C<0, y=40-10c, c>=0, y=40+8c ,所以y_min=40,c=0
}

class Chapter4_3_11 : public testing::Test {

public:
  Chapter4_3_11() {
    b_ = (Vector4d() << 0, 8, 8, 20).finished();
    A_.col(0).setOnes();
    A_.col(1) = (Vector4d() << 0, 1, 3, 4).finished();
  }
  void SetUp() override {}

  void TearDown() override {}

protected:
  Matrix<double, 4, 2> A_;
  Vector4d b_;
  static constexpr int m_ = 4;
  static constexpr int n_ = 2;
};

TEST_F(Chapter4_3_11, problem_set_1) {

  // normal equation is
  Vector<double, n_> x_hat = (A_.transpose() * A_).lu().solve(A_.transpose() * b_);

  Vector<double, m_> E = A_ * x_hat - b_;

  EXPECT_EQ(E.squaredNorm(), 44);
  EXPECT_EQ(x_hat, (Vector<double, n_>{ 1, 4 }));
}

TEST_F(Chapter4_3_11, problem_set_2) {
  Vector<double, n_> x_hat = (A_.transpose() * A_).lu().solve(A_.transpose() * b_);
  Vector<double, m_> p = A_ * x_hat;

  EXPECT_EQ(p, (Vector4d{ 1, 5, 13, 17 }));
  EXPECT_TRUE((A_ * x_hat - p).isZero());

  Vector<double, n_> x_hat2 = (A_.transpose() * A_).lu().solve(A_.transpose() * p);
  EXPECT_EQ(x_hat, x_hat2);
}

TEST_F(Chapter4_3_11, problem_set_3) {
  Vector<double, n_> x_hat = (A_.transpose() * A_).lu().solve(A_.transpose() * b_);
  Vector<double, m_> p = A_ * x_hat;
  Vector<double, m_> e = b_ - p;

  EXPECT_TRUE((e.transpose() * A_.col(0)).isZero());
  EXPECT_TRUE((e.transpose() * A_.col(1)).isZero());

  EXPECT_DOUBLE_EQ(e.norm(), sqrt(44));
}

TEST_F(Chapter4_3_11, problem_set_4) {
  Matrix<double, n_, n_> AtA = A_.transpose() * A_;
  Vector<double, n_> Atb = A_.transpose() * b_;

  EXPECT_TRUE(AtA.isApprox((Matrix<double, n_, n_>() << 4, 8, 8, 26).finished()));
  EXPECT_TRUE(Atb.isApprox((Vector<double, n_>() << 36, 112).finished()));
}

// 使用水平线对观测点进行拟合,即 y=C
TEST_F(Chapter4_3_11, problem_set_5) {
  Matrix<double, 4, 1> A = Matrix<double, 4, 1>::Ones();
  Vector<double, 4> b = b_;

  double x_hat = (A.transpose() * A).inverse() * A.transpose() * b;
  Vector<double, 4> e = b - A * x_hat;

  EXPECT_DOUBLE_EQ(x_hat, b_.mean());
  EXPECT_DOUBLE_EQ(x_hat, 9);
  EXPECT_TRUE(e.isApprox(Vector4d{ -9, -1, -1, 11 }));
}

// 将b投影到A.col(0)
TEST_F(Chapter4_3_11, problem_set_6) {
  Vector<double, m_> a0 = A_.col(0);

  double x_hat = (a0.transpose() * a0).inverse() * a0.transpose() * b_;
  Vector<double, m_> p = a0 * x_hat;
  Vector<double, m_> e = b_ - p;

  EXPECT_TRUE(p.isApprox(Vector4d{ 9, 9, 9, 9 }));
  EXPECT_TRUE((e.transpose() * a0).isZero());
  EXPECT_DOUBLE_EQ(e.norm(), sqrt(204));
}

TEST_F(Chapter4_3_11, problem_set_7) {

  Vector<double, m_> a1 = A_.col(1);

  double x_hat = (a1.transpose() * a1).inverse() * a1.transpose() * b_;
  Vector<double, m_> p = a1 * x_hat;
  Vector<double, m_> e = b_ - p;

  EXPECT_DOUBLE_EQ(x_hat, 56.0 / 13);
}

TEST_F(Chapter4_3_11, problem_set_8) {
  // 在5,6中,单独解C,得到C=9
  // 在7中,单独解D,得到D=56.0/13
  // 在1中,解得 [C D] = [1 4]
  // 他们并不相等,因为A的两列 a0,a1并不正交,所以无法单独解出CD
}

TEST_F(Chapter4_3_11, problem_set_9) {
  Matrix<double, m_, n_ + 1> A;
  A.leftCols(2) = A_;
  A.col(2) = A_.col(1).cwiseAbs2();

  Vector<double, n_ + 1> x_hat = A.fullPivLu().solve(b_);
  Vector<double, n_ + 1> Atb = A.transpose() * b_;

  EXPECT_TRUE(Atb.isApprox(Vector3d{ 36, 112, 400 }));
}

TEST_F(Chapter4_3_11, problem_set_10) {
  Matrix<double, m_, n_ + 2> A;
  A.leftCols(2) = A_;
  A.col(2) = A_.col(1).cwiseAbs2();
  A.col(3) = A_.col(1).array().pow(3);

  Vector<double, n_ + 2> x_hat = (A.transpose() * A).fullPivLu().solve(A.transpose() * b_);
  Vector<double, n_ + 2> p = A * x_hat;

  EXPECT_TRUE(x_hat.isApprox(Vector4d{ 0, 47, -28, 5 } / 3.0));
  EXPECT_TRUE((b_ - p).isZero());
  EXPECT_TRUE(b_.isApprox(p));
}

TEST_F(Chapter4_3_11, problem_set_11) {
  double t_mean = A_.col(1).mean();
  double b_mean = b_.mean();

  EXPECT_DOUBLE_EQ(t_mean, 2);
  EXPECT_DOUBLE_EQ(b_mean, 9);

  Vector<double, n_> x_hat = (A_.transpose() * A_).fullPivLu().solve(A_.transpose() * b_);

  ELOGD << " x_hat is \n" << x_hat;

  // 确认过均值
  Vector<double, n_> p_mean{ t_mean, b_mean };
  EXPECT_TRUE(x_hat.isApprox(Vector2d{ 1, 4 }));
  EXPECT_DOUBLE_EQ(x_hat(0) + t_mean * x_hat(1), b_mean);
}