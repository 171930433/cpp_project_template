#include <eigen3/Eigen/Dense>
#include <gtest/gtest.h>
// 矩阵消元  Elimination of matrix

using namespace Eigen;

class Lesson3 : public testing::Test {

public:
  Lesson3() {
    m_ = 3;
    n_ = 5;
    p_ = 4;
  }
  void SetUp() override {
    A_ = MatrixXd::Random(m_, n_);
    B_ = MatrixXd::Random(n_, p_);
    C_ = A_ * B_;
  }

  void TearDown() override {}

protected:
  MatrixXd A_;
  MatrixXd B_;
  MatrixXd C_;
  int m_;
  int n_;
  int p_;
};

TEST_F(Lesson3, product_basic1) {
  MatrixXd C0(m_, p_);
  C0.setZero();
  for (int i = 0; i < m_; ++i) {
    for (int j = 0; j < p_; ++j) {
      for (int k = 0; k < n_; ++k) { C0(i, j) += A_(i, k) * B_(k, j); }
    }
  }

  EXPECT_TRUE(C0.isApprox(C_));
}

TEST_F(Lesson3, product_basic2) {
  MatrixXd C0(m_, p_);
  for (int i = 0; i < m_; ++i) {
    for (int j = 0; j < p_; ++j) { C0(i, j) = A_.row(i) * B_.col(j); }
  }

  EXPECT_TRUE(C0.isApprox(C_));
}

TEST_F(Lesson3, product_row_picture) {
  MatrixXd C0(m_, p_);
  C0.setZero();

  // C0的每一行
  for (int i = 0; i < m_; ++i) {
    for (int j = 0; j < n_; j++) { C0.row(i) += A_(i, j) * B_.row(j); }
  }
  EXPECT_TRUE(C0.isApprox(C_));
}

TEST_F(Lesson3, product_col_picture) {
  MatrixXd C0(m_, p_);
  C0.setZero();

  for (int i = 0; i < p_; ++i) {
    for (int j = 0; j < n_; j++) { C0.col(i) += B_(j, i) * A_.col(j); }
  }

  EXPECT_TRUE(C0.isApprox(C_));
}

TEST_F(Lesson3, product_block_picture) {
  MatrixXd C0(m_, p_);
  C0.setZero();
  for (int i = 0; i < n_; i++) { C0 += A_.col(i) * B_.row(i); }

  EXPECT_TRUE(C0.isApprox(C_));
}

TEST_F(Lesson3, product_block) {
  // blocks of A
  auto A00 = A_.block(0, 0, 2, 2);
  auto A01 = A_.block(0, 2, 2, n_ - 2);
  auto A10 = A_.block(2, 0, m_ - 2, 2);
  auto A11 = A_.block(2, 2, m_ - 2, n_ - 2);
  // blocks of B
  auto B00 = B_.block(0, 0, 2, 2);
  auto B01 = B_.block(0, 2, 2, p_ - 2);
  auto B10 = B_.block(2, 0, n_ - 2, 2);
  auto B11 = B_.block(2, 2, n_ - 2, p_ - 2);

  MatrixXd C0(m_, p_);
  C0.block(0, 0, 2, 2) = A00 * B00 + A01 * B10;
  C0.block(0, 2, 2, p_ - 2) = A00 * B01 + A01 * B11;
  C0.block(2, 0, m_ - 2, 2) = A10 * B00 + A11 * B10;
  C0.block(2, 2, m_ - 2, p_ - 2) = A10 * B01 + A11 * B11;

  EXPECT_TRUE(C0.isApprox(C_));
}

// 如果一个矩阵A可逆,则
// 1. A的行列式不等于0
// 2. A的所有列向量线性无关
// 3. AX = 0, 仅有X=0 时成立
class Lesson3Inverse : public testing::Test {
protected:
  Lesson3Inverse() {
    a_ << 1, 2, 3, 7;
    a_inverse_ = a_.inverse();
  }
  void SetUp() override {}
  void TearDown() override {}
  Matrix2d a_;
  Matrix2d a_inverse_;
};

TEST_F(Lesson3Inverse, gauss_jordan_2d) {
  Matrix<double, 2, 4> A_argumented;
  A_argumented << a_, Matrix2d::Identity();

  Matrix2d Elementry1;
  Elementry1 << RowVector2d(1, 0), RowVector2d(-3, 1);

  Matrix2d A1;
  A1 << 1, 2, 0, 1;

  EXPECT_TRUE((A1.isApprox((Elementry1 * A_argumented).leftCols(2))));

  // guass到主元下方元素都是0就结束了
  // jordan 是在guass完成的基础上,将U矩阵分解成 U = D * U2,  即对角阵和另外一个上三角阵
  Matrix2d Elementry2;
  Elementry2 << RowVector2d(1, -2), RowVector2d(0, 1);

  EXPECT_TRUE((Matrix2d::Identity().isApprox((Elementry2 * Elementry1 * A_argumented).leftCols(2))));
  EXPECT_TRUE((a_inverse_.isApprox((Elementry2 * Elementry1 * A_argumented).rightCols(2))));
}

class ProblemSet1_3 : public testing::Test {
protected:
  ProblemSet1_3() {
    S_.col(0) << 1, 1, 1;
    S_.col(1) << 0, 1, 1;
    S_.col(2) << 0, 0, 1;

    W_.col(0) << 1, 2, 3;
    W_.col(1) << 4, 5, 6;
    W_.col(2) << 7, 8, 9;
  }
  void SetUp() override {}
  void TearDown() override {}
  Matrix3d S_;
  Matrix3d W_;
};

