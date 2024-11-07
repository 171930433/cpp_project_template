#include <eigen3/Eigen/Dense>
#include <gtest/gtest.h>
//   LU分解

using namespace Eigen;

// 添加reduced row echelon from,计算一个矩阵的行最简形式
// 高斯-约尔当消元

template <typename _Scalar, int _m, int _n>
std::vector<Matrix<_Scalar, _m, _m>> GaussJordanEiliminate(const Matrix<_Scalar, _m, _n>& A) {
  using ElemetrayType = Matrix<_Scalar, _m, _m>;
  Matrix<_Scalar, _m, _n> matrix = A; // 创建输入矩阵的副本
  int rows = _m;
  int cols = _n;

  // 用于存储每一步的初等矩阵
  std::vector<ElemetrayType> elementary_matrices;

  for (int i = 0; i < rows; ++i) {
    // 创建单位矩阵作为初等矩阵的基础
    ElemetrayType E = ElemetrayType::Identity();

    int maxRow = 0;
    _Scalar const max_pivot = matrix.col(i).tail(_m - i).maxCoeff(&maxRow);
    maxRow += i;
    GTEST_LOG_(INFO) << " maxRow = " << maxRow + i;

    // 如果整个列都是零，跳过此列
    if (fabs(max_pivot) <= 1e-10) { continue; }

    // 交换最大主元行和当前行
    if (maxRow != i) {
      PermutationMatrix<_m> P;
      P.setIdentity();
      std::swap(P.indices()[i], P.indices()[maxRow]);

      elementary_matrices.push_back(P.toDenseMatrix().template cast<_Scalar>());
      matrix = P * matrix;
      std::cout << "Row Swap Matrix (Row " << i << " <-> Row " << maxRow << "):\n" << E << "\n\n";
    }

    // 归一化主元行
    _Scalar pivot = matrix(i, i);
    matrix.row(i) /= pivot;
    E(i, i) = 1 / pivot; // 记录归一化
    elementary_matrices.push_back(E);
    std::cout << "Scaling Matrix for Row " << i << ":\n" << E << "\n\n";
    E = ElemetrayType::Identity();

    // 消元
    for (int j = 0; j < rows; ++j) {
      if (j != i) {
        _Scalar factor = matrix(j, i);
        matrix.row(j) -= factor * matrix.row(i);
        E(j, i) = -factor; // 记录行加减
        elementary_matrices.push_back(E);
        std::cout << "Elimination Matrix for Row " << j << " using Row " << i << ":\n" << E << "\n\n";
        E = ElemetrayType::Identity();
      }
    }
  }

  std::cout << "Final Matrix (RREF):\n" << matrix << "\n\n";
  return elementary_matrices;
}

class Lesson5_6_7 : public testing::Test {

public:
  Lesson5_6_7() {
    a_ << RowVector4d{ 1, 2, 2, 2 }, RowVector4d{ 2, 4, 6, 8 }, RowVector4d{ 3, 6, 8, 10 };
    at_ = a_.transpose();
  }
  void SetUp() override {}

  void TearDown() override {}

protected:
  Matrix<double, 3, 4> a_;
  Matrix<double, 4, 3> at_;
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

// LU分解的最终目标是 将 Ax=b, 分解成 PLUx=b, 令 Ux=y, 则
// 先解 PLy=b,又因为PL是下三角矩阵，则可以使用前向迭代依次解出来y
// y解出后，又因为Ux=y,U是上三角矩阵，则可以再通过后向迭代依次解出来x
TEST_F(Lesson5_6_7, rref_eigen2_At) {
  constexpr int m = 4;
  constexpr int n = 3;

  Matrix<double, m, n> At = at_;

  FullPivLU<Matrix<double, m, n>> flu = At.fullPivLu();

  Matrix<double, m, n> A2 = flu.permutationP() * At * flu.permutationQ();

  GTEST_LOG_(INFO) << "Here is the A2:\n" << A2;

  // A2 将不再有行列变换，更容易看清楚过程
  auto flu2 = A2.fullPivLu();

  EXPECT_EQ((flu2.permutationP().indices()), (Vector<int, m>{ 0, 1, 2, 3 }));
  EXPECT_EQ((flu2.permutationQ().indices()), (Vector<int, n>{ 0, 1, 2 }));

  GTEST_LOG_(INFO) << "Here is the LU:\n" << flu2.matrixLU();

  Matrix<double, m, m> L = Matrix<double, m, m>::Identity();
  L.block<m, n>(0, 0).triangularView<StrictlyLower>() = flu2.matrixLU();
  GTEST_LOG_(INFO) << "Here is the L:\n" << L;

  Matrix<double, m, n> U = flu2.matrixLU().triangularView<Upper>();
  GTEST_LOG_(INFO) << "Here is the U:\n" << U;

  int rank = flu2.rank();
  EXPECT_EQ(rank, 2);

  MatrixXd NullA2(n, n - rank);
  NullA2 = flu2.kernel();

  EXPECT_TRUE((A2 * NullA2).isApprox(MatrixXd::Zero(m, n - rank)));
}

TEST_F(Lesson5_6_7, guass_jordan_ellimination) {
  auto re = GaussJordanEiliminate(a_);

  PermutationMatrix<3> P;
  P.setIdentity();

  GTEST_LOG_(INFO) << "Here is the v " << P.indices().transpose();

  std::swap(P.indices()[0], P.indices()[2]);

  GTEST_LOG_(INFO) << "Here is the v " << P.indices().transpose();

  // GTEST_LOG_(INFO) << "Here is the A2:\n" << A2;
}