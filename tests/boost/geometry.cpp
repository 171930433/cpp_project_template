#include <gtest/gtest.h>

#include <eigen3/Eigen/Dense>
#include <eigen3/Eigen/Geometry>

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

struct PointTag {};
struct LineTag {};

namespace traits {
template <typename _T>
struct tag {};

template <typename _Scalar>
struct tag<MyPoint2<_Scalar>> {
  using type = PointTag;
};

}

template <typename _T>
struct tag : public traits::tag<_T> {};

namespace dispatch {
template <typename _Tag1, typename _Tag2, typename _G1, typename _G2>
struct distance;

template <typename _G1, typename _G2>
struct distance<PointTag, PointTag, _G1, _G2> {
  static auto apply(_G1 const& g1, _G2 const& g2) {
    static_assert(dimension<_G1>::value == dimension<_G2>::value);
    return pythagoras2<_G1, _G2, dimension<_G1>::value>::apply(g1, g2);
  }
};

template <typename _G1, typename _G2>
struct distance<PointTag, LineTag, _G1, _G2> {
  static auto apply(_G1 const& g1, _G2 const& g2) {
    return g2.distance(Eigen::Vector2d{ get<_G1, 0>(g1), get<_G1, 1>(g1) });
  }
};

}

template <typename _G1, typename _G2>
auto Distance4(_G1 const& g1, _G2 const& g2) {
  return dispatch::distance<typename tag<_G1>::type, typename tag<_G2>::type, _G1, _G2>::apply(g1, g2);
}

namespace traits {

template <typename _Scalar, int _dim>
struct tag<Eigen::ParametrizedLine<_Scalar, _dim>> {
  using type = LineTag;
};
}

// Different Geometries
TEST(geometry, Distance4) {
  MyPoint2d p1{ 0, 0 };
  MyPoint2d p2{ 1, 1 };

  auto dis1 = Distance4(p1, p2);

  EXPECT_EQ(dis1, 2);

  Eigen::ParametrizedLine<double, 2> line{ Eigen::Vector2d::Zero(), Eigen::Vector2d::UnitY() };
  auto dis2 = Distance4(p1, line);
  EXPECT_EQ(dis2, 0);
}

enum class CoordinateSystem { Meter, Radius, Degree };

// 使用eigen类型作为自定义类型的点
template <typename _Scalar, int _dim, CoordinateSystem _cs = CoordinateSystem::Meter>
class EigenPoint : public Eigen::Vector<_Scalar, _dim> {
  using Eigen::Vector<_Scalar, _dim>::Vector;
};

template <typename _Scalar, int _dim>
class Myline : public Eigen::ParametrizedLine<_Scalar, _dim> {
  using Eigen::ParametrizedLine<_Scalar, _dim>::ParametrizedLine;
};

namespace traits {

template <typename _Scalar, int _dim>
struct access<EigenPoint<_Scalar, _dim>, 0> {
  _Scalar static get(EigenPoint<_Scalar, _dim> const& pt) { return pt.x(); };
};

template <typename _Scalar, int _dim>
struct access<EigenPoint<_Scalar, _dim>, 1> {
  _Scalar static get(EigenPoint<_Scalar, _dim> const& pt) { return pt.y(); };
};

template <typename _Scalar, int _dim>
struct access<EigenPoint<_Scalar, _dim>, 2> {
  _Scalar static get(EigenPoint<_Scalar, _dim> const& pt) { return pt.z(); };
};

template <typename _Scalar, int _dim, CoordinateSystem _cs>
struct tag<EigenPoint<_Scalar, _dim, _cs>> {
  using TagType = PointTag;
  using coordinate_type = _Scalar;
  static constexpr int dim = _dim;
  static constexpr CoordinateSystem cs = _cs;
};

template <typename _Scalar, int _dim>
struct tag<Myline<_Scalar, _dim>> {
  using TagType = LineTag;
  using coordinate_type = _Scalar;
  static constexpr int dim = _dim;
};

}

// template <typename _T, int _D>
// double get(_T const& pt) {
//   return traits::access<_T, _D>::get(pt);
// }

template <typename _T>
struct MyTag : public traits::tag<_T> {};

template <typename _T1, typename _T2, int _d>
struct MyPythagoras {

  using _CommonType = std::common_type_t<typename MyTag<_T1>::coordinate_type, typename MyTag<_T2>::coordinate_type>;

  _CommonType static Apply(_T1 const& p1, _T2 const& p2) {
    _CommonType d = get<_T1, _d - 1>(p1) - get<_T2, _d - 1>(p2);
    return d * d + MyPythagoras<_T1, _T2, _d - 1>::Apply(p1, p2);
  }
};

template <typename _T1, typename _T2>
struct MyPythagoras<_T1, _T2, 0> {
  int static Apply(_T1 const& p1, _T2 const& p2) { return 0; }
};

struct DegreeDistance {
  template <typename _G1, typename _G2>
  int static Apply(_G1 const& p1, _G2 const& p2) {
    return 0;
  }
};

namespace dispatcher {

template <CoordinateSystem _cs1, CoordinateSystem _cs2, typename _G1, typename _G2>
struct DistanceStrategy;

template <typename _G1, typename _G2>
struct DistanceStrategy<CoordinateSystem::Meter, CoordinateSystem::Meter, _G1, _G2> {
  using Func = MyPythagoras<_G1, _G2, MyTag<_G1>::dim>;
};

template <typename _G1, typename _G2>
struct DistanceStrategy<CoordinateSystem::Degree, CoordinateSystem::Degree, _G1, _G2> {
  using Func = DegreeDistance;
};

template <typename _Tag1, typename _Tag2, typename _G1, typename _G2>
struct Distance;

template <typename _G1, typename _G2>
struct Distance<PointTag, PointTag, _G1, _G2> {
  static auto Apply(_G1 const& g1, _G2 const& g2) {
    static_assert(MyTag<_G1>::dim == MyTag<_G2>::dim);
    using DistanceFunc = typename DistanceStrategy<MyTag<_G1>::cs, MyTag<_G2>::cs, _G1, _G2>::Func;
    return DistanceFunc::Apply(g1, g2);
  }
};

template <typename _G1, typename _G2>
struct Distance<PointTag, LineTag, _G1, _G2> {
  static_assert(MyTag<_G1>::cs == CoordinateSystem::Meter);
  static auto Apply(_G1 const& g1, _G2 const& g2) { return g2.distance({ get<_G1, 0>(g1), get<_G1, 1>(g1) }); }
};

}

template <typename _G1, typename _G2>
auto Distance5(_G1 const& g1, _G2 const& g2) {
  return dispatcher::Distance<typename MyTag<_G1>::TagType, typename MyTag<_G2>::TagType, _G1, _G2>::Apply(g1, g2);
}

TEST(geometry, eigen) {
  EigenPoint<double, 2> p1{ 0, 0 };
  EigenPoint<float, 2> p2{ 1, 1 };

  auto dis1 = Distance5(p1, p2);

  EXPECT_EQ(dis1, 2);

  Myline<double, 2> line{ Eigen::Vector2d::Zero(), Eigen::Vector2d::UnitY() };
  auto dis2 = Distance5(p1, line);
  EXPECT_EQ(dis2, 0);

  EigenPoint<double, 2, CoordinateSystem::Degree> p3{ 0, 0 };
  auto dis3 = Distance5(p3, p3);
  EXPECT_EQ(dis3, 0);
}