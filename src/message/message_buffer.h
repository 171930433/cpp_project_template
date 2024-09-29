#pragma once

#include "message/message.h"

#include <boost/multi_index/indexed_by.hpp>
#include <boost/multi_index/key.hpp>
#include <boost/multi_index/ordered_index.hpp>
#include <boost/multi_index/random_access_index.hpp>
#include <boost/multi_index/sequenced_index.hpp>
#include <boost/multi_index_container.hpp>

struct id {};
struct t0 {};
struct t1 {};

namespace boost {
namespace multi_index {

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

// clang-format on

}
}

// class SingleBuffer : public boost::multi_index::SingleMIC {
// public:
//   std::string channle_name_ = "";
//   uint32_t rate_hz_ = 0;
//   std::string device_name_ = "";
//   double duration_s_ = 1;
// };

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
  if (!this->empty()
    && (this->back()->t1_ - this->front()->t1_) >= duration_s_) {
    this->pop_front();
  }
  this->push_back(frame);
}
