#include "rref.h"
#include <eigen3/Eigen/Dense>
#include <gtest/gtest.h>

// 相关性，基和4个基本的子空间
// 1. R的r个主元行是R和A的行空间的基
// 2. A的r个主元列，是A的列空间的基
// 3. n-r个special solution是A和R的零空间的基
// 4. I的BottomRows(m-r)是R的左零空间的基
// 5. E的BottomRows(m-r)是A的左零空间的基
using namespace Eigen;

class Lesson9_10 : public testing::Test {

public:
  Lesson9_10() { a_ << RowVector4d{ 1, 2, 2, 2 }, RowVector4d{ 2, 4, 6, 8 }, RowVector4d{ 3, 6, 8, 10 }; }
  void SetUp() override {}

  void TearDown() override {}

  double rand_01() const { return (std::rand() % 100) / 100.0; }

protected:
  Matrix<double, 3, 4> a_;
};

TEST_F(Lesson9_10, rref2_nullspace) {
  MatrixXd A = a_;

  constexpr int m = 3;
  constexpr int n = 4;
  auto const [Einv, R, Qinv, rank] = RREF2(A);
  EXPECT_EQ(rank, 2);

  // N(A)'s dim is n-r=2, N(A) in R^n
  MatrixXd N_R = MatrixXd::Zero(n, n - rank);
  N_R.topRows(rank) = -R.topRightCorner(rank, n - rank);
  N_R.bottomRows(n - rank).setIdentity();
  MatrixXd N_A = Qinv.inverse() * N_R;

  EXPECT_TRUE((A * N_A).isZero());
}

TEST_F(Lesson9_10, rref2_left_nullspace) {
  MatrixXd A = a_.transpose();

  int const m = A.rows();
  int const n = A.cols();
  auto const [Einv, R, Qinv, rank] = RREF2(A);
  EXPECT_EQ(rank, 2);

  // N(A)'s dim is n-r=2, N(A) in R^n
  MatrixXd N_R = MatrixXd::Zero(n, n - rank);
  N_R.topRows(rank) = -R.topRightCorner(rank, n - rank);
  N_R.bottomRows(n - rank).setIdentity();
  MatrixXd N_A = Qinv.inverse() * N_R;

  ELOGD << "N_A is \n" << N_A;

  EXPECT_TRUE((A * N_A).isZero());
}
// 事实上,N(a_.Transpose)，在做guass-jordan消元的时候已经获得，R底部的0行都是通过消元获得，即
//  EAQ=R , R的下方有m-r行
//  EA = R*Qinv
TEST_F(Lesson9_10, rref2_left_nullspace2) {

  MatrixXd A = a_;

  int const m = A.rows(); // 3
  int const n = A.cols(); // 4
  auto const [Einv, R, Qinv, rank] = RREF2(A);
  EXPECT_EQ(rank, 2);

  ELOGD << "Einv is \n" << Einv;
  ELOGD << "Einv * R * Qinv is \n" << Einv * R * Qinv;

  // N(Rt)'s dim is m-r=1, N(Rt) in R^m
  MatrixXd N_At = Einv.inverse().bottomRows(m - rank);

  ELOGD << "N_At is \n" << N_At;
  ELOGD << "N_At * A is \n" << N_At * A;

  EXPECT_TRUE((N_At * A).isZero());
}

TEST_F(Lesson9_10, problem_set_3_6_1) {
  // if m=7,n=9,r=5
  constexpr int m = 7;
  constexpr int n = 9;
  constexpr int r = 5;
  //
  MatrixXd A = MatrixXd::Random(m, n);
  A.bottomRows(m - r) = A.topRows(m - r);
  A.rightCols(n - r) = A.leftCols(n - r);

  // auto R = RREF(A);

  EXPECT_EQ(A.fullPivLu().rank(), r);
  EXPECT_EQ(A.fullPivLu().kernel().cols(), n - r);
  EXPECT_EQ(A.transpose().fullPivLu().kernel().cols(), m - r);
}
TEST_F(Lesson9_10, problem_set_3_6_2) {

  Matrix<double, 2, 3> A = (Matrix<double, 2, 3>() << 1, 2, 4, 2, 4, 8).finished();
  Matrix<double, 2, 3> B = (Matrix<double, 2, 3>() << 1, 2, 4, 2, 5, 8).finished();
}

TEST_F(Lesson9_10, kf_demo) {
  Matrix2d D0 = (Matrix2d() << 2, 1, 1, 100).finished();
  Vector2d X0{ 0, 0 };
  // 两侧方程x=1
  Vector<double, 1> m = (Vector<double, 1>() << 1).finished();
  Matrix<double, 1, 1> Rm = (Matrix<double, 1, 1>() << 2).finished();
  Matrix<double, 1, 2> H = (Matrix<double, 1, 2>() << 1, 0).finished();
  // 计算增益
  MatrixXd K = D0 * H.transpose() * (H * D0 * H.transpose() + Rm).inverse();
  Vector2d X1 = X0 +  K * (m - H * X0);
  MatrixXd D2 = (MatrixXd::Identity(2,2) - K * H) * D0;
  // 

  ELOGD << "K = \n" << K;
  ELOGD << "X1 = \n" << X1;
  ELOGD << "D2 = \n" << D2;

}