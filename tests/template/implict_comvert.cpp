#include <common/extral_units.hpp>
#include <gtest/gtest.h>
class MyClass {
private:
  double value;

public:
  MyClass(double val)
    : value(val) {}
  operator double() { return value; }
};

double Fun1(double x) { return x; }

class MyClass2 {
private:
  double value;

public:
  MyClass2(double val)
    : value(val) {}

  template <typename _T>
  operator _T() {
    return value;
  }
};

template <typename _T>
struct MyFun {
  _T Fun(_T x) { return x; }
};

TEST(implict_convert, base) {

  MyClass2 obj(5.0);
  double result = obj;
  EXPECT_DOUBLE_EQ(result, 5);
  EXPECT_DOUBLE_EQ(Fun1(obj), 5);
}

TEST(implict_convert, base2) {

  using namespace units;
  using namespace units::literals;
  using namespace units::velocity_random_walk;

  MyClass2 obj(5.0);
  double result = obj;
  EXPECT_DOUBLE_EQ(result, 5);
  EXPECT_DOUBLE_EQ(Fun1(obj), 5);

  MyFun<double> mf;
  EXPECT_DOUBLE_EQ(mf.Fun(obj), 5);
  EXPECT_DOUBLE_EQ(Fun1(10.0_ugpshz()), 5);
  // EXPECT_DOUBLE_EQ(mf.Fun(10.0_ugpshz), 10.0_ugpshz());
}