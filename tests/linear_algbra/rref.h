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
    // ELOGD << fmt::format("pivot is A({},{})={}", row_of_pivot, col_of_pivot, pivot);

    if (np != row_of_pivot) {
      PermutationMatrix<_m> P;
      P.setIdentity();
      P = P.applyTranspositionOnTheLeft(np, row_of_pivot);
      A = P * A;
      E = P * E;
      // ELOGD << "P is \n" << P.toDenseMatrix() << " A is \n" << A;
    }
    // 2. 主元缩放成1
    ElemetrayType Eij = ElemetrayType::Identity();
    Eij(np, np) /= pivot;
    A = Eij * A;
    E = Eij * E;
    // ELOGD << "step2 Eij is \n" << Eij << " A is \n" << A;
    // 3. 主元向下消成0
    for (int i2 = 0; i2 < _m; ++i2) {
      if (i2 != np) {
        Eij.setIdentity();
        Eij(i2, np) = -A(i2, col_of_pivot);
        A = Eij * A;
        E = Eij * E;
        // ELOGD << "Eij is \n" << Eij << " A is \n" << A;
      }
    }
    // ELOGD << "--------------------n_pivot = " << np << "done  \n";
  }

  ELOGD << " rank is " << np << " Final Matrix (RREF):\n" << A;
  return std::make_tuple(A, E, np);
}

// rref with column permutation [I F; 0]
// I, r*r
// F, r*(n-r)
// 0, (m-r)*n
// A = E_inv * rref * P_inv
// E A P = rref;
template <typename _Scalar, int _m, int _n>
class RREF {
public:
  // RREF() {}

  RREF(Eigen::Matrix<_Scalar, _m, _n> const& A) {
    std::tie(rref_, E_, rank_) = GaussJordanEiliminate(A);

    P_.setIdentity();
    for (int i = 0; i < rank_; ++i) {
      for (int j = i; j < _n; ++j) {
        if (fabs(rref_(i, j)) >= 1e-10) {
          std::swap(P_.indices()[i], P_.indices()[j]);
          break;
        }
      }
    }
  }

  Eigen::Matrix<_Scalar, _m, _n> WithColumnPermutation() { return rref_ * P_; }
  Eigen::Matrix<_Scalar, -1, -1> NullSpace() {
    using namespace Eigen;

    Eigen::Matrix<_Scalar, -1, -1> nullspace;
    nullspace = MatrixXd::Zero(_n, _n - rank_);

    if (_n - rank_ > 0) {
      nullspace.topRows(rank_) = -(rref_ * P_).topRightCorner(rank_, _n - rank_);
      nullspace.bottomRows(_n - rank_).setIdentity();
      P_.applyThisOnTheLeft(nullspace);
    }

    return nullspace;
  }

public:
  int rank_;
  Eigen::Matrix<_Scalar, _m, _m> E_;
  Eigen::Matrix<_Scalar, _m, _n> rref_; // 没有进行列置换
  Eigen::PermutationMatrix<_n> P_;
};

// 根据LU分解，获得rref矩阵
// A = Pinv L U Qinv
// A = Pinv L E R Qinv
// E A Q = R
// A = Einv R Qinv
inline std::tuple<Eigen::MatrixXd, Eigen::MatrixXd, Eigen::PermutationMatrix<-1>, int> RREF2(Eigen::MatrixXd const& A) {
  using namespace Eigen;
  int rows = A.rows();
  int cols = A.cols();

  auto fullLU = A.fullPivLu();
  int const rank = fullLU.rank();

  MatrixXd U = fullLU.matrixLU().triangularView<Eigen::Upper>();
  MatrixXd L = MatrixXd::Identity(rows, rows);

  // ELOGD << " fullLU.matrixLU() is \n" << fullLU.matrixLU();

  L.block(0, 0, rows, rank).triangularView<StrictlyLower>() = fullLU.matrixLU().block(0, 0, rows, rank);

  // ELOGD << " L is \n" << L;
  // ELOGD << " U is \n" << U;

  MatrixXd R = U;

  // 再次对U进行主元的向上消元
  MatrixXd E = MatrixXd::Identity(rows, rows);
  for (int np = 0; np < rank; ++np) {
    double pivot = R(np, np);
    // 2. 主元缩放成1
    MatrixXd Eii = MatrixXd::Identity(rows, rows);
    Eii(np, np) /= pivot;

    Eii.applyThisOnTheLeft(R);
    Eii.applyThisOnTheLeft(E);
    // ELOGD << "step2 Eii is \n" << Eii << " R is \n" << R;
    // 3. 主元向上消成0
    for (int i2 = 0; i2 < rows; ++i2) {
      if (i2 == np) { break; }

      Eii.setIdentity();
      Eii(i2, np) = -R(i2, np);
      Eii.applyThisOnTheLeft(R);
      Eii.applyThisOnTheLeft(E);
      // ELOGD << "Eii is \n" << Eii << " R is \n" << R;
    }
  }
  ELOGD << "Final E is \n" << E << " R is \n" << R;

  // EU=R --> U=Einv*R

  return { fullLU.permutationP().inverse() * L * E.inverse(), R, fullLU.permutationQ().inverse(), rank };
  // return { E * L.inverse() * fullLU.permutationP(), R, fullLU.permutationQ().inverse(), rank };
}