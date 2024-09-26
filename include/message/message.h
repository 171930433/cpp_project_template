#pragma once
#include "message/sensors.h"
#include <fmt/core.h>
#include <fmt/ostream.h>
#include <unordered_set>
#include <ylt/reflection/template_string.hpp>

struct MessageBase {
  using SPtr = std::shared_ptr<MessageBase>;
  using SCPtr = std::shared_ptr<MessageBase const>;
  virtual ~MessageBase() = default;
  virtual double t0() const = 0;
  virtual std::string to_json_str() const {
    std::stringstream ss;
    ss << fmt::format(R"("channel_name_":"{}",)", channel_name_);
    ss << fmt::format(R"("channel_type_":"{}",)", channel_type_);
    ss << fmt::format(R"("t0_":{},)", t0());
    ss << fmt::format(R"("t1_":{},)", t1_);
    ss << fmt::format(R"("t2_":{},)", t2_);
    return ss.str();
  }

public:
  std::string_view channel_name_ = "";
  std::string_view channel_type_ = "";
  double t1_ = 0; // 接收到时间
  double t2_ = 0; // 落盘时间
public:
  static std::unordered_set<std::string> channel_names_;
  static std::unordered_set<std::string_view> channel_types_;
};

template <typename _Message>
struct ChannelMsg : public MessageBase {
  using SPtr = std::shared_ptr<ChannelMsg>;
  using SCPtr = std::shared_ptr<ChannelMsg const>;
  ChannelMsg(std::string const& channel_name);

  static std::shared_ptr<ChannelMsg> Create(std::string const& channel_name);

  std::string to_json_str() const;

public:
  double t0() const override { return msg_.t0_; }
  _Message msg_;
};

// inline std::ostream& operator<<(std::ostream& os, MessageBase::SPtr elem) {
//   os << elem->to_json_str();
//   return os;
// }

// inline std::ostream& operator<<(std::ostream& os, MessageBase::SCPtr elem) {
//   os << elem->to_json_str();
//   return os;
// }

#include "message_impl.h"