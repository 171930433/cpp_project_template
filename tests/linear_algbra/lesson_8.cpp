#include "rref.h"
#include <eigen3/Eigen/Dense>
#include <gtest/gtest.h>

// Ax=b
// EAP=IF
// Ax=b --> EAx=Eb --> EAP * Pinv*x=Eb,令y=Pinv*x,则
// EAP * y = Eb, 即 IF*y=Eb, y = [Eb; 0]
// x = P*y
using namespace Eigen;

class Lesson8 : public testing::Test {

public:
  Lesson8() { a_ << RowVector4d{ 1, 2, 2, 2 }, RowVector4d{ 2, 4, 6, 8 }, RowVector4d{ 3, 6, 8, 10 }; }
  void SetUp() override {}

  void TearDown() override {}

  double rand_01() const { return (std::rand() % 100) / 100.0; }

protected:
  Matrix<double, 3, 4> a_;
};

TEST_F(Lesson8, full_row_rank) {
  constexpr int m = 2;
  constexpr int n = 4;
  Matrix<double, m, n> A;
  A << a_.row(0), a_.row(2);

  // rref is [I F],
  auto rref = RREF(A);
  auto const& E = rref.E_;
  auto const rank = rref.rank_;
  auto const& IF = rref.WithColumnPermutation();
  EXPECT_FALSE(IF.bottomRows(1).isZero());

  ELOGD << " IF is \n" << IF;

  // Ax=b ,行满秩，所有C(A) 与R^2一致,包含所有的b向量,又因为0空间的维度为n-r,所有一定有无穷多个解
  // r = m < n

  Vector<double, m> b = Vector<double, m>::Random();
  Vector<double, n> y = (Vector<double, n>() << E * b, VectorXd::Zero(n - rank)).finished();
  MatrixXd nullspace = rref.NullSpace();

  // ELOGD << "nullspace IS \n" << nullspace;
  // ELOGD << "P_inv *  nullspace IS \n" << rref.P_.inverse() * nullspace;

  Vector<double, n> x_p = rref.P_ * y;
  Vector<double, n> x_s = rand_01() * nullspace.col(0) + rand_01() * nullspace.col(1);
  Vector<double, n> x = x_p + x_s;

  // ELOGD << " A * nullspace is \n" << A * nullspace;
  // ELOGD << " IF * x_p is \n" << IF * x_p;

  EXPECT_EQ((IF * y), (E * b));
  EXPECT_TRUE((A * x_p).isApprox(b));
  EXPECT_TRUE((A * x_s).isZero());
  EXPECT_TRUE((A * x).isApprox(b));

  // EXPECT_TRUE((A * x_s).isZero());
  // EXPECT_EQ((A * rref.P_ * x), b);
}

TEST_F(Lesson8, full_col_rank) {
  constexpr int m = 3;
  constexpr int n = 2;
  Matrix<double, m, n> A;
  A << a_.col(0), a_.col(2);

  //  r = n < m
  //  rref 的形式如[I;0],0的维度为m-r, 无自由变量,
  // 如果b经过初等变换E后的结果Eb的tail(m-r) == 0, 则有唯一解; 即b在C(A)中
  // 如果b经过初等变换E后的结果Eb的tail(m-r) != 0, 则无解; 即b不在C(A)中
  auto rref = RREF(A);
  auto const& E = rref.E_;
  auto const rank = rref.rank_;
  auto const& IF = rref.WithColumnPermutation();

  EXPECT_EQ(rank, 2);

  // 1 有解情况, b在C(A)中
  Vector<double, 3> b = A.col(0) + A.col(1);
  EXPECT_TRUE(((E * b).tail(m - rank).isZero()));

  VectorXd y = (E * b).head(rank);
  EXPECT_TRUE((IF * y).isApprox(E * b));

  VectorXd x = y;
  ELOGD << "x is " << x.transpose();
  EXPECT_TRUE((A * x).isApprox(b));

  // 2 无解情况
  Vector<double, 3> b2 = A.col(0).cross(A.col(1));
  EXPECT_FALSE(((E * b2).tail(m - rank).isZero()));
}