#include <eigen3/Eigen/Dense>
#include <gtest/gtest.h>
// 线性方程组的几何解释  The Geometry of Linear Equations

using namespace Eigen;

class Lesson1 : public testing::Test {

public:
  Lesson1() {
    a_ << 2, 3, 1, -1;
    b_ << 7, 1;
    x_ << 2, 1;
  }
  void SetUp() override {}

  void TearDown() override {}

protected:
  Matrix2d a_;
  Vector2d b_;
  Vector2d x_;
};

// 行视图直接决定了b的单一元素
TEST_F(Lesson1, row_picture) {
  Vector2d b0;
  b0.row(0) = a_.row(0) * x_;
  b0.row(1) = a_.row(1) * x_;

  EXPECT_EQ(b0, b_);
}

// 列视图，描述为按照x的各个分量，对矩阵A的各列向量进行线性组合；
// b的所有取值，为以A的列向量为基底，所展开的空间
TEST_F(Lesson1, col_picture) {
  Vector2d b0;
  b0 = x_[0] * a_.col(0) + x_[1] * a_.col(1);

  EXPECT_EQ(b0, b_);
}

TEST(problem_set_1_1, 1) {
  Vector3d a1{ 1, 2, 3 };
  Vector3d a2{ 3, 6, 9 };
  EXPECT_TRUE(!std::string("all linear combination of a_i is line").empty());

  Vector3d b1{ 1, 0, 0 };
  Vector3d b2{ 0, 2, 3 };
  EXPECT_TRUE(!std::string("all linear combination of b_i is plane").empty());

  Vector3d c1{ 2, 0, 0 };
  Vector3d c2{ 0, 2, 2 };
  Vector3d c3{ 2, 2, 3 };
  EXPECT_TRUE(!std::string("all linear combination of b_i is R^3").empty());
}

TEST(problem_set_1_1, 2) {
  Vector2d v{ 4, 1 };
  Vector2d w{ -2, 2 };

  Vector2d v_plus_w = v + w;
  Vector2d v_minus_w = v - w;

  EXPECT_TRUE(v_plus_w.isApprox(Vector2d{ 2, 3 }));
  EXPECT_TRUE(v_minus_w.isApprox(Vector2d{ 6, -1 }));
}

TEST(problem_set_1_1, 3) {
  Vector2d v_plus_w{ 5, 1 };
  Vector2d v_minus_w{ 1, 5 };

  Vector2d v = (v_plus_w + v_minus_w) / 2;
  Vector2d w = (v_plus_w - v_minus_w) / 2;

  EXPECT_TRUE(v.isApprox(Vector2d{ 3, 3 }));
  EXPECT_TRUE(w.isApprox(Vector2d{ 2, -2 }));
}

TEST(problem_set_1_1, 4) {
  Vector2d v{ 2, 1 };
  Vector2d w{ 1, 2 };

  Vector2d re = 3 * v + w;
  EXPECT_TRUE(re.isApprox(Vector2d{ 7, 5 }));
}

TEST(problem_set_1_1, 5) {
  Vector3d u{ 1, 2, 3 };
  Vector3d v{ -3, 1, -2 };
  Vector3d w{ 2, -3, -1 };

  Vector3d uvw = u + v + w;
  Vector3d _2u_2v_w = 2 * u + 2 * v + w;

  EXPECT_TRUE(uvw.isApprox(Vector3d::Zero()));
  EXPECT_TRUE(_2u_2v_w.isApprox(Vector3d{ -2, 3, 1 }));

  EXPECT_TRUE(!std::string("uvw == 0,所以w是u,v的线性组合,在一个平面").empty());
}

TEST(problem_set_1_1, 6) {
  Vector3d u{ 1, -2, 1 };
  Vector3d v{ 0, 1, -1 };

  double re = u.array().sum() + v.array().sum();

  EXPECT_EQ(re, 0);

  Matrix<double, 3, 2> uv;
  uv << u, v;

  Vector3d cv_dw{ 3, 3, -6 };

  Vector2d x = (uv.transpose() * uv).ldlt().solve(uv.transpose() * cv_dw);

  GTEST_LOG_(INFO) << x.transpose();

  EXPECT_TRUE(x.isApprox(Vector2d{ 3, 9 }));
}

TEST(problem_set_1_1, 7_31) {
  EXPECT_TRUE(1);
}