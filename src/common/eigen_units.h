#include <Eigen/Dense>
#include <units.h>
namespace Eigen {

template <typename _UnitType>
struct NumTraits<units::unit_t<_UnitType>>
  : NumTraits<double> // permits to get the epsilon, dummy_precision, lowest,
                      // highest functions
{};

}

// 将任意单位的Eigen容器,转换成对应的标准单位制对应的无单位数值类型
template <typename _Derived, typename _Unit_t = typename Eigen::internal::traits<_Derived>::Scalar,
  typename _Units = _Unit_t::unit_type,
  typename _BaseUnits =
    typename units::unit<std::ratio<1>, typename units::traits::unit_traits<_Units>::base_unit_type>>
inline auto SI(const Eigen::DenseBase<_Derived>& b) {
  using _BaseUnit_t = units::unit_t<_BaseUnits>;
  using _Scalar = _Unit_t::underlying_type;
  return b.template cast<_BaseUnit_t>().template cast<_Scalar>();
}