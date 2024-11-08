#include <eigen3/Eigen/Dense>
#include <gtest/gtest.h>
#include <ylt/easylog.hpp>

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
  int max_rank = std::min(_m, _n);
  int np = 0;

  for (; np < max_rank; ++np) {
    // 遍历行列寻找第n个主元
    _Scalar pivot = 0;
    int row_of_pivot = 0;
    int col_of_pivot = 0;
    for (int j = np; j < _n; ++j) {
      if (A.col(j).tail(_m - np).isZero()) { continue; }
      pivot = A.col(j).tail(_m - np).maxCoeff(&row_of_pivot);
      col_of_pivot = j;
      break;
    }
    row_of_pivot += np;
    if (fabs(pivot) <= 1e-10) {
      ELOGW << "np = " << np << " with all zero";
      break;
    }
    // 1. 最大元素交换到主元行
    ELOGD << fmt::format("pivot is A({},{})={}", row_of_pivot, col_of_pivot, pivot);

    if (np != row_of_pivot) {
      PermutationMatrix<_m> P;
      P.setIdentity();
      P = P.applyTranspositionOnTheLeft(np, row_of_pivot);
      A = P * A;
      E = P * E;
      ELOGD << "P is \n" << P.toDenseMatrix() << " A is \n" << A;
    }
    // 2. 主元缩放成1
    ElemetrayType Eij = ElemetrayType::Identity();
    Eij(np, np) /= pivot;
    A = Eij * A;
    E = Eij * E;
    ELOGD << "step2 Eij is \n" << Eij << " A is \n" << A;
    // 3. 主元向下消成0
    for (int i2 = 0; i2 < _m; ++i2) {
      if (i2 != np) {
        Eij.setIdentity();
        Eij(i2, np) = -A(i2, col_of_pivot);
        A = Eij * A;
        E = Eij * E;
        ELOGD << "Eij is \n" << Eij << " A is \n" << A;
      }
    }
    ELOGD << "--------------------n_pivot = " << np << "done  \n";
  }

  ELOGD << " rank is " << np << " Final Matrix (RREF):\n" << A;
  return std::make_tuple(A, E, np);
}

//  IF = [I F ; 0]
//  E*A*P=IF
// A = E_inv * IF * P_inv;
template <typename _Scalar, int _m, int _n>
std::tuple<Eigen::Matrix<_Scalar, _m, _m>, Eigen::Matrix<_Scalar, _m, _n>, Eigen::PermutationMatrix<_n>, int>
IdentityFree(Eigen::Matrix<_Scalar, _m, _n> const& input_matrix) {
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

  return { E, rref * P, P, rank };
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
  Eigen::Matrix<_Scalar, _m, _n> rref_;
};


template <typename _Scalar, int _m, int _n>
Eigen::Matrix<_Scalar, -1, -1> NullSpace(Eigen::Matrix<_Scalar, _m, _n> const& input_matrix) {
  using namespace Eigen;

  auto const& [E, IF, P, rank] = IdentityFree(input_matrix);

  Eigen::Matrix<_Scalar, -1, -1> nullspace;
  nullspace = MatrixXd::Zero(_n, _n - rank);

  if (_n - rank > 0) {
    nullspace.topRows(rank) = -IF.topRightCorner(rank, _n - rank);
    nullspace.bottomRows(_n - rank).setIdentity();
    P.applyThisOnTheLeft(nullspace);
  }

  return nullspace;
}