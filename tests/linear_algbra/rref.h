#include <eigen3/Eigen/Dense>
#include <gtest/gtest.h>

// 添加reduced row echelon from,计算一个矩阵的行最简形式
// 高斯-约尔当消元
template <typename _Scalar, int _m, int _n>
std::tuple<Eigen::Matrix<_Scalar, _m, _n>, Eigen::Matrix<_Scalar, _m, _m>, int> GaussJordanEiliminate(
  Eigen::Matrix<_Scalar, _m, _n> const& input_matrix) {
  using namespace Eigen;
  using ElemetrayType = Matrix<_Scalar, _m, _m>;
  Matrix<_Scalar, _m, _n> A = input_matrix; // 创建输入矩阵的副本

  ElemetrayType E = ElemetrayType::Identity();

  // 确认每一行的主元
  int n_pivot = 0;
  for (int i = 0; i < _m; ++i) {

    if (i >= _n) { break; }

    int row_of_pivot = i;
    _Scalar pivot = A.col(i).tail(_m - n_pivot).maxCoeff(&row_of_pivot);
    row_of_pivot += n_pivot;

    if (fabs(pivot) <= 1e-10) { continue; }
    GTEST_LOG_(INFO) << " n_pivot = " << n_pivot << ", row of pivot is " << row_of_pivot;

    // 1. 最大元素交换到主元行
    if (n_pivot != row_of_pivot) {
      PermutationMatrix<_m> P;
      P.setIdentity();
      P = P.applyTranspositionOnTheLeft(n_pivot, row_of_pivot);
      A = P * A;
      E = P * E;
      GTEST_LOG_(INFO) << "P is \n" << P.toDenseMatrix() << " A is \n" << A;
    }

    // 2. 主元缩放成1
    ElemetrayType Eij = ElemetrayType::Identity();
    Eij(n_pivot, n_pivot) /= pivot;
    A = Eij * A;
    E = Eij * E;
    GTEST_LOG_(INFO) << "step2 Eij is \n" << Eij << " A is \n" << A;

    // 3. 主元向下消成0
    for (int i2 = 0; i2 < _m; ++i2) {
      if (i2 != n_pivot) {
        Eij.setIdentity();
        Eij(i2, n_pivot) = -A(i2, i);
        A = Eij * A;
        E = Eij * E;
        GTEST_LOG_(INFO) << "Eij is \n" << Eij << " A is \n" << A;
      }
    }

    n_pivot++;
    GTEST_LOG_(INFO) << "--------------------n_pivot = " << n_pivot << "done  \n";
  }

  GTEST_LOG_(INFO) << " rank is " << n_pivot << " Final Matrix (RREF):\n" << A;
  return std::make_tuple(A, E, n_pivot);
}

//  [I F ; 0]
template <typename _Scalar, int _m, int _n>
std::pair<Eigen::Matrix<_Scalar, _m, _n>, Eigen::PermutationMatrix<_n>> IdentityFree(
  Eigen::Matrix<_Scalar, _m, _n> const& input_matrix) {
  using namespace Eigen;

  auto const [rref, E, rank] = GaussJordanEiliminate(input_matrix);

  PermutationMatrix<_n> P;
  P.setIdentity();
  for (int i = 0; i < rank; ++i) {
    for (int j = i; j < _n; ++j) {
      if (fabs(rref(i, j)) >= 1e-10) {
        std::swap(P.indices()[i], P.indices()[j]);
        break;
      }
    }
  }

  // GTEST_LOG_(INFO) << " P is " << P.indices().transpose() << "rref * P is \n" << rref * P;

  return { rref * P, P };
}

// rref with column permutation [I F; 0]
// I, r*r
// F, r*(n-r)
// 0, (m-r)*n
// A = E_inv * rref * P_inv
template <typename _Scalar, int _m, int _n>
struct FullRREF {
  int rank_;
  Eigen::Matrix<_Scalar, _m, _m> Einv_;
  Eigen::PermutationMatrix<_n> Pinv_;
  Eigen::Matrix<_Scalar, -1, -1> null_space_;
};

template <typename _Scalar, int _m, int _n>
FullRREF<_Scalar, _m, _n> RREF2(Eigen::Matrix<_Scalar, _m, _n> const& input_matrix) {
  using namespace Eigen;
  FullRREF<_Scalar, _m, _n> re;
  auto const& [rref, E, rank] = GaussJordanEiliminate(input_matrix);

  auto const& [IF, P] = IdentityFree(input_matrix);

  re.rank_ = rank;
  re.Einv_ = E.inverse();
  re.Pinv_ = P.inverse();

  //   nullspace
  if (rank == _n) {
    re.null_space_ = Matrix<_Scalar, 0, 0>::Zero();
  } else {
    re.null_space_ = MatrixXd::Zero(_m, _n - rank);
    re.null_space_.topRows(rank) = -IF.topRightCorner(rank, _n - rank);
    re.null_space_.bottomRows(_m - rank).setIdentity();
  }

  return re;
}