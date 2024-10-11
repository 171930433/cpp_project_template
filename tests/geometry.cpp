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

struct MyPoint3 {
  double x;
  double y;
  double z;
};

namespace traits {

template <>
struct dimension<MyPoint3> : public std::integral_constant<int, 3> {};

template <>
struct access<MyPoint3, 0> {
  double static get(MyPoint3 const& pt) { return pt.x; };
};

template <>
struct access<MyPoint3, 1> {
  double static get(MyPoint3 const& pt) { return pt.y; };
};

template <>
struct access<MyPoint3, 2> {
  double static get(MyPoint3 const& pt) { return pt.z; };
};

}
// Dimension Agnosticism
TEST(geometry, Distance2) {
  MyPoint3 p1{ 0, 0, 0 };
  MyPoint3 p2{ 1, 1, 1 };

  double dis = Distance2(p1, p2);

  EXPECT_EQ(dis, 3);
}

namespace traits {
template <typename _T>
struct coordinate_type {};

template <>
struct coordinate_type<MyPoint> {
  using type = double;
};

}

template <typename _T>
struct coordinate_type : public traits::coordinate_type<_T> {};

template <typename _T1, typename _T2, int _d>
struct pythagoras2 {

  using _CommonType = std::common_type_t<typename coordinate_type<_T1>::type, typename coordinate_type<_T2>::type>;

  _CommonType static apply(_T1 const& p1, _T2 const& p2) {
    _CommonType d = get<_T1, _d - 1>(p1) - get<_T2, _d - 1>(p2);
    return d * d + pythagoras2<_T1, _T2, _d - 1>::apply(p1, p2);
  }
};

template <typename _T1, typename _T2>
struct pythagoras2<_T1, _T2, 0> {
  int static apply(_T1 const& p1, _T2 const& p2) { return 0; }
};

template <typename _Scalar>
struct MyPoint2 {
  _Scalar x;
  _Scalar y;
};

using MyPoint2d = MyPoint2<double>;
using MyPoint2f = MyPoint2<float>;

namespace traits {

template <typename _Scalar>
struct access<MyPoint2<_Scalar>, 0> {
  static _Scalar get(MyPoint2<_Scalar> const& pt) { return pt.x; }
};

template <typename _Scalar>
struct access<MyPoint2<_Scalar>, 1> {
  static _Scalar get(MyPoint2<_Scalar> const& pt) { return pt.y; }
};

template <typename _Scalar>
struct dimension<MyPoint2<_Scalar>> : public std::integral_constant<int, 2> {};

template <typename _Scalar>
struct coordinate_type<MyPoint2<_Scalar>> {
  using type = _Scalar;
};

}

template <typename _T1, typename _T2>
auto Distance3(_T1 const& p1, _T2 const& p2) {
  static_assert(dimension<_T1>::value == dimension<_T2>::value);
  return pythagoras2<_T1, _T2, dimension<_T1>::value>::apply(p1, p2);
}

// Coordinate Type
TEST(geometry, Distance3) {
  MyPoint2d p1{ 0, 0 };
  MyPoint2d p2{ 1, 1 };

  auto dis1 = Distance3(p1, p2);

  EXPECT_TRUE((std::is_same_v<decltype(dis1), double>));

  EXPECT_EQ(dis1, 2);

  MyPoint2f p3{ 0, 0 };
  MyPoint2f p4{ 1, 1 };

  auto dis2 = Distance3(p3, p4);
  EXPECT_TRUE((std::is_same_v<decltype(dis2), float>));

  EXPECT_EQ(dis2, 2);
}