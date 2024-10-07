#include "common/earth.hpp"
#include "common/eigen_units.h"
#include <gtest/gtest.h>

using namespace units::angle;
using namespace units::literals;

TEST(Earth, base) {

  WGS84 wgs84;

  Eigen::Vector3d ecef_true = { 801927, -5.13661e+06, 3.68258e+06 };
  Eigen::Vector3d llh{ degree_t(120.0_deg).to<double>(), degree_t(30.0_deg()).to<double>(), 0 };
  Eigen::Vector3d ecef = WGS84::LLH2ECEF(llh);

  GTEST_LOG_(INFO) << ecef.transpose();

  EXPECT_TRUE(ecef_true.isApprox(ecef, 1e-6));
}

TEST(Earth, origin) {

  Eigen::Vector3d origin{ degree_t(120.0_deg).to<double>(), degree_t(30.0_deg()).to<double>(), 0 };
  WGS84 wgs84(origin);

  Eigen::Vector3d enu_true = { 25901.5, 31823, -122.186 };
  Eigen::Vector3d llh{ degree_t(120.005_deg).to<double>(), degree_t(30.005_deg()).to<double>(), 10 };
  Eigen::Vector3d enu = wgs84.LLH2ENU(llh);

  GTEST_LOG_(INFO) << enu.transpose();

  EXPECT_TRUE(enu_true.isApprox(enu, 1e-6));
}