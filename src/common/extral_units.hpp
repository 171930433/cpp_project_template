#include <units.h>

namespace units::category {

typedef base_unit<detail::meter_ratio<0>, std::ratio<0>, std::ratio<-1, 2>, std::ratio<1>>
  angle_random_walk_unit; ///< Represents an SI base unit of angle_random_walk_unit rad/s^(1/2)

typedef base_unit<detail::meter_ratio<1>, std::ratio<0>, std::ratio<-3, 2>>
  velocity_random_walk_unit; ///< Represents an SI base unit of velocity_random_walk_unit m/s^(3/2)

}

namespace units {

UNIT_ADD(
  angular_velocity, degrees_per_hour, degrees_per_hour, deg_per_h, compound_unit<angle::degrees, inverse<time::hours>>)

// 角度随机游走
UNIT_ADD(angle_random_walk, degrees_per_second_sqrt, degrees_per_second_sqrt, dpss,
  compound_unit<angle::degree, inverse<unit<std::ratio<1>, detail::sqrt_base<category::time_unit>>>>)
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