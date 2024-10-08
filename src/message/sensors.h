#pragma once

struct Vec2d {
  double x_ = 0;
  double y_ = 0;
};

struct Vec3d {
  double x_ = 0;
  double y_ = 0;
  double z_ = 0;
};

struct Imu {
  double t0_ = 0;           // 产生时间 s
  Vec3d acc_ = { 0, 0, 0 }; // imu系比力观测值
  Vec3d gyr_ = { 0, 0, 0 }; // imu系陀螺观测值
  double temperature_ = 0;  // 温度 摄氏度
};

struct GnssPosition {
  double t0_ = 0; // 产生时间 s
  double hdop = 0;
  Vec3d pos = { 0, 0, 0 };
  Vec3d pos_llh = { 0, 0, 0 };
  int status = 0;
  int sat_num = 0;
  double rtk_age = 0;
  Vec3d std_pos = { 0, 0, 0 };
};

struct GnssVelocity {
  double t0_ = 0; // 产生时间 s
  Vec3d vel = { 0, 0, 0 };
  double azi_track = 0;
  double speed = 0;
  Vec3d std_vel = { 0, 0, 0 };
};

struct GnssDualAntenna {
  double t0_ = 0; // 产生时间 s
  Vec2d dual_antenna_angle = { 0, 0 };
  Vec2d dual_antenna_std = { 0, 0 };
  int heading_status = 0;
  int heading_sat_num = 0;
};

struct Gnss {
  double t0_ = 0; // 产生时间 s
  GnssPosition pos_;
  GnssVelocity vel_;
  GnssDualAntenna dual_antenna_;
};

struct State {
  double t0_ = 0; // 产生时间 s
  Vec3d pos_ = { 0, 0, 0 };
  Vec3d vel_ = { 0, 0, 0 };
  Vec3d att_ = { 0, 0, 0 };
};

struct FramesPack {
  Imu imu;
  GnssPosition gnss_pos;
  GnssVelocity gnss_vel;
  GnssDualAntenna gnss_dual_antenna;
  Gnss gnss;
  //   状态
  State state;
};