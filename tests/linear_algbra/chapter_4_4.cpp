#include "rref.h"
#include <eigen3/Eigen/Dense>
#include <gtest/gtest.h>

// 4.4 gram-schmidt 正交化
//

using namespace Eigen;

std::pair<MatrixXd, MatrixXd> QRDecomposition(MatrixXd const& A) {
  int const row = A.rows();
  int const col = A.cols();

  MatrixXd Q = A;
  MatrixXd R = A;

  Q.setZero();
  R.setIdentity();

  for (int i = 0; i < col; ++i) {
    VectorXd Vec = A.col(i);

    // ELOGD << "start vec is = " << Vec.transpose();

    // 遍历已经求解的Qi
    for (int j = 0; j < i; ++j) {
      R(j, i) = Q.col(j).transpose() * A.col(i); //
      Vec -= R(j, i) * Q.col(j);
      // ELOGD << " R(j, i) is = " << R(j, i) << " Vec is " << Vec.transpose();
    }

    // ELOGD << "end vec is = " << Vec.transpose();

    R(i, i) = Vec.norm();
    Q.col(i) = Vec.normalized();
  }

  return { Q, R };
}

class Chapter4_4 : public testing::Test {

public:
  Chapter4_4() {}
  void SetUp() override {}

  void TearDown() override {}

protected:
};

TEST_F(Chapter4_4, test_QR) {
  Matrix3d A = (Matrix3d() << 1, 2, 3, -1, 0, -3, 0, -2, 3).finished();

  auto const& [Q, R] = QRDecomposition(A);

  ELOGD << " A = \n" << A;
  ELOGD << " Q = \n" << Q;
  ELOGD << " R = \n" << R;
  ELOGD << " QR = \n" << Q * R;

  EXPECT_TRUE(A.isApprox(Q * R));
}

// householderQ结构比较复杂,可以在细读一下
TEST_F(Chapter4_4, test_QR2) {
  Matrix3d A = (Matrix3d() << 1, 2, 3, -1, 0, -3, 0, -2, 3).finished();
  auto qr = A.householderQr();

  ELOGD << " qr.householderQ = \n" << qr.householderQ() * Matrix3d::Identity();
}