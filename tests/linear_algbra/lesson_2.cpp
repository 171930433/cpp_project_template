#include <eigen3/Eigen/Dense>
#include <gtest/gtest.h>
// 矩阵消元  Elimination of matrix

using namespace Eigen;

class Lesson2 : public testing::Test {

public:
  Lesson2() {
    a_ << 1, 2, 3, 4, 5, 6, 7, 8, 9;
    b_ << 6, 12, 18;
    x_ << 1, -2, 3;
    u_ << 1, 2, 3, 0, -3, -6, 0, 0, 0;
  }
  void SetUp() override {}

  void TearDown() override {}

protected:
  Matrix3d a_;
  Vector3d b_;
  Vector3d x_;
  Matrix3d u_;
};

// 行视图直接决定了b的单一元素
TEST_F(Lesson2, row_picture) {
  Vector3d b0;
  b0.row(0) = a_.row(0) * x_;
  b0.row(1) = a_.row(1) * x_;
  b0.row(2) = a_.row(2) * x_;

  EXPECT_EQ(b0, b_);
}

// 列视图，描述为按照x的各个分量，对矩阵A的各列向量进行线性组合；
// b的所有取值，为以A的列向量为基底，所展开的空间
TEST_F(Lesson2, col_picture) {
  Vector3d b0;
  b0 = x_[0] * a_.col(0) + x_[1] * a_.col(1) + x_[2] * a_.col(2);

  EXPECT_EQ(b0, b_);
}

// Elimination of A
TEST_F(Lesson2, eliminate) {
  Matrix3d A00 = a_;
  // step 1, 让 a00下面的元素消成0
  A00.row(1) = a_.row(1) - 4 * a_.row(0);
  A00.row(2) = a_.row(2) - 7 * a_.row(0);

  // step 2, 让a11下的元素消成0
  Matrix3d A11 = A00;
  A11.row(2) = A00.row(2) - 2 * A00.row(1);

  EXPECT_EQ(u_, A11);
}

// permutaion matrix, 置换行或列的矩阵 P * M 则为对M的行置换, M * P则是对M的列置换
TEST_F(Lesson2, permutaion) {
  Matrix3d p1;
  p1 << RowVector3d{ 1, 0, 0 }, RowVector3d{ -4, 1, 0 }, RowVector3d{ 0, 0, 1 };

  Matrix3d p2;
  p2 << RowVector3d{ 1, 0, 0 }, RowVector3d{ 0, 1, 0 }, RowVector3d{ -7, 0, 1 };

  Matrix3d p3;
  p3 << RowVector3d{ 1, 0, 0 }, RowVector3d{ 0, 1, 0 }, RowVector3d{ 0, -2, 1 };

  Matrix3d A = p3 * p2 * p1 * a_;
  EXPECT_EQ(A, u_);
}

TEST_F(Lesson2, permutaion2) {
  PermutationMatrix p1 = PermutationMatrix<3>(Vector3i{ 2, 1, 0 });
  p1.indices();

  Matrix3i A;
  A << 0, 0, 1, 0, 1, 0, 1, 0, 0;

  EXPECT_EQ(p1.indices(), (Vector3i{ 2, 1, 0 }));
  EXPECT_EQ(p1.toDenseMatrix(), A);
}

// elementary matrix: 当进行矩阵行（或列）的线性组合操作时，从理论角度来讲，这种操作可以通过初等矩阵来表示
// 即表达为 A 有 c(i) 列, 任意列是c(i)的线性组合时,可以由初等矩阵完成
TEST_F(Lesson2, elementary) {
  // 使得a00下的元素为0
  Matrix3d E00;

  E00 << RowVector3d{ 1, 0, 0 }, RowVector3d{ -4, 1, 0 }, RowVector3d{ -7, 0, 1 };

  // 使得E11下的元素为0
  Matrix3d E11;
  E11 << RowVector3d{ 1, 0, 0 }, RowVector3d{ 0, 1, 0 }, RowVector3d{ 0, -2, 1 };

  Matrix3d u = E11 * E00 * a_;

  EXPECT_EQ(u, u_);
}