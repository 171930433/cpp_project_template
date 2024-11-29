#include <eigen3/Eigen/Dense>
#include <gtest/gtest.h>
#include <ylt/easylog.hpp>

// 6.3 差分方程的应用

using namespace Eigen;

class Chapter6_3 : public testing::Test {

public:
  Chapter6_3() {}
  void SetUp() override {}

  void TearDown() override {}

protected:
};

TEST_F(Chapter6_3, problem_set_1) {
  Matrix2d A = (Matrix2d() << 4, 3, 0, 1).finished();

  EigenSolver<Matrix2d> solver(A);

  EXPECT_TRUE(solver.info() == Eigen::ComputationInfo::Success);

  if (solver.info() != Eigen::ComputationInfo::Success) return;

  Vector2cd lambdas = solver.eigenvalues();
  ELOGD << " lambdas = " << lambdas.transpose();

  EXPECT_EQ(lambdas, (Vector2cd(Eigen::dcomplex(4), Eigen::dcomplex(1))));

  Matrix2cd S = solver.eigenvectors();

  ELOGD << " S = \n" << S;
}
