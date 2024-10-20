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
        ordered_unique<tag<t0>, key<&MessageBase::t0>> 
    >
>SingleMIC;

typedef multi_index_container<MessageBase::SPtr,
    indexed_by<
        random_access<tag<id>>,
        ordered_non_unique<tag<t1>, key<&MessageBase::t1_>>,
        ordered_non_unique<tag<t0>, key<&MessageBase::t0>>
    >
>TotalMIC;


template <typename _Sensor>
using MICSensor = multi_index_container<typename Message<_Sensor>::SCPtr,
    indexed_by<
        random_access<tag<id>>,
        ordered_unique<tag<t0>, key<&Message<_Sensor>::t0>> 
    >
>;

// clang-format on

}

namespace bm = boost::multi_index;

template <typename _Sensor>
class MICBuffer : public std::unordered_map<std::string_view, bm::MICSensor<_Sensor>> {
public:
  static constexpr const std::string_view channel_type_ = ylt::reflection::type_string<_Sensor>();
  void Append(MessageBase::SCPtr frame) {
    if (!frame || frame->channel_type_ != channel_type_) return;
    (*this)[frame->channel_name_].push_back(std::dynamic_pointer_cast<Message<_Sensor> const>(frame));
  }
};

// template <typename _Sensor>
// using MICBuffer = std::unordered_map<std::string_view, MICSensor2<_Sensor>>;

// class MessageBuffer : public boost::multi_index::SingleMIC {
// public:
//   std::string channle_name_ = "";
//   uint32_t rate_hz_ = 0;
//   std::string device_name_ = "";
//   double duration_s_ = 1;
// };

// class Buffers : public std::unordered_map<std::string_view, MessageBuffer> {
// public:
//   void Append(MessageBase::SPtr frame) {
//     channel_types_.insert(frame->channel_type_);
//     (*this)[frame->channel_name_].push_back(frame);
//   }

// public:
//   std::unordered_set<std::string_view> channel_names_;
//   std::unordered_set<std::string_view> channel_types_;
// };

using MessageBuffer = boost::multi_index::SingleMIC;
using Buffers = std::unordered_map<std::string_view, MessageBuffer>;

class TotalBuffer : public boost::multi_index::TotalMIC {
public:
  // template <typename _Message>
  // void Append(std::shared_ptr<ChannelMsg<_Message>> frame);
  void Append(MessageBase::SPtr frame);
  double duration_s_ = 1;

protected:
  std::mutex mtx_;

public:
  std::unordered_set<std::string_view> channel_names_;
  std::unordered_set<std::string_view> channel_types_;
};

// template <typename _Message>
inline void TotalBuffer::Append(MessageBase::SPtr frame) {
  // 缓存通道与类型消息
  channel_types_.insert(frame->channel_type_);
  channel_names_.insert(frame->channel_name_);

  // 限定区间的缓存
  std::lock_guard<std::mutex> lg(mtx_);
  if (!this->empty() && (this->back()->t1_ - this->front()->t1_) >= duration_s_) { this->pop_front(); }
  this->push_back(frame);
}

class TotalBuffer2 : public boost::multi_index::TotalMIC {
public:
  void Append(MessageBase::SPtr frame);
  double duration_s_ = 1;

public:
  MICBuffer<Gnss> gnss_;
  MICBuffer<Imu> imu_;
  MICBuffer<State> state_;
  std::unordered_set<std::string_view> channel_names_;
  std::unordered_set<std::string_view> channel_types_;

protected:
  std::mutex mtx_;
};

inline void TotalBuffer2::Append(MessageBase::SPtr frame) {

  static auto tuple_buffer = std::forward_as_tuple(gnss_, imu_, state_);
  using _TupleBuffer = decltype(tuple_buffer);
  static auto appenders = []<size_t... _I>(std::index_sequence<_I...>) {
    return std::unordered_map<std::string_view, std::function<void(MessageBase::SCPtr)>>{
      { { std::get<_I>(tuple_buffer).channel_type_,
        std::bind(&std::remove_reference_t<std::tuple_element_t<_I, _TupleBuffer>>::Append, &std::get<_I>(tuple_buffer),
          std::placeholders::_1) }... },
    };
  }(std::make_index_sequence<std::tuple_size_v<_TupleBuffer>>{});

  // 缓存通道与类型消息
  channel_types_.insert(frame->channel_type_);
  channel_names_.insert(frame->channel_name_);

  // 限定区间的缓存
  std::lock_guard<std::mutex> lg(mtx_);
  if (!this->empty() && (this->back()->t1_ - this->front()->t1_) >= duration_s_) { this->pop_front(); }
  this->push_back(frame);

  if (appenders.contains(frame->channel_type_)) { appenders[frame->channel_type_](frame); }
}

template <typename... _Sensors>
class TotalBuffer3 : public std::tuple<MICBuffer<_Sensors>...> {
  using _Base = std::tuple<MICBuffer<_Sensors>...>;

public:
  void Append(MessageBase::SPtr frame) {
    constexpr static size_t _N = sizeof...(_Sensors);
    static auto appenders = [this]<size_t... _I>(std::index_sequence<_I...>) {
      return std::unordered_map<std::string_view, std::function<void(MessageBase::SCPtr)>>{
        { { std::get<_I>(*this).channel_type_,
          std::bind(&std::tuple_element_t<_I, _Base>::Append, &std::get<_I>(*this), std::placeholders::_1) }... }
      };
    }(std::make_index_sequence<_N>{});

    // 缓存通道与类型消息
    channel_types_.insert(frame->channel_type_);
    channel_names_.insert(frame->channel_name_);

    // 限定区间的缓存
    std::lock_guard<std::mutex> lg(mtx_);
    if (!msgs_.empty() && (msgs_.back()->t1_ - msgs_.front()->t1_) >= duration_s_) { msgs_.pop_front(); }
    msgs_.push_back(frame);

    if (appenders.contains(frame->channel_type_)) { appenders[frame->channel_type_](frame); }
  }

  template<typename _Sensor>
  auto& Get(std::string_view channel_name) { 
    return std::get<MICBuffer<_Sensor>>(*this)[channel_name];
  }

  double duration_s_ = 1;
  boost::multi_index::TotalMIC msgs_;
  std::unordered_set<std::string_view> channel_names_;
  std::unordered_set<std::string_view> channel_types_;

protected:
  std::mutex mtx_;
};
