#include <eigen3/Eigen/Dense>
#include <gtest/gtest.h>
//   LU分解

using namespace Eigen;

class Lesson5_6_7 : public testing::Test {

public:
  Lesson5_6_7() { a_ << RowVector4d{ 1, 2, 2, 2 }, RowVector4d{ 2, 4, 6, 8 }, RowVector4d{ 3, 6, 8, 10 }; }
  void SetUp() override {}

  void TearDown() override {}

protected:
  Matrix<double, 3, 4> a_;
};

// 行阶梯形，主元以下均为0
TEST_F(Lesson5_6_7, row_echelon_form) {
  Matrix<double, 3, 4> A = a_;
  A.row(1) = a_.row(1) - 2 * a_.row(0);
  A.row(2) = a_.row(2) - 3 * a_.row(0);

  Matrix<double, 3, 4> A00;
  A00 << RowVector4d{ 1, 2, 2, 2 }, RowVector4d{ 0, 0, 2, 4 }, RowVector4d{ 0, 0, 2, 4 };

  EXPECT_TRUE(A.isApprox(A00));

  Matrix<double, 3, 4> A12 = A00;
  A12.row(2) = A00.row(2) - A00.row(1);
  A = A12;

  Matrix<double, 3, 4> U;
  U << RowVector4d{ 1, 2, 2, 2 }, RowVector4d{ 0, 0, 2, 4 }, RowVector4d{ 0, 0, 0, 0 };
  EXPECT_TRUE(A.isApprox(U));
}

// 简化行阶梯形，在ref的基础上，将主元缩放成1，主元上下均消元成1
TEST_F(Lesson5_6_7, reduced_row_echelon_form) {
  Matrix<double, 3, 4> U;
  U << RowVector4d{ 1, 2, 2, 2 }, RowVector4d{ 0, 0, 2, 4 }, RowVector4d{ 0, 0, 0, 0 };

  Matrix<double, 3, 4> R = U;
  R.row(1) *= 0.5;
  R.row(0) = R.row(0) - 2 * R.row(1);

  Matrix<double, 3, 4> R_expect;
  R_expect << RowVector4d{ 1, 2, 0, -2 }, RowVector4d{ 0, 0, 1, 2 }, RowVector4d{ 0, 0, 0, 0 };
  EXPECT_TRUE(R.isApprox(R_expect));

  // 交换1, 2列
  PermutationMatrix<4> P13{ Vector4i{ 0, 2, 1, 3 } };
  R = R * P13;

  Matrix<double, 3, 4> RREF_expect;
  RREF_expect << RowVector4d{ 1, 0, 2, -2 }, RowVector4d{ 0, 1, 0, 2 }, RowVector4d{ 0, 0, 0, 0 };
  EXPECT_TRUE(R.isApprox(RREF_expect));

  int rank = a_.fullPivLu().rank();
  EXPECT_EQ(rank, 2); // R_expect 的最后一行全是0

  // 根据rref求解0空间
  Matrix<double, 3, 4> M = Matrix<double, 3, 4>::Zero();
  Matrix<double, 2, 2> F = R.block(0, rank, rank, 4 - rank);
  M.block(0, 0, rank, rank).setIdentity();
  M.block(0, rank, rank, 4 - rank) = F;

  MatrixXd NullA(4, 4 - rank);
  NullA.topRows(2) = -F;
  NullA.bottomRows(2).setIdentity();

  EXPECT_EQ((M * NullA), (Matrix<double, 3, 2>::Zero()));
  EXPECT_EQ((a_ * P13 * NullA.col(0)), Vector3d::Zero());
  EXPECT_EQ((a_ * P13 * NullA.col(1)), Vector3d::Zero());

  // 验证0空间
  GTEST_LOG_(INFO) << "nullspace is \n" << NullA;
  GTEST_LOG_(INFO) << "kernal is \n" << a_.fullPivLu().kernel();
}

TEST_F(Lesson5_6_7, rref_eigen) {
  FullPivLU<Matrix<double, 3, 4>> flu = a_.fullPivLu();

  Matrix<double, 3, 3> P_inv = flu.permutationP().inverse();
  Matrix<double, 4, 4> Q_inv = flu.permutationQ().inverse();

  GTEST_LOG_(INFO) << "P is " << flu.permutationP().indices().transpose();
  GTEST_LOG_(INFO) << "Q is " << flu.permutationQ().indices().transpose();

  GTEST_LOG_(INFO) << "LU is \n" << flu.matrixLU();
  GTEST_LOG_(INFO) << "A rank is " << flu.rank();

  Matrix<double, 3, 3> L = flu.matrixLU().block<3, 3>(0, 0).triangularView<UnitLower>();
  GTEST_LOG_(INFO) << "L is \n" << L;
  GTEST_LOG_(INFO) << "P_inv * L is \n" << P_inv * L;

  Matrix<double, 3, 4> U = flu.matrixLU().triangularView<Upper>();
  GTEST_LOG_(INFO) << "U is \n" << U;
  GTEST_LOG_(INFO) << "U * Q_inv is \n" << U * Q_inv;

  EXPECT_TRUE(a_.isApprox(P_inv * L * U * Q_inv));
}

TEST_F(Lesson5_6_7, rref_eigen2) {
  using namespace std;
  typedef Matrix<double, 5, 3> Matrix5x3;
  typedef Matrix<double, 5, 5> Matrix5x5;
  Matrix5x3 m = Matrix5x3::Random();
  cout << "Here is the matrix m:" << endl << m << endl;

  Eigen::FullPivLU<Matrix5x3> lu(m);
  cout << "Here is, up to permutations, its LU decomposition matrix:" << endl << lu.matrixLU() << endl;

  cout << "Here is the L part:" << endl;
  Matrix5x5 l = Matrix5x5::Identity();
  l.block<5, 3>(0, 0).triangularView<StrictlyLower>() = lu.matrixLU();
  cout << l << endl;

  cout << "Here is the U part:" << endl;
  Matrix5x3 u = lu.matrixLU().triangularView<Upper>();
  cout << u << endl;

  cout << "Let us now reconstruct the original matrix m:" << endl;
  cout << lu.permutationP().inverse() * l * u * lu.permutationQ().inverse() << endl;

  EXPECT_TRUE(m.isApprox(lu.permutationP().inverse() * l * u * lu.permutationQ().inverse()));
}