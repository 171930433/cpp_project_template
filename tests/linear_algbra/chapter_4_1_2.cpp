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
  EXPECT_TRUE((P_line * e0).isZero());  // e0在P_line的零空间中

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