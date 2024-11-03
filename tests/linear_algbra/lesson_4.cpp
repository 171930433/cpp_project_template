#include <eigen3/Eigen/Dense>
#include <gtest/gtest.h>
//   LU分解

using namespace Eigen;

class Lesson4 : public testing::Test {

public:
  Lesson4() {
    // clang-format off
    a_ << 3, 7, 1, 2;
    a2_ << 7, 8, 10, 
           4, 5,  6,
           1, 2, 3;
    // clang-format on
  }
  void SetUp() override {}

  void TearDown() override {}

protected:
  Matrix2d a_;
  Matrix3d a2_;
};

TEST_F(Lesson4, LU_2) {
  Matrix2d A = a_;

  Matrix2d Elementray10 = Matrix2d::Identity();
  Elementray10.row(1) << -1.0 / 3, 1;

  Matrix2d U = Elementray10 * A;
  Matrix2d L = Elementray10.inverse();

  EXPECT_TRUE((U.isApprox((Matrix2d() << 3, 7, 0, -1.0 / 3).finished())));
  EXPECT_TRUE((L * U).isApprox(A));

  // check
  Matrix2d L2 = A.partialPivLu().matrixLU().triangularView<UnitLower>();
  Matrix2d U2 = A.partialPivLu().matrixLU().triangularView<Upper>();
  Matrix2d P2 = A.lu().permutationP();
  EXPECT_TRUE((P2 * L2 * U2).isApprox(A));

  // GTEST_LOG_(INFO) <<"P2 = \n" << P2;
  // GTEST_LOG_(INFO) <<"L2 = \n" << L2;
  // GTEST_LOG_(INFO) <<"U2 = \n" << U2;
}

TEST_F(Lesson4, LU_3) {
  Matrix3d A = a2_;

  Matrix3d E10 = Matrix3d::Identity();
  E10.row(1) << -4.0 / 7, 1, 0;

  Matrix3d E20 = Matrix3d::Identity();
  E20.row(2) << -1.0 / 7, 0, 1;

  Matrix3d tempA = E20 * E10 * A;

  GTEST_LOG_(INFO) << "tempA = \n" << tempA;

  Matrix3d E21 = Matrix3d::Identity();
  E21.row(2) << 0, -tempA(2, 1) / tempA(1, 1), 1;

  Matrix3d E = E21 * E20 * E10;
  Matrix3d L = E.inverse();
  Matrix3d U = E21 * E20 * E10 * A;

  EXPECT_TRUE((L * U).isApprox(A));

  // check
  Matrix3d L2 = A.lu().matrixLU().triangularView<UnitLower>();
  Matrix3d U2 = A.lu().matrixLU().triangularView<Upper>();
  Matrix3d P2 = A.lu().permutationP();

  EXPECT_TRUE((P2 * L2 * U2).isApprox(A));

  GTEST_LOG_(INFO) << "L = \n" << L;
  GTEST_LOG_(INFO) << "U = \n" << U;

  GTEST_LOG_(INFO) << "P2 = \n" << P2;
  GTEST_LOG_(INFO) << "L2 = \n" << L2;
  GTEST_LOG_(INFO) << "U2 = \n" << U2;
}