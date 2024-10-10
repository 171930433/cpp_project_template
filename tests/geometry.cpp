#include <gtest/gtest.h>

struct MyPoint {
  double x;
  double y;
};

namespace traits {

template <typename _T, int _D>
struct access;

template <>
struct access<MyPoint, 0> {
  double static get(MyPoint const& pt) { return pt.x; };
};

template <>
struct access<MyPoint, 1> {
  double static get(MyPoint const& pt) { return pt.y; };
};

}

template <typename _T, int _D>
double get(_T const& pt) {
  return traits::access<_T, _D>::get(pt);
}

template <typename _T1, typename _T2>
double Distance(_T1 const& p1, _T2 const& p2) {
  double d1 = get<_T1, 0>(p1) - get<_T2, 0>(p2);
  double d2 = get<_T1, 1>(p1) - get<_T2, 1>(p2);
  return d1 * d1 + d2 * d2;
}

TEST(geometry, distance1) {
  MyPoint p1{ 0, 0 };
  MyPoint p2{ 1, 1 };

  double dis = Distance(p1, p2);

  EXPECT_EQ(dis, 2);
}

namespace traits {
template <typename _T>
struct dimension {};

template <>
struct dimension<MyPoint> : public std::integral_constant<int, 2> {};

}

template <typename _T>
struct dimension : public traits::dimension<_T> {};

template <typename _T1, typename _T2, int _d>
struct pythagoras {
  double static apply(_T1 const& p1, _T2 const& p2) {
    double d = get<_T1, _d - 1>(p1) - get<_T2, _d - 1>(p2);
    return d * d + pythagoras<_T1, _T2, _d - 1>::apply(p1, p2);
  }
};

template <typename _T1, typename _T2>
struct pythagoras<_T1, _T2, 0> {
  double static apply(_T1 const& p1, _T2 const& p2) { return 0; }
};

template <typename _T1, typename _T2>
double Distance2(_T1 const& p1, _T2 const& p2) {
  static_assert(dimension<_T1>::value == dimension<_T2>::value);
  return pythagoras<_T1, _T2, dimension<_T1>::value>::apply(p1, p2);
}

TEST(geometry, distance2) {
  MyPoint p1{ 0, 0 };
  MyPoint p2{ 1, 1 };

  double dis = Distance2(p1, p2);

  EXPECT_EQ(dis, 2);
}