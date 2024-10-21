#pragma once

#include "message/message.h"
#include <unordered_map>

#include <boost/multi_index/indexed_by.hpp>
#include <boost/multi_index/key.hpp>
#include <boost/multi_index/ordered_index.hpp>
#include <boost/multi_index/random_access_index.hpp>
#include <boost/multi_index/sequenced_index.hpp>
#include <boost/multi_index_container.hpp>

struct id {};
struct t0 {};
struct t1 {};

namespace boost::multi_index {

// clang-format off

typedef multi_index_container<MessageBase::SCPtr,
    indexed_by<
        random_access<tag<id>>,
        ordered_non_unique<tag<t1>, key<&MessageBase::t1>>,
        ordered_non_unique<tag<t0>, key<&MessageBase::t0>>
    >
>MessageBaseConstSPContainer;


template <typename _Sensor>
class SensorContainer : public multi_index_container<typename Message<_Sensor>::SCPtr,
    indexed_by<
        random_access<tag<id>>,
        ordered_unique<tag<t0>, key<&Message<_Sensor>::t0>> 
    >>
{
public:
  std::string_view channel_name_;
  static constexpr const std::string_view channel_type_ = ylt::reflection::type_string<_Sensor>();
};

// clang-format on

}

namespace bm = boost::multi_index;

using bm::SensorContainer;

template <typename _Sensor>
class SensorMap : public std::unordered_map<std::string_view, bm::SensorContainer<_Sensor>> {
public:
  static constexpr const std::string_view channel_type_ = ylt::reflection::type_string<_Sensor>();
  void Append(MessageBase::SCPtr frame) {
    if (!frame || frame->channel_type_ != channel_type_) return;

    auto& container = (*this)[frame->channel_name_];
    if (container.empty()) { container.channel_name_ = frame->channel_name_; }
    container.push_back(std::dynamic_pointer_cast<Message<_Sensor> const>(frame));
  }
};

template <typename... _Sensors>
class TotalBuffer
  : public boost::multi_index::MessageBaseConstSPContainer
  , public std::tuple<SensorMap<_Sensors>...> {
  using _Base = std::tuple<SensorMap<_Sensors>...>;

public:
  TotalBuffer() {
    this->channel_types_ =
      std::apply([this](auto&&... args) { return std::unordered_set<std::string_view>{ { args.channel_type_ }... }; },
        *static_cast<_Base*>(this));

    this->appenders_ = std::apply(
      [](auto&&... args) {
        return std::unordered_map<std::string_view, std::function<void(MessageBase::SCPtr)>>{ { args.channel_type_,
          std::bind(&std::remove_reference_t<decltype(args)>::Append, &args, std::placeholders::_1) }... };
      },
      *static_cast<_Base*>(this));
  }

  void Append(MessageBase::SCPtr frame) {

    // 缓存通道与类型消息
    channel_names_.insert(frame->channel_name_);

    // 限定区间的缓存
    std::lock_guard<std::mutex> lg(mtx_);
    if (!this->empty() && (this->back()->t1_ - this->front()->t1_) >= duration_s_) { this->pop_front(); }
    this->push_back(frame);

    if (appenders_.contains(frame->channel_type_)) { appenders_[frame->channel_type_](frame); }
  }

  template <typename _Sensor>
  auto& Get(std::string_view channel_name) {
    return std::get<SensorMap<_Sensor>>(*this)[channel_name];
  }

  double duration_s_ = 1;
  std::unordered_set<std::string_view> channel_names_;
  std::unordered_set<std::string_view> channel_types_;

protected:
  std::mutex mtx_;
  std::unordered_map<std::string_view, std::function<void(MessageBase::SCPtr)>> appenders_;
};

using SensorsBuffer = TotalBuffer<Gnss, Imu, State>;

// todo duration_s_控制各子buffer