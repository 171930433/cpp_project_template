#include <eigen3/Eigen/Dense>
#include <gtest/gtest.h>

// 5.1 特征值

using namespace Eigen;

class Chapter5_1 : public testing::Test {

public:
  Chapter5_1() {}
  void SetUp() override {}

  void TearDown() override {}

protected:
};

TEST_F(Chapter5_1, problem_set_1) {
  constexpr int n = 4;
  Matrix4d A = Matrix4d::Identity();
  A(0, 0) = 0.5;

  double det_a = 0.5;

  EXPECT_DOUBLE_EQ(Matrix4d::Identity().determinant(), 1);
  EXPECT_DOUBLE_EQ(A.determinant(), det_a);
  EXPECT_DOUBLE_EQ((2 * A).determinant(), det_a * pow(2, n));
  EXPECT_DOUBLE_EQ((A * A).determinant(), det_a * det_a);
  EXPECT_DOUBLE_EQ((A.inverse()).determinant(), 1 / det_a);
}

TEST_F(Chapter5_1, problem_set_2) {
  constexpr int n = 3;
  Matrix3d A = Matrix3d::Identity();
  A(0, 0) = -1;

  double det_a = -1;

  EXPECT_DOUBLE_EQ(Matrix3d::Identity().determinant(), 1);
  EXPECT_DOUBLE_EQ(A.determinant(), det_a);
  EXPECT_DOUBLE_EQ((0.5 * A).determinant(), det_a * pow(0.5, n));
  EXPECT_DOUBLE_EQ((A * A).determinant(), det_a * det_a);
  EXPECT_DOUBLE_EQ((A.inverse()).determinant(), 1 / det_a);
}

TEST_F(Chapter5_1, problem_set_13) {
  Matrix3d A = (Matrix3d() << 1, 1, 1, 1, 2, 2, 1, 2, 3).finished();
  EXPECT_DOUBLE_EQ(
    A.determinant(), A.lu().permutationP().determinant() * A.lu().matrixLU().triangularView<Upper>().determinant());
  EXPECT_DOUBLE_EQ(A.determinant(), 1);

  Matrix3d B = (Matrix3d() << 1, 2, 3, 2, 2, 3, 3, 3, 3).finished();
  EXPECT_DOUBLE_EQ(
    B.determinant(), B.lu().permutationP().determinant() * B.lu().matrixLU().triangularView<Upper>().determinant());
  EXPECT_DOUBLE_EQ(B.determinant(), 3);
}

TEST_F(Chapter5_1, problem_set_14) {
  Matrix4d A = (Matrix4d() << 1, 2, 3, 0, 2, 6, 6, 1, -1, 0, 0, 3, 0, 2, 0, 7).finished();
  // Matrix4d U = (Matrix4d() << 1, 2, 3, 0, 0, 2, 0, 1, 0, 0, 3, 2, 0, 0, 0, 6).finished();
  // EXPECT_TRUE(A.lu().matrixLU().triangularView<Upper>().toDenseMatrix().isApprox(U));
  EXPECT_DOUBLE_EQ(
    A.determinant(), A.lu().permutationP().determinant() * A.lu().matrixLU().triangularView<Upper>().determinant());
  EXPECT_DOUBLE_EQ(A.determinant(), 1 * 2 * 3 * 6);
}

TEST_F(Chapter5_1, problem_set_15) {
  Matrix3d A = (Matrix3d() << 101, 201, 301, 102, 202, 302, 103, 203, 303).finished();
  EXPECT_DOUBLE_EQ(A.determinant(), 0);
}