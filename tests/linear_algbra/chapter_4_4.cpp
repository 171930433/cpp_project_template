#include "rref.h"
#include <eigen3/Eigen/Dense>
#include <gtest/gtest.h>

// 4.4 gram-schmidt 正交化
// 正交向量 q.transpose() * p = 0
// 正交矩阵 Q是方阵， Q.transpose() * Q = I, 基Qinv=Q.transpose()

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

TEST_F(Chapter4_4, problem_set_11) {
  Vector<double, 5> a = (Vector<double, 5>() << 1, 3, 4, 5, 7).finished();
  Vector<double, 5> b = (Vector<double, 5>() << -6, 6, 8, 0, 8).finished();
  Vector<double, 5> c = (Vector<double, 5>() << 1, 0, 0, 0, 0).finished();
  Matrix<double, 5, 2> A = (Matrix<double, 5, 2>() << a, b).finished();

  Vector<double, 5> q0 = a.normalized();
  Vector<double, 5> q1 = (b - q0.transpose() * b * q0).normalized();

  auto const& [Q, R] = QRDecomposition(A);

  EXPECT_TRUE(Q.col(0).isApprox(q0));
  EXPECT_TRUE(Q.col(1).isApprox(q1));

  // 求解c在ab的列空间投影点
  Vector<double, 5> p = Q * Q.transpose() * c;
  Vector<double, 5> p0 = q0.transpose() * c * q0;
  Vector<double, 5> p1 = q1.transpose() * c * q1;

  EXPECT_TRUE(p.isApprox(p0 + p1));

  EXPECT_TRUE(p.isApprox((Vector<double, 5>() << 0.5, -0.18, -0.24, 0.4, 0).finished()));
}

class Chapter4_4_ProbelemSet_30_34 : public testing::Test {

public:
  Chapter4_4_ProbelemSet_30_34() {
    double s2 = sqrt(2);
    W << 1, 1, s2, 0, 1, 1, -s2, 0, 1, -1, 0, s2, 1, -1, 0, -s2;
    W *= 0.5;
    Q << 1, -1, -1, -1, -1, 1, -1, -1, -1, -1, 1, -1, -1, -1, -1, 1;
  }
  void SetUp() override {}

  void TearDown() override {}

protected:
  Matrix4d W;
  Matrix4d Q;
};

TEST_F(Chapter4_4_ProbelemSet_30_34, 30) {
  // W的列正交，行也正交，行列向量的模值都是1
  Matrix4d Winv = W.inverse();
  EXPECT_TRUE(W.transpose().isApprox(Winv));
}

TEST_F(Chapter4_4_ProbelemSet_30_34, 31) { Vector4d b = { 1, 1, 1, 1 }; }