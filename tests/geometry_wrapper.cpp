#include <boost/geometry/geometry.hpp>
#include <eigen3/Eigen/Dense>
#include <gtest/gtest.h>

namespace bg = boost::geometry;

// adapter
namespace boost {
namespace geometry {
namespace traits {

template <typename _Scalar, int _dim>
struct tag<Eigen::Vector<_Scalar, _dim>> {
  typedef point_tag type;
};

template <typename _Scalar, int _dim>
struct coordinate_type<Eigen::Vector<_Scalar, _dim>> {
  typedef _Scalar type;
};

template <typename _Scalar, int _dim>
struct coordinate_system<Eigen::Vector<_Scalar, _dim>> {
  typedef cs::cartesian type;
};

template <typename _Scalar, int _dim>
struct dimension<Eigen::Vector<_Scalar, _dim>> : boost::mpl::int_<_dim> {};

template <typename _Scalar, int _dim>
struct access<Eigen::Vector<_Scalar, _dim>, 0> {
  static _Scalar get(Eigen::Vector<_Scalar, _dim> const& p) { return p.x(); }

  static void set(Eigen::Vector<_Scalar, _dim>& p, _Scalar const& value) { p.x() = value; }
};

template <typename _Scalar, int _dim>
struct access<Eigen::Vector<_Scalar, _dim>, 1> {
  static _Scalar get(Eigen::Vector<_Scalar, _dim> const& p) { return p.y(); }

  static void set(Eigen::Vector<_Scalar, _dim>& p, _Scalar const& value) { p.y() = value; }
};

template <typename _Scalar, int _dim>
struct access<Eigen::Vector<_Scalar, _dim>, 2> {
  static _Scalar get(Eigen::Vector<_Scalar, _dim> const& p) { return p.z(); }

  static void set(Eigen::Vector<_Scalar, _dim>& p, _Scalar const& value) { p.z() = value; }
};
}
}
} // namespace boost::geometry::traits

template <typename _Scalar, int _dim>
using MyMultiPoint = boost::geometry::model::multi_point<Eigen::Vector<_Scalar, _dim>>;

TEST(geometry_eigen, adapter_point) {

  Eigen::Vector3d p1{ 1, 2, 3 };
  Eigen::Vector3d p2{ 1, 2, 3 };

  auto dis = boost::geometry::distance(p1, p2);

  EXPECT_EQ(dis, 0);

  Eigen::Vector2f p3{ 1, 2 };
  Eigen::Vector2f p4{ 1, 2 };

  auto dis2 = boost::geometry::distance(p1, p2);
  EXPECT_EQ(dis2, 0);

  // multi_point
  MyMultiPoint<double, 2> mpt;
  bg::append(mpt, Eigen::Vector2d{ 1, 1 });
  bg::append(mpt, Eigen::Vector2d{ 2, 2 });
  EXPECT_EQ(bg::num_points(mpt), 2);
}

template <typename _Scalar, int _dim>
using MyLineString = boost::geometry::model::linestring<Eigen::Vector<_Scalar, _dim>>;

TEST(geometry_eigen, MyLineString) {

  MyLineString<double, 2> ls1;
  ls1.push_back({ 1, 1 });
  ls1.push_back({ 1, 2 });
  ls1.push_back({ 2, 2 });

  auto length1 = bg::length(ls1);

  EXPECT_EQ(length1, 2);
}

template <typename _Scalar, int _dim>
using MyMultiLineString = boost::geometry::model::multi_linestring<MyLineString<_Scalar, _dim>>;

TEST(geometry_eigen, MyMultiLineString) {

  MyMultiLineString<double, 2> mls;
  mls.resize(2);

  auto& ls1 = mls[0];
  ls1.push_back({ 1, 1 });
  ls1.push_back({ 1, 2 });
  ls1.push_back({ 2, 2 });

  auto& ls2 = mls[0];
  ls2.push_back({ 2, 2 });
  ls2.push_back({ 2, 3 });
  ls2.push_back({ 3, 3 });

  auto length1 = bg::length(mls);

  EXPECT_EQ(length1, 4);
}

template <typename _Scalar, int _dim>
using MySegment = boost::geometry::model::segment<Eigen::Vector<_Scalar, _dim>>;

TEST(geometry_eigen, MySegment) {

  MySegment<double, 3> seg;
  seg.first = { 1, 1, 1 };
  seg.second = { 2, 2, 2 };

  auto length = bg::length(seg);

  EXPECT_DOUBLE_EQ(length, sqrt(3));

  // referring_segment
  bg::model::referring_segment<Eigen::Vector3d> ref_seg(seg.first, seg.second);
  ref_seg.first = { 3, 3, 3 };
  EXPECT_TRUE(seg.first.isApprox(ref_seg.first));
}

