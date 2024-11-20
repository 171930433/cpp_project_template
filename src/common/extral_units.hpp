#include <units.h>

namespace units::category {

typedef base_unit<detail::meter_ratio<0>, std::ratio<0>, std::ratio<-1, 2>, std::ratio<1>>
  angle_random_walk_unit; ///< Represents an SI base unit of angle_random_walk_unit rad/s^(1/2)

typedef base_unit<detail::meter_ratio<1>, std::ratio<0>, std::ratio<-3, 2>>
  velocity_random_walk_unit; ///< Represents an SI base unit of velocity_random_walk_unit m/s^(3/2)

}

namespace units {

UNIT_ADD(
  angular_velocity, degrees_per_hour, degrees_per_hour, dph, compound_unit<angle::degrees, inverse<time::hours>>)

// 角度随机游走
UNIT_ADD(angle_random_walk, radius_per_sqrt_second, radius_per_sqrt_second, rpss,
  unit<std::ratio<1>, category::angle_random_walk_unit>)
UNIT_ADD(angle_random_walk, degrees_per_second_sqrt, degrees_per_second_sqrt, dpss,
  unit<std::ratio<1, 180>, radius_per_sqrt_second, std::ratio<1>>)
UNIT_ADD(angle_random_walk, degrees_per_hour_sqrt, degrees_per_hour_sqrt, dpsh,
  unit<std::ratio<1, 60>, degrees_per_second_sqrt>)
UNIT_ADD_CATEGORY_TRAIT(angle_random_walk)

// 速度随机游走
UNIT_ADD(velocity_random_walk, g_per_sqrt_hz, g_per_sqrt_hz, gpshz,
  compound_unit<acceleration::standard_gravity,
    inverse<unit<std::ratio<1>, detail::sqrt_base<category::frequency_unit>>>>)
UNIT_ADD(velocity_random_walk, mg_per_sqrt_hz, mg_per_sqrt_hz, mgpshz, unit<std::ratio<1, 1000>, g_per_sqrt_hz>)
UNIT_ADD(velocity_random_walk, ug_per_sqrt_hz, ug_per_sqrt_hz, ugpshz, unit<std::ratio<1, 1000>, mg_per_sqrt_hz>)
UNIT_ADD_CATEGORY_TRAIT(velocity_random_walk)

}

template <class _Units, typename _T, template <typename> class _NonLinearScale>
inline constexpr _T SI(units::unit_t<_Units, _T, _NonLinearScale> const& obj) {
  using namespace units;
  using BaseUnits = unit<std::ratio<1>, typename traits::unit_traits<_Units>::base_unit_type>;
  return convert<_Units, BaseUnits>(obj());
}
