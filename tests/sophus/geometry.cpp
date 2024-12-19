#include "common/extral_units.hpp"
#include <eigen3/unsupported/Eigen/EulerAngles>
#include <gtest/gtest.h>
#include <sophus/geometry.hpp>
#include <ylt/easylog.hpp>

using namespace Sophus;
using namespace Eigen;
using namespace units;
using namespace units::literals;
using namespace units::angle;

TEST(sophus_geometry, so_normal) {
  SO2d Rnv(M_PI_4);

  Vector2d normal = normalFromSO2(Rnv);
  EXPECT_TRUE(normal.isApprox(Vector2d(-1, 1).normalized()));

  SO2d Rnv2 = SO2FromNormal(normal);
  EXPECT_TRUE(Rnv2.matrix().isApprox(Rnv.matrix()));

  SO3d Rnv_3d{ Quaterniond{ EulerAnglesZXYd{ SI(1_deg), SI(2_deg), SI(3_deg) } } };

  Vector3d normal_3d = normalFromSO3(Rnv_3d);

  SO3d Rnv_3d_2 = SO3FromNormal(normal_3d);
  EXPECT_TRUE(Rnv_3d_2.matrix().col(2).isApprox(Rnv_3d.matrix().col(2)));
}

TEST(sophus_geometry, se2_hyperplane) {
  // 2d hyperplane is line
  SE2d Twv{ M_PI_4, Vector2d{ 1, 1 } };

  // Twv坐标系的x轴
  Line2d line1 = lineFromSE2(Twv);
  Line2d line_true{ normalFromSO2(Twv.so2()), Twv.translation() };

  EXPECT_TRUE(line1.isApprox(line_true));

  // 根据直线的点法构建一个se2, 原点并不是直线的原点,而是W的原点在直线的投影点,也比较合理
  // 因为所有共线的直线应该只对应一个SE2
  SE2d Twv2 = SE2FromLine(line_true);
  SE2d Twv2_true{ SO2FromNormal((Vector2d)line_true.normal()), line_true.projection(Vector2d::Zero()) };

  ELOGD << "Twv = \n" << Twv.matrix();
  ELOGD << "Twv2 = \n" << Twv2.matrix();
  ELOGD << "Twv2_true = \n" << Twv2_true.matrix();
  EXPECT_TRUE(Twv2.matrix().isApprox(Twv2_true.matrix()));
}

// offset是原点到直线或者平面的距离
TEST(sophus_geometry, se3_hyperplane) {
  SE3d Twv{ Quaterniond{ AngleAxisd{ M_PI_4, Vector3d::UnitZ() } }, Vector3d::Constant(1) };

  // 获得Twv的xy轴对应的平面
  Plane3d plane1 = planeFromSE3(Twv);
  Plane3d plane_true{ normalFromSO3(Twv.so3()), Twv.translation() };

  EXPECT_TRUE(plane1.isApprox(plane_true));
}