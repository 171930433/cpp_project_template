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