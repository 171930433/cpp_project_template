#include <boost/geometry/geometry.hpp>
#include <eigen3/Eigen/Dense>
#include <gtest/gtest.h>

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
TEST(geometry_eigen, adapter_point) {

  Eigen::Vector3d p1{ 1, 2, 3 };
  Eigen::Vector3d p2{ 1, 2, 3 };

  auto dis = boost::geometry::distance(p1, p2);

  EXPECT_EQ(dis, 0);

  Eigen::Vector2f p3{ 1, 2 };
  Eigen::Vector2f p4{ 1, 2 };

  auto dis2 = boost::geometry::distance(p1, p2);
  EXPECT_EQ(dis2, 0);
}