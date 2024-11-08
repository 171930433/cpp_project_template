#include "rref.h"
#include <eigen3/Eigen/Dense>
#include <gtest/gtest.h>

//   Ax=b
using namespace Eigen;

class Lesson8 : public testing::Test {

public:
  Lesson8() { a_ << RowVector4d{ 1, 2, 2, 2 }, RowVector4d{ 2, 4, 6, 8 }, RowVector4d{ 3, 6, 8, 10 }; }
  void SetUp() override {}

  void TearDown() override {}

protected:
  Matrix<double, 3, 4> a_;
};

TEST_F(Lesson8, full_row_rank) {
  constexpr int m = 2;
  constexpr int n = 4;
  Matrix<double, m, n> A;
  A << a_.row(0), a_.row(2);

  // rref is [I F],
  auto const& re = RREF2(A);

  EXPECT_EQ(m, re.rank_);
}