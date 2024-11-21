#pragma once

#include <eigen3/Eigen/Dense>

namespace Eigen {
using VectorMap3d = Eigen::Map<Eigen::Vector3d>;
using CVectorMap3d = Eigen::Map<Eigen::Vector3d const>;
using CQuaternionMapd = Eigen::Map<Eigen::Quaterniond const>;
}
