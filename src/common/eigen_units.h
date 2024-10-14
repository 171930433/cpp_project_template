#include <Eigen/Dense>
#include <units.h>
namespace Eigen {

template <typename _UnitType, typename _Scalar, template <typename> typename _NonLinearScale>
struct NumTraits<units::unit_t<_UnitType, _Scalar, _NonLinearScale>>
  : NumTraits<_Scalar> // permits to get the epsilon, dummy_precision, lowest, highest functions
{};

}


namespace units {
namespace traits {
template <typename _T>
constexpr bool is_unit_t_v = is_unit_t<_T>::value;
}
}

// 将任意单位的Eigen容器,转换成对应的标准单位制对应的无单位数值类型 International System of Units
template <typename _Derived, typename _Scalar = typename Eigen::internal::traits<_Derived>::Scalar,
  typename = std::enable_if_t<units::traits::is_unit_t_v<_Scalar>>>
inline auto SI(const Eigen::DenseBase<_Derived>& b) {

  using _underlying_type = typename units::traits::unit_t_traits<_Scalar>::underlying_type;
  using _unit_type = typename units::traits::unit_t_traits<_Scalar>::unit_type;
  using _base_unit_type = units::unit<std::ratio<1>, typename units::traits::unit_traits<_unit_type>::base_unit_type>;
  using _base_unit_type_t = units::unit_t<_base_unit_type>;

  return b.template cast<_base_unit_type_t>().template cast<_underlying_type>();
}