template <typename _Scalar, int _dim>
using MyBox = boost::geometry::model::box<Eigen::Vector<_Scalar, _dim>>;

TEST(geometry_eigen, MyBox) {

  MyBox<double, 2> box(Eigen::Vector2d{ 1, 1 }, Eigen::Vector2d{ 2, 2 });

  auto area = bg::area(box);
  EXPECT_DOUBLE_EQ(area, 1);

  Eigen::Vector2d pt{ 1.5, 1.5 };
  EXPECT_TRUE(bg::within(pt, box));
}

template <typename _Scalar, int _dim, bool _clockWise = true, bool _closed = true>
using MyRing = boost::geometry::model::ring<Eigen::Vector<_Scalar, _dim>, _clockWise, _closed>;

TEST(geometry_eigen, MyRing) {

  MyRing<double, 2> ring;

  ring.push_back({ 0, 0 });
  ring.push_back({ 0, 1 });
  ring.push_back({ 1, 1 });
  ring.push_back({ 1, 0 });
  ring.push_back({ 0, 0 });

  auto area = bg::area(ring);
  EXPECT_DOUBLE_EQ(area, 1);

  Eigen::Vector2d pt{ 0.5, 0.5 };
  EXPECT_TRUE(bg::within(pt, ring));
}

template <typename _Scalar, int _dim, bool _clockWise = true, bool _closed = true>
using MyPolygon = boost::geometry::model::polygon<Eigen::Vector<_Scalar, _dim>, _clockWise, _closed>;

TEST(geometry_eigen, MyPolygon) {
  MyPolygon<double, 2> polygon;

  polygon.outer().push_back({ -2, -2 });
  polygon.outer().push_back({ -2, 2 });
  polygon.outer().push_back({ 2, 2 });
  polygon.outer().push_back({ 2, -2 });
  polygon.outer().push_back({ -2, -2 });

  polygon.inners().resize(1);
  auto& inner = polygon.inners().front();
  inner.push_back({ -1, -1 });
  inner.push_back({ -1, 1 });
  inner.push_back({ 1, 1 });
  inner.push_back({ 1, -1 });
  inner.push_back({ -1, -1 });
  bg::reverse(inner);

  // 面积，顺时针是正数，逆时针是负数
  auto area0 = bg::area(polygon.outer());
  auto area1 = bg::area(polygon.inners().front());
  auto area2 = bg::area(polygon);
  EXPECT_DOUBLE_EQ(area0, 16);
  EXPECT_DOUBLE_EQ(area1, -4);
  EXPECT_DOUBLE_EQ(area2, 16 - 4);
}

template <typename _Scalar, int _dim, bool _clockWise = true, bool _closed = true>
using MyMultiPolygon = boost::geometry::model::multi_polygon<MyPolygon<_Scalar, _dim, _clockWise, _closed>>;

TEST(geometry_eigen, MyMultiPolygon) {

  MyMultiPolygon<double, 2> ppolygon;
  ppolygon.resize(2);

  // ploygon1
  auto& polygon = ppolygon.front();
  polygon.outer().push_back({ -2, -2 });
  polygon.outer().push_back({ -2, 2 });
  polygon.outer().push_back({ 2, 2 });
  polygon.outer().push_back({ 2, -2 });
  polygon.outer().push_back({ -2, -2 });

  polygon.inners().resize(1);
  auto& inner = polygon.inners().front();
  inner.push_back({ -1, -1 });
  inner.push_back({ -1, 1 });
  inner.push_back({ 1, 1 });
  inner.push_back({ 1, -1 });
  inner.push_back({ -1, -1 });
  bg::reverse(inner);

  // ploygon1
  auto& polygon2 = ppolygon.back();
  polygon2.outer().push_back({ -3, -3 });
  polygon2.outer().push_back({ -3, 3 });
  polygon2.outer().push_back({ 3, 3 });
  polygon2.outer().push_back({ 3, -3 });
  polygon2.outer().push_back({ -3, -3 });

  polygon2.inners().resize(1);
  auto& inner2 = polygon2.inners().front();
  inner2.push_back({ -1, -1 });
  inner2.push_back({ -1, 1 });
  inner2.push_back({ 1, 1 });
  inner2.push_back({ 1, -1 });
  inner2.push_back({ -1, -1 });
  bg::reverse(inner2);

  auto area = bg::area(ppolygon);

  EXPECT_DOUBLE_EQ(area, (16 - 4) + (36 - 4));
}