TEST_F(ProblemSet1_3, 1) {
  Vector3d x{ 2, 3, 4 };

  Vector3d b{ 2, 5, 9 };

  EXPECT_DOUBLE_EQ(S_.row(0).dot(x), b[0]);
  EXPECT_DOUBLE_EQ(S_.row(1).dot(x), b[1]);
  EXPECT_DOUBLE_EQ(S_.row(2).dot(x), b[2]);
  EXPECT_TRUE((S_ * x).isApprox(b));
}

TEST_F(ProblemSet1_3, 2) {

  Matrix<double, 3, 2> b;
  b.col(0) << 1, 1, 1;
  b.col(1) << 1, 4, 9;

  Matrix<double, 3, 2> y;
  y.col(0) << 1, 0, 0;
  y.col(1) << 1, 3, 5;

  EXPECT_TRUE((S_.lu().solve(b)).isApprox(y));
}

TEST_F(ProblemSet1_3, 3) { EXPECT_TRUE(S_.determinant() != 0); }

TEST_F(ProblemSet1_3, 4) {

  Vector3d x = { 1, -2, 1 };

  GTEST_LOG_(INFO) << x.transpose();

  EXPECT_TRUE(W_.determinant() == 0);
  EXPECT_TRUE((W_ * x).isApprox(Vector3d::Zero()));
}

TEST_F(ProblemSet1_3, 5) {
  Matrix3d R = W_.transpose();
  Vector3d x = { 1, -2, 1 };

  EXPECT_TRUE(R.determinant() == 0);
  EXPECT_TRUE((R * x).isApprox(Vector3d::Zero()));
}

TEST_F(ProblemSet1_3, 6) {
  Matrix3d A;

  double c = 3;
  A.row(0) << 1, 3, 5;
  A.row(1) << 0, -1, -1;
  A.row(2) << 0, -2, c - 5;
  A.row(2) << 0, 0, c - 3;

  EXPECT_TRUE(A.determinant() == 0);

  Matrix3d B;
  double c2 = -1;
  B.row(0) << 1, 0, c2;
  B.row(1) << 0, 1, -c2;
  B.row(2) << 0, 1, 1;
  B.row(2) << 0, 0, 1 + c2;

  EXPECT_TRUE(B.determinant() == 0);

  Matrix3d C;
  double c3 = 0;
  C.col(0) << c3, 2, 3;
  C.col(1) << 0, -1, 0;
  C.col(2) << 0, 3, 3;

  EXPECT_TRUE(C.determinant() == 0);
}

TEST_F(ProblemSet1_3, 7) {
  // 因为ri*x == 0,所以x垂直于r1,r2,r3组成的平面; 自然r1,r2,r3线性相关
  // 而非零向量x就是 r1,r2,r3所张成的平面π的法向量
  EXPECT_TRUE(1);
}

TEST_F(ProblemSet1_3, 8) {
  Matrix4d A;
  A.row(0) << 1, 0, 0, 0;
  A.row(1) << -1, 1, 0, 0;
  A.row(2) << 0, -1, 1, 0;
  A.row(3) << 0, 0, -1, 1;

  Matrix<double, 4, 8> A_argumented;
  A_argumented << A, Matrix4d::Identity();

  Matrix4d E10 = Matrix4d::Identity();
  E10.row(1) << 1, 1, 0, 0;

  Matrix4d E21 = Matrix4d::Identity();
  E21.row(2) << 0, 1, 1, 0;

  Matrix4d E32 = Matrix4d::Identity();
  E32.row(3) << 0, 0, 1, 1;

  Matrix<double, 4, 8> U = E32 * E21 * E10 * A_argumented;

  // GTEST_LOG_(INFO) << "U.leftCols(4) \n " << U.leftCols(4);
  // GTEST_LOG_(INFO) << "U.rightCols(4) \n " << U.rightCols(4);
  // GTEST_LOG_(INFO) << "A.inv() \n " << A.inverse();

  EXPECT_TRUE((U.leftCols(4).isApprox(Matrix4d::Identity())));
  EXPECT_TRUE((U.rightCols(4).isApprox(A.inverse())));
}

TEST_F(ProblemSet1_3, 9) {
  Matrix4d C;
  C.row(0) << 1, 0, 0, -1;
  C.row(1) << -1, 1, 0, 0;
  C.row(2) << 0, -1, 1, 0;
  C.row(3) << 0, 0, -1, 1;

  //  1,1,1,1 为C的列向量张成空间的正交补空间
  Vector4d x = Vector4d::Ones();

  EXPECT_TRUE((C * x).isApprox(Vector4d::Zero()));
}

TEST_F(ProblemSet1_3, 10) {
  Matrix3d A;
  A.row(0) << -1, 1, 0;
  A.row(1) << 0, -1, 1;
  A.row(2) << 0, 0, -1;

  // GTEST_LOG_(INFO) <<" a.inv = \n" << A.inverse();
  Matrix3d A_inv;
  A_inv.row(0) << -1, -1, -1;
  A_inv.row(1) << 0, -1, -1;
  A_inv.row(2) << 0, 0, -1;

  EXPECT_TRUE(A.inverse().isApprox(A_inv));
}