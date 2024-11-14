#include "rref.h"
#include <eigen3/Eigen/Dense>
#include <gtest/gtest.h>

// 4.1 4个基础空间的正交性
//

using namespace Eigen;

class Chapter4_1_2 : public testing::Test {

public:
  Chapter4_1_2() {}
  void SetUp() override {}

  void TearDown() override {}

  double rand_01() const { return (std::rand() % 100) / 100.0; }

protected:
};

TEST_F(Chapter4_1_2, worked_examples_A) {
  Vector3d b{ 3, 3, 4 };
  Vector3d a0{ 2, 2, 1 };
  Vector3d a1{ 1, 0, 0 };

  // 1. 将b投影到a0上
  Matrix3d P_line = a0 * (a0.transpose() * a0).inverse() * a0.transpose();
  Vector3d p0 = P_line * b;
  Vector3d e0 = b - p0;

  EXPECT_TRUE((a0.transpose() * (e0)).isZero());
  EXPECT_TRUE(P_line.isApprox(P_line * P_line));
  EXPECT_TRUE((P_line * e0).isZero()); // e0在P_line的零空间中

  // 2. 将b投影到a0,a1所在的平面
  Matrix<double, 3, 2> A = (Matrix<double, 3, 2>() << a0, a1).finished();
  EXPECT_EQ(A.fullPivLu().rank(), 2);
  EXPECT_EQ(A.fullPivLu().dimensionOfKernel(), 0);

  Matrix3d P_plane = A * (A.transpose() * A).inverse() * A.transpose();
  Vector3d p1 = P_plane * b;
  Vector3d e1 = b - p1;

  EXPECT_TRUE((a0.transpose() * (e1)).isZero());
  EXPECT_TRUE((a1.transpose() * (e1)).isZero());
  EXPECT_TRUE((A.transpose() * (e1)).isZero());
  EXPECT_TRUE(P_plane.isApprox(P_plane * P_plane));
  EXPECT_TRUE((P_plane * e1).isZero()); // e1在p_plane的零空间中
}

TEST_F(Chapter4_1_2, worked_examples_B) {
  Vector3d A{ 1, 1, 1 };
  Vector3d b{ 70, 80, 120 };

  Vector<double, 1> x = (A.transpose() * A).inverse() * A.transpose() * b;
  Vector3d p = A * x;
  Matrix3d P = A * (A.transpose() * A).inverse() * A.transpose();

  EXPECT_DOUBLE_EQ(x(0), b.mean());
}

TEST_F(Chapter4_1_2, problem_set_4_2_1) {
  Vector3d b1{ 1, 2, 2 };
  Vector3d a1{ 1, 1, 1 };

  double x = (a1.transpose() * a1).inverse() * a1.transpose() * b1;
  Vector3d p1 = a1 * x;
  Vector3d e1 = b1 - p1;
  Matrix3d P1 = a1 * (a1.transpose() * a1).inverse() * a1.transpose();

  EXPECT_TRUE((P1 * b1).isApprox(p1));
  EXPECT_TRUE((a1.transpose() * e1).isZero());

  //
  Vector3d b2{ 1, 2, 2 };
  Vector3d a2{ 1, 1, 1 };
  double x2 = (a2.transpose() * a2).inverse() * a2.transpose() * b2;
  Vector3d p2 = a2 * x2;
  Vector3d e2 = b2 - p2;
  Matrix3d P2 = a2 * (a2.transpose() * a2).inverse() * a2.transpose();

  EXPECT_TRUE((P2 * b2).isApprox(p2));
  EXPECT_TRUE((a2.transpose() * e2).isZero());
}

TEST_F(Chapter4_1_2, problem_set_4_2_30) {
  Matrix<double, 2, 3> A = (Matrix<double, 2, 3>() << 3, 6, 6, 4, 8, 8).finished();

  EXPECT_EQ(A.fullPivLu().rank(), 1);
  EXPECT_EQ(A.fullPivLu().dimensionOfKernel(), A.cols() - 1);

  Matrix<double, 2, 1> A1 = A.col(0);
  Matrix2d Pc = A1 * (A1.transpose() * A1).inverse() * A1.transpose();

  Matrix<double, 3, 1> A2 = A.transpose().col(0);

  Matrix3d Pr = A2 * (A2.transpose() * A2).inverse() * A2.transpose();

  ELOGD << "A is \n" << A;
  ELOGD << "Pc is \n" << Pc;
  ELOGD << "Pr is \n" << Pr;

  Matrix<double, 2, 3> B = Pc * A * Pr;
  ELOGD << "B is \n" << B;

  // Pc * A 是将A的列向量往A的主列投影，而A的秩为1，所以所有列都与主列线性相关，所以
  // Pc * A = A
  EXPECT_TRUE((Pc * A).isApprox(A));

  // 同理
  EXPECT_TRUE((A * Pr).isApprox(A));
  EXPECT_TRUE((Pc * A * Pr).isApprox(A));
}