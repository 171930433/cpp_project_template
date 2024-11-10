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

// r = m = n
TEST_F(Lesson8, full_rank) {
  constexpr int m = 3;
  constexpr int n = 3;
  Matrix<double, m, n> A;
  A << a_.col(0), a_.col(2), A.col(0).cross(A.col(1));

  // 因为A是方阵, r=m=n, 所以 C(A)==R^3, 则b一定在C(A)中,所以任意b有唯一解
  // rref=[I]
  auto rref = RREF(A);
  auto const& E = rref.E_;
  auto const rank = rref.rank_;
  auto const& IF = rref.WithColumnPermutation();

  EXPECT_EQ(rank, 3);
  Vector<double, 3> b = Vector<double, 3>::Random();
  Vector<double, 3> y = E * b;
  Vector<double, 3> x = y;

  EXPECT_TRUE(IF.isApprox(Matrix3d::Identity()));
  EXPECT_TRUE((A * x).isApprox(b));

  EXPECT_TRUE((x).isApprox(A.lu().solve(b)));
}

// r < m, r < n
TEST_F(Lesson8, rank_deficiency) {
  constexpr int m = 3;
  constexpr int n = 4;
  Matrix<double, m, n> A = a_;

  // rref is [I F; 0], 0 is m-r,n
  // I is r*r, F is r*n-r
  auto rref = RREF(A);
  auto const& E = rref.E_;
  auto const rank = rref.rank_;
  auto const& IF = rref.WithColumnPermutation();
  auto const& P = rref.P_;

  EXPECT_EQ(rank, 2);

  // 1. 如果b在 C(A)中, 则有无穷多个解
  Vector<double, m> b = A.col(0) + A.col(2);
  EXPECT_TRUE((E * b).tail(m - rank).isZero());

  Vector<double, n> y = (Vector<double, n>() << (E * b).head(rank), VectorXd::Zero(n - rank)).finished();
  EXPECT_TRUE((IF * y).isApprox(E * b));

  Vector<double, n> x_p = P * y;
  EXPECT_TRUE((A * x_p).isApprox(b));

  Vector<double, n> x_s = rand_01() * rref.NullSpace().col(0) + rand_01() * rref.NullSpace().col(1);
  EXPECT_TRUE((A * x_s).isZero());

  Vector<double, n> x = x_p + x_s;
  EXPECT_TRUE((A * x).isApprox(b));

  // 2. 如果b不在 C(A)中, 则无解
  Vector<double, 3> b2 = A.col(0).cross(A.col(2));
  EXPECT_FALSE((E * b2).tail(m - rank).isZero());
}

TEST_F(Lesson8, worked_example_3_4A) {
  constexpr int m = 3;
  constexpr int n = 4;
  Matrix<double, 3, 4> A;
  A.row(0) << 1, 2, 3, 5;
  A.row(1) << 2, 4, 8, 12;
  A.row(2) << 3, 6, 7, 13;
  Vector<double, 3> b = Vector<double, 3>::Random();

  // [A b] --> [U c], without column permutation
  Matrix<double, 3, 5> Ab;
  Ab << A, b;

  // 1
  auto rref1 = RREF(Ab);
  ELOGD << " rref1.rref_ is \n" << rref1.rref_;
  EXPECT_TRUE(rref1.rref_.isUpperTriangular());

  // 2 因为A的IF形式为 [I F; 0]
  bool with_solution1 = Ab.bottomRows(A.rows() - rref1.rank_).isZero();
  bool with_solution2 = Ab.fullPivLu().kernel().cols() != 0; // Ab的0空间>0,则b与A的列向量线性相关
  EXPECT_EQ(with_solution1, with_solution2);

  // 3 .C(A) 是 c(0)与c(2)的线性组合; C(A) 是所有b向量线性组合

  // 4 N(A) is n*(n-r)
  auto rref = RREF(A);
  auto const& nullspace = rref.NullSpace();
  EXPECT_EQ(nullspace.cols(), A.cols() - rref.rank_);
  EXPECT_TRUE((A * nullspace).isZero());

  // b = {0,6,-6}, [I F; 0]
  b = Vector3d{ 0, 6, -6 };
  bool with_solution3 = (rref.E_ * b).bottomRows(m - rref.rank_).isZero();
  EXPECT_TRUE(with_solution3);

  Vector<double, n> y =
    (Vector<double, n>() << (rref.E_ * b).head(rref.rank_), VectorXd::Zero(n - rref.rank_)).finished();
  Vector<double, n> x_p = rref.P_ * y;
  EXPECT_TRUE((A * x_p).isApprox(b));

  // [A b] --> [R d], with column permutation
  Matrix<double, 3, 5> Ab2;
  Ab2 << A, b;

  auto const Rd = RREF(Ab2).WithColumnPermutation();
  ELOGD << "Rd is \n" << Rd;
}

TEST_F(Lesson8, worked_example_3_4B) {
  // 如果Ax=b, Ais m*n, rank of A is r

  // 1. 如果只有一个解 IF 是 [I;0], 说明没有自由变量,则列满秩 n=r, 即 m>=n=r

  // 2. x=x_p + c*x_s, 说明 [I F; 0], 说明有一个自由变量, N(A)is n*1, m>= n=r+1
  // 且 r=1, n=2, m>=1

  // 3. 无解, 说明 b不在C(A)中 , IF is [I F;0], 即 r<m

  // 4. x=x_p + c*x_s, 且x_p 是3*1, IF is [I F; 0], n=3, r=2, m>=2

  // 5. 无穷多个解 rref is [I F; 0], r<n
}

TEST_F(Lesson8, worked_example_3_4C) {
  constexpr int m = 3;
  constexpr int n = 4;
  Matrix<double, 3, 4> A;
  A.row(0) << 1, 2, 1, 0;
  A.row(1) << 2, 4, 4, 8;
  A.row(2) << 4, 8, 6, 8;
  Vector<double, m> b{ 4, 2, 10 };

  MatrixXd Ab = (MatrixXd(3,5) << A, b).finished();

  auto const R = RREF(A);

  EXPECT_TRUE(1);
}