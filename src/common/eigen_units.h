#include <Eigen/Dense>
#include <units.h>

namespace Eigen {

template <typename _UnitType>
struct NumTraits<units::unit_t<_UnitType>>
  : NumTraits<double> // permits to get the epsilon, dummy_precision, lowest,
                      // highest functions
{};

}
