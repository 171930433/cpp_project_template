#include <eigen3/Eigen/Dense>
#include <gtest/gtest.h>
//   LU分解

using namespace Eigen;

class Lesson4 : public testing::Test {

public:
  Lesson4() { a_ << 3, 7, 1, 2; }
  void SetUp() override {}

  void TearDown() override {}

protected:
  Matrix2d a_;
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
