#pragma once
#include "common/earth.hpp"
#include "message/sensors.h"
#include <eigen3/Eigen/Dense>
#include <eigen3/Eigen/Geometry>
#include <eigen3/unsupported/Eigen/EulerAngles>
#include <fmt/core.h>
#include <fmt/ostream.h>
#include <memory>
#include <unordered_set>
#include <ylt/reflection/template_string.hpp>
#include <ylt/struct_json/json_reader.h>
#include <ylt/struct_json/json_writer.h>
#include <ylt/struct_pb.hpp>

inline Eigen::Vector3d ToVector3d(Vec3d const& v3d) { return { v3d.x_, v3d.y_, v3d.z_ }; }

inline Vec3d ToVec3d(Eigen::Vector3d const& v3d) { return { v3d.x(), v3d.y(), v3d.z() }; }

inline Eigen::Isometry3d ToIsometry3d(Gnss const& gnss) {
  Eigen::Vector3d pos = ToVector3d(gnss.pos_.pos);
  Eigen::AngleAxisd aa(gnss.dual_antenna_.dual_antenna_angle.x_, Eigen::Vector3d::UnitZ());
  return Eigen::Translation3d(pos) * aa;
}

inline Eigen::Isometry3d ToIsometry3d(State const& state) {
  Eigen::Vector3d pos = ToVector3d(state.pos_);
  Eigen::EulerAnglesZXYd att{ state.att_.z_, state.att_.x_, state.att_.y_ };
  return Eigen::Translation3d(pos) * att;
}

namespace Eigen {
template <bool Is_writing_escape, typename Stream>
IGUANA_INLINE void to_json_impl(Stream& ss, Eigen::Isometry3d const& v) {
  double data[6] = { 0 };
  Eigen::Map<Eigen::Vector3d> pos(data);
  pos = v.translation();
  Eigen::Map<Eigen::Vector3d> att(data + 3);
  att = v.rotation().eulerAngles(2, 0, 1);
  att = att.array() * 180 / M_PI;

  iguana::detail::to_json_impl<Is_writing_escape>(ss, data);
}
}

template <typename _T>
struct IsTrajectory : public std::false_type {};

template <>
struct IsTrajectory<State> : public std::true_type {};

template <>
struct IsTrajectory<Gnss> : public std::true_type {};

template <typename _T>
constexpr bool IsTrajectory_v = IsTrajectory<_T>::value;

struct MessageBase {
  using SPtr = std::shared_ptr<MessageBase>;
  using SCPtr = std::shared_ptr<MessageBase const>;
  using Func = std::function<void(SPtr)>;
  using CFunc = std::function<void(SCPtr)>;
  virtual ~MessageBase() = default;
  virtual double t0() const = 0;
  double t1() const { return t1_; };
  virtual std::string to_json() const = 0;
  std::string to_header_str() const {
    std::stringstream ss;
    ss << fmt::format(R"("channel_name_":"{}",)", channel_name_);
    ss << fmt::format(R"("channel_type_":"{}",)", channel_type_);
    ss << fmt::format(R"("t0_":{:.3f},)", t0());
    ss << fmt::format(R"("t1_":{:.3f},)", t1_);
    ss << fmt::format(R"("t2_":{:.3f})", t2_);
    return ss.str();
  }

public:
  std::string_view channel_name_ = "";
  std::string_view channel_type_ = "";
  double t1_ = 0; // 接收到时间
  double t2_ = 0; // 落盘时间
};

template <typename _Sensor, bool = IsTrajectory<_Sensor>::value>
struct Message;

template <typename _Sensor>
struct Message<_Sensor, false> : public MessageBase {
  using SPtr = std::shared_ptr<Message>;
  using SCPtr = std::shared_ptr<Message const>;
  using Func = std::function<void(SPtr)>;
  using CFunc = std::function<void(SCPtr)>;

  Message(std::string const& channel_name);

  std::string to_json() const override;

public:
  double t0() const override { return msg_.t0_; }
  _Sensor msg_;
};

template <typename _Sensor>
struct Message<_Sensor, true> : public Message<_Sensor, false> {
  using SPtr = std::shared_ptr<Message>;
  using SCPtr = std::shared_ptr<Message const>;
  using Func = std::function<void(SPtr)>;
  using CFunc = std::function<void(SCPtr)>;
  // 继承构造函数
  using Base = Message<_Sensor, false>;

  Message(std::string const& channel_name)
    : Base(channel_name) {}

  std::string to_json() const override;
  void UpdateRelativePose();

public:
  Eigen::Isometry3d* origin_ = nullptr;
  Eigen::Isometry3d rpose_ = Eigen::Isometry3d::Identity();
};

template <typename _Sensor>
inline std::shared_ptr<Message<_Sensor>> CreateMessage(std::string const& channel_name) {
  return std::make_shared<Message<_Sensor>>(channel_name);
}

#include "message_impl.h"