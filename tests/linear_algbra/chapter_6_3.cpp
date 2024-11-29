#include <eigen3/Eigen/Dense>
#include <eigen3/unsupported/Eigen/MatrixFunctions>
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

TEST_F(Chapter6_3, eigenvalues_eigenvectors) {
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

TEST_F(Chapter6_3, eigenvalues_eigenvectors2) {
  Matrix2d A = (Matrix2d() << 4, 3, 3, 1).finished();

  SelfAdjointEigenSolver<Matrix2d> solver(A);

  EXPECT_TRUE(solver.info() == Eigen::ComputationInfo::Success);

  if (solver.info() != Eigen::ComputationInfo::Success) return;

  Vector2d lambdas = solver.eigenvalues();
  ELOGD << " lambdas = " << lambdas.transpose();

  double const d = 3 * sqrt(5);

  EXPECT_TRUE(lambdas.isApprox(Vector2d{ 0.5 * (5 - d), 0.5 * (5 + d) }));

  Matrix2d S = solver.eigenvectors();

  ELOGD << " S = \n" << S;
}

TEST_F(Chapter6_3, problem_set_1) {

  Matrix2d A = (Matrix2d() << 4, 3, 0, 1).finished();

  Eigen::EigenSolver<Matrix2d> solver(A);

  Vector2d u0{ 5, -2 };

  Matrix2d S = solver.eigenvectors().real();
  Vector2d lambdas = solver.eigenvalues().real();

  Vector2d c = S.colPivHouseholderQr().solve(u0);

  EXPECT_TRUE((S * c).isApprox(u0));

  auto ut = [S, lambdas, c](
              int t) { return c(0) * exp(lambdas(0) * t) * S.col(0) + c(1) * exp(lambdas(1) * t) * S.col(1); };

  EXPECT_TRUE(ut(0).isApprox(u0));
  ELOGD << " ut(0) = " << ut(0).transpose() << " u0 = " << u0.transpose();

  auto ut2 = [A, u0](int t) -> Vector2d { return (A * t).exp() * u0; };
  auto ut3 = [S, lambdas, c](int t) -> Vector2d { return S * (lambdas.asDiagonal().toDenseMatrix() * t).exp() * c; };

  ELOGD << " ut(1) = " << ut(1).transpose() << " ut2(1) = " << ut2(1).transpose();

  ELOGD << " ut(2) = " << ut(2).transpose() << " ut2(2) = " << ut2(2).transpose();
  ELOGD << " ut(3) = " << ut(3).transpose() << " ut2(3) = " << ut2(3).transpose();

  EXPECT_TRUE((ut(1).isApprox(ut2(1))));
  EXPECT_TRUE((ut(2).isApprox(ut2(2))));
  EXPECT_TRUE((ut(3).isApprox(ut2(3))));
  EXPECT_TRUE((ut(3).isApprox(ut3(3))));
}