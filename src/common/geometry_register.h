#pragma once
#include <boost/geometry/geometry.hpp>
#include <boost/qvm.hpp>
#include <eigen3/Eigen/Geometry>

namespace bg = boost::geometry;
namespace bgm = boost::geometry::model;
namespace bgt = boost::geometry::traits;

// adapter point
// https://www.boost.org/doc/libs/1_86_0/libs/geometry/doc/html/geometry/examples/example_source_code__adapting_a_legacy_geometry_object_model.html#adaption_of_qpoint_source_code
namespace boost::geometry::traits {

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

template <typename _Scalar, int _dim, unsigned long _index>
struct access<Eigen::Vector<_Scalar, _dim>, _index> {
  static _Scalar get(Eigen::Vector<_Scalar, _dim> const& p) { return p.data()[_index]; }

  static void set(Eigen::Vector<_Scalar, _dim>& p, _Scalar const& value) { p.data()[_index] = value; }
};

template <typename _Geometry>
constexpr bool is_geometry_v = !std::is_void_v<typename tag<_Geometry>::type>;

}

// adaper Eigen::Transform to qvm matrix
// https://www.boost.org/doc/libs/1_83_0/libs/qvm/doc/html/index.html#_quaternions_vectors_matrices
namespace boost::qvm {

template <typename _Scalar, int _dim, int _mode>
struct mat_traits<Eigen::Transform<_Scalar, _dim, _mode>>
  : mat_traits_defaults<Eigen::Transform<_Scalar, _dim, _mode>, _Scalar, _dim + 1, _dim + 1> {

  using EigenType = Eigen::Transform<_Scalar, _dim, _mode>;

  template <int _R, int _C>
  static inline _Scalar& write_element(EigenType& m) {
    return m.matrix()(_R, _C);
  }

  static inline _Scalar& write_element_idx(int r, int c, EigenType& m) { return m.matrix(r, c); }
};

}

// Eigen::Transform * _Geometry
template <typename _Transform, typename _Geometry, typename = std::enable_if_t<bgt::is_geometry_v<_Geometry>>>
inline _Geometry operator*(_Transform const& transform, _Geometry const& geometry) {
  boost::geometry::concepts::check<_Geometry>();

  using _PointType = typename bg::traits::point_type<_Geometry>::type;
  using _PointScalar = typename _PointType::Scalar;
  static constexpr int _point_dimension = Eigen::internal::traits<_PointType>::RowsAtCompileTime;
  static constexpr int _trans_dimension = Eigen::internal::transform_traits<_Transform>::Dim;

  static_assert(std::is_same_v<_PointScalar, typename _Transform::Scalar>, "Scalar type mismatch");
  static_assert(_point_dimension == _trans_dimension, "dimentation mismatch");

  _Geometry result;
  bg::strategy::transform::matrix_transformer<_PointScalar, _point_dimension, _point_dimension> transformer(transform);
  bg::transform(geometry, result, transformer);
  return result;